#include <Arduino.h>
#include <LowPower.h>
#include "config.h"
#include "modem.h"
#include "scale.h"
#include "commands.h"
#include "users.h"
#include "rapport.h"
#include "utils.h"

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
}

void loop() {

#if TEST_CAPTEURS
  testerCapteursEtVariations();
  delay(5000);

#elif TEST_BATTERIE
  mesureBatterie();
  delay(5000);

#elif TEST_HORLOGE
  modemAllumer();
  bool ok = modemSynchroniserHeure();
  modemEteindre();
  long s = horlogeSecondesDepuisMinuit();
  Serial.print(F("Synchro : ")); Serial.println(ok ? F("OK") : F("ECHEC"));
  Serial.print(F("Heure : "));
  Serial.print(s / 3600); Serial.print(':');
  Serial.print((s % 3600) / 60); Serial.print(':');
  Serial.println(s % 60);
  delay(8000);
  horlogeAjouterSecondes(8);   // simule un sommeil de 8 s

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