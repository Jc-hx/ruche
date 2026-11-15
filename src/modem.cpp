#include <Arduino.h>
#include <SoftwareSerial.h>
#include "config.h"
#include "modem.h"

static SoftwareSerial modem(MODEM_RX, MODEM_TX);
static char tampon[80];

// --- Horloge interne ---
static unsigned long secondes = 0;       // secondes écoulées (modulo jour géré à la lecture)
static unsigned long resteMs = 0;
static unsigned long dernierMs = 0;
static unsigned long secondesDepuisSync = 0;
static unsigned long delaiAvantSync = 0; // 0 = synchro immédiate
static bool valide = false;

static void normaliserHorloge() {
  while (resteMs >= 1000) { resteMs -= 1000; secondes++; secondesDepuisSync++; }
}

static void rattraperMillis() {
  unsigned long maintenant = millis();
  resteMs += maintenant - dernierMs;
  dernierMs = maintenant;
  normaliserHorloge();
}

void horlogeAjouterSecondes(unsigned long s) {
  rattraperMillis();
  secondes += s;
  secondesDepuisSync += s;
}

bool horlogeValide() { return valide; }

long horlogeSecondesDepuisMinuit() {
  rattraperMillis();
  return secondes % SECONDES_PAR_JOUR;
}

bool horlogeSynchroNecessaire() {
  rattraperMillis();
  return secondesDepuisSync >= delaiAvantSync;
}

// --- Dialogue avec le modem ---
static bool envoyer(const char *commande, const char *attendu, unsigned long timeoutMs) {
  while (modem.available()) modem.read();
  tampon[0] = '\0';
  modem.println(commande);
  unsigned long debut = millis();
  size_t n = 0;
  while (millis() - debut < timeoutMs) {
    while (modem.available() && n < sizeof(tampon) - 1) {
      tampon[n++] = modem.read();
      tampon[n] = '\0';
    }
    if (strstr(tampon, attendu)) return true;
    if (strstr(tampon, "ERROR")) return false;
  }
  return false;
}

void modemAllumer() {
  pinMode(MODEM_PWR, OUTPUT);
  digitalWrite(MODEM_PWR, HIGH);
  modem.begin(MODEM_BAUD);
  delay(5000);
  for (int i = 0; i < 5 && !envoyer("AT", "OK", 1000); i++) {}
  envoyer("ATE0", "OK", 1000);
}

void modemEteindre() {
  digitalWrite(MODEM_PWR, LOW);
}

static bool attendreReseau(unsigned long timeoutMs) {
  unsigned long debut = millis();
  while (millis() - debut < timeoutMs) {
    if (envoyer("AT+CEREG?", "OK", 1000) &&
        (strstr(tampon, ",1") || strstr(tampon, ",5"))) return true;
    delay(2000);
  }
  return false;
}

static bool attendre(const char *attendu, unsigned long timeoutMs) {
  tampon[0] = '\0';
  size_t n = 0;
  unsigned long debut = millis();
  while (millis() - debut < timeoutMs) {
    while (modem.available() && n < sizeof(tampon) - 1) {
      tampon[n++] = modem.read();
      tampon[n] = '\0';
    }
    if (strstr(tampon, attendu)) return true;
    if (strstr(tampon, "ERROR")) return false;
  }
  return false;
}

static bool envoyerUnSms(const char *numero, const char *texte) {
  char commande[32];
  snprintf(commande, sizeof(commande), "AT+CMGS=\"%s\"", numero);
  if (!envoyer(commande, ">", 5000)) return false;   // le modem attend le texte
  modem.print(texte);
  modem.write(26);                                    // Ctrl+Z : fin du message
  return attendre("+CMGS:", 30000);                   // confirmation d'envoi
}

bool envoyerSms(const char *const numeros[], int nombre, const char *texte) {
  modemAllumer();
  bool toutEnvoye = false;
  if (attendreReseau(30000)) {
    envoyer("AT+CMGF=1", "OK", 1000);                 // mode texte
    envoyer("AT+CSCS=\"GSM\"", "OK", 1000);
    toutEnvoye = true;
    for (int i = 0; i < nombre; i++) {
      if (!envoyerUnSms(numeros[i], texte)) toutEnvoye = false;
    }
  }
  modemEteindre();
  return toutEnvoye;
}

// Lit "+CCLK: "26/11/14,15:42:07+04" et renvoie les secondes depuis minuit
static bool lireHeureModem(long &secDepuisMinuit) {
  if (!envoyer("AT+CCLK?", "OK", 2000)) return false;
  char *p = strchr(tampon, '"');
  if (!p) return false;
  int a, mo, j, h, mi, s;
  if (sscanf(p + 1, "%d/%d/%d,%d:%d:%d", &a, &mo, &j, &h, &mi, &s) != 6) return false;
  if (a < 25) return false;               // 70/01/01 ou 04/01/01 = heure non réglée
  secDepuisMinuit = h * SECONDES_PAR_HEURE + mi * 60L + s;
  return true;
}

bool modemSynchroniserHeure() {
  bool ok = false;
  if (attendreReseau(30000)) {
    char commande[48];
    snprintf(commande, sizeof(commande), "AT+CGDCONT=1,\"IP\",\"%s\"", MODEM_APN);
    envoyer(commande, "OK", 1000);
    if (envoyer("AT+CGACT=1,1", "OK", 10000)) {
      envoyer("AT+CNTP=\"pool.ntp.org\",4", "OK", 1000);
      if (envoyer("AT+CNTP", "+CNTP: 0", 15000)) {
        long sec;
        if (lireHeureModem(sec)) {
          secondes = sec;
          resteMs = 0;
          dernierMs = millis();
          valide = true;
          ok = true;
        }
      }
      envoyer("AT+CGACT=0,1", "OK", 5000);   // coupe la data
    }
  }
  delaiAvantSync = ok ? SYNC_INTERVALLE_S : SYNC_RETRY_S;
  secondesDepuisSync = 0;
  return ok;
}

bool synchroniserHeure() {
  modemAllumer();
  bool heureRecue = modemSynchroniserHeure();
  modemEteindre();
  return heureRecue;
}