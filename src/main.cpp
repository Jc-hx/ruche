#include <Arduino.h>
#include <LowPower.h>
#include "config.h"
#include "modem.h"
#include "scale.h"
#include "commands.h"
#include "users.h"
#include "rapport.h"
#include "utils.h"

#if TEST_NITZ
#include <SoftwareSerial.h>
static SoftwareSerial modemTest(MODEM_RX, MODEM_TX);

static void commandeAT(const char *commande, unsigned long attenteMs = 1000) {
  Serial.print(F("> ")); Serial.println(commande);
  modemTest.println(commande);
  unsigned long debut = millis();
  while (millis() - debut < attenteMs) {
    while (modemTest.available()) Serial.write(modemTest.read());
  }
  Serial.println();
}
#endif

static long secondesDepuisMesure = MESURE_PERIODE_S;   // mesure dès le démarrage
static bool rapportEnvoye;
static bool synchroAvantEnvoiFaite = false;
static const long HEURE_ENVOI_S = HEURE_SMS_QUOTIDIEN * SECONDES_PAR_HEURE + MINUTE_SMS_QUOTIDIEN * 60L;
static long secondesDepuisRapport = 0;

static void dormir() {
  dormir8s();
  horlogeAjouterSecondes(8);
  secondesDepuisMesure += 8;
  secondesDepuisRapport += 8;
}

static void faireMesure() {
  char texte[161];
  Variation variation = mesureMasse();
  int batteriePct = mesureBatterie();

  if (variation != ERREUR) {
    noterMesure((long)(mesureActuelle.totalKg * 1000), horlogeSecondesDepuisMinuit() / 3600);
  }
  if (variation != STABLE) {
    construireAlerte(texte, sizeof(texte), variation, batteriePct);
    envoyerATous(texte);
    noterAlerteEnvoyee();
  }

  AlerteBatterie alerteBatterie = verifierBatterie(batteriePct);
  if (alerteBatterie == BATT_30) envoyerATous("ALERTE\nBatterie inferieure a 30%");
  if (alerteBatterie == BATT_10) envoyerATous("ALERTE\nBatterie inferieure a 10%");
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("=== Initialisation Arduino Nano ==="));
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  initialiserCapteursMasse();
  initialiserRapport();
  rapportEnvoye = rapportDejaEnvoye();

  #if !(TEST_CAPTEURS || TEST_BATTERIE || TEST_HORLOGE || TEST_NITZ || TEST_RAPPORT || TEST_SMS)
    bipper();                   // les 3 bips du démarrage, comme avant
    synchroniserHeure();        // l'heure doit être connue avant le message
    envoyerInitialisation();
  #endif
}

void loop() {

#if TEST_CAPTEURS
  testerCapteursEtVariations();
  delay(5000);

#elif TEST_BATTERIE
  mesureBatterie();
  delay(5000);

#elif TEST_HORLOGE
  static int numeroEssai = 0;
  numeroEssai++;

  unsigned long debut = millis();
  bool heureRecue = synchroniserHeure();
  unsigned long dureeS = (millis() - debut) / 1000;

  long s = horlogeSecondesDepuisMinuit();
  Serial.print(F("Essai ")); Serial.print(numeroEssai);
  Serial.print(F(" : ")); Serial.print(heureRecue ? F("OK") : F("ECHEC"));
  Serial.print(F(" en ")); Serial.print(dureeS); Serial.println(F(" s"));
  Serial.print(F("Heure : "));
  Serial.print(s / 3600); Serial.print(':');
  Serial.print((s % 3600) / 60); Serial.print(':');
  Serial.println(s % 60);
  Serial.println();

  delay(10000);

#elif TEST_NITZ
  const long vitesses[] = {9600, 115200};
  pinMode(MODEM_PWR, OUTPUT);
  digitalWrite(MODEM_PWR, LOW);          // vraie coupure
  delay(3000);
  digitalWrite(MODEM_PWR, HIGH);
  delay(5000);

  for (int i = 0; i < 2; i++) {
    Serial.print(F("=== Vitesse ")); Serial.println(vitesses[i]);
    modemTest.end();
    modemTest.begin(vitesses[i]);
    commandeAT("AT", 1500);
    commandeAT("AT", 1500);
  }

  digitalWrite(MODEM_PWR, LOW);
  Serial.println(F("=== Fin ==="));
  while (true) {}

#elif TEST_RAPPORT
  mesureMasse();
  noterMesure((long)(mesureActuelle.totalKg * 1000), 12);
  char texte[161];
  construireRapport(texte, sizeof(texte), 75);
  Serial.println(texte);
  construireDetails(texte, sizeof(texte), 20);
  Serial.println(texte);
  delay(5000);

#elif TEST_SMS
  envoyerATous("Test ruche");
  delay(60000);
  
  #else
  if (boutonAppuye) {
    bipper();
    while (digitalRead(buttonPin) == LOW) {}   // attend le relâchement
    envoyerRapport(false);
    boutonAppuye = false;
  }

  if (horlogeSynchroNecessaire()) synchroniserHeure();

  long heureS = horlogeSecondesDepuisMinuit();
  if (heureS < HEURE_ENVOI_S - AVANCE_SYNCHRO_S) {
    synchroAvantEnvoiFaite = false;
  } else if (heureS < HEURE_ENVOI_S && !synchroAvantEnvoiFaite) {
    synchroniserHeure();
    synchroAvantEnvoiFaite = true;
    heureS = horlogeSecondesDepuisMinuit();
  }

  if (horlogeValide()) {
    if (heureS < HEURE_ENVOI_S) {
      rapportEnvoye = false;
      memoriserRapportEnvoye(false);
    } else if (!rapportEnvoye) {
      envoyerRapport(true);
      secondesDepuisRapport = 0;
      rapportEnvoye = true;
      memoriserRapportEnvoye(true);
    }
  } else if (secondesDepuisRapport >= SECOURS_DELAI_S) {
    envoyerRapport(true);        // heure inconnue : envoi de secours
    secondesDepuisRapport = 0;
  }

  if (secondesDepuisMesure >= MESURE_PERIODE_S) {
    faireMesure();
    secondesDepuisMesure = 0;
  }

  dormir();
#endif
}