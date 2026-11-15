#include <Arduino.h>
#include <HX711.h>
#include "config.h"
#include "scale.h"

// --- Instanciation des 4 balances ---
static HX711 hx711[NB_CAPTEURS];
static const int   PINS_DT[NB_CAPTEURS]      = {DT1, DT2, DT3, DT4};
static const float FACTEURS_CAL[NB_CAPTEURS] = {CAL_AG, CAL_ARG, CAL_ARD, CAL_AD};
static const long  OFFSETS[NB_CAPTEURS]      = {OFFSET_AG, OFFSET_ARG, OFFSET_ARD, OFFSET_AD};

Mesure mesureActuelle;
Mesure mesurePrecedente;
Mesure mesureAvantAlerte;
Mesure mesuresConfirmation[ALERT_CONFIRMATIONS];

static void reveillerCapteurs() {
  for (int i = 0; i < NB_CAPTEURS; i++) hx711[i].power_up();
  delay(DELAI_STABILISATION);
}

static void endormirCapteurs() {
  for (int i = 0; i < NB_CAPTEURS; i++) hx711[i].power_down();
}

static bool lireMasses(Mesure &m) {
  m.totalKg = 0;
  for (int i = 0; i < NB_CAPTEURS; i++) {
    if (!hx711[i].wait_ready_timeout(1000)) return false;   // capteur muet après 1 s
    m.masseKg[i] = hx711[i].get_units(nombreMesuresMasse);
    m.totalKg += m.masseKg[i];
  }
  return true;
}

static Variation detecterVariation(const Mesure &avant, const Mesure &apres) {
  float ecartKg = apres.totalKg - avant.totalKg;

  Variation sens = STABLE;
  if (ecartKg >  ALERT_DELTA_KG) sens = HAUSSE;
  if (ecartKg < -ALERT_DELTA_KG) sens = BAISSE;
  return sens;
}

void initialiserCapteursMasse() {
  for (int i = 0; i < NB_CAPTEURS; i++) {
    hx711[i].begin(PINS_DT[i], CLK);
    hx711[i].set_scale(FACTEURS_CAL[i]);
    hx711[i].set_offset(OFFSETS[i]);
  }
  // Mesure de référence au démarrage
  reveillerCapteurs();
  if (!lireMasses(mesurePrecedente)) {
    endormirCapteurs();
    Serial.println("ERREUR : un capteur ne répond pas");
  }
  endormirCapteurs();
}

Variation mesureMasse() {
  Serial.println("Mesure de masse");
  reveillerCapteurs();
  if (!lireMasses(mesureActuelle)) {
    endormirCapteurs();
    return ERREUR;
  }
  mesureAvantAlerte = mesurePrecedente;

  Variation sens = detecterVariation(mesurePrecedente, mesureActuelle);
  int nbConfirmations = 0;

  // Variation détectée : mesure supplementaires
  while (sens != STABLE && nbConfirmations < ALERT_CONFIRMATIONS) {
    delay(ALERT_CONFIRM_DELAI);
    Mesure &confirmation = mesuresConfirmation[nbConfirmations];
    if (!lireMasses(confirmation)) {
      sens = ERREUR;
      break;
    }
    nbConfirmations++;
    if (detecterVariation(mesurePrecedente, confirmation) != sens) sens = STABLE;
  }

  if (sens == ERREUR) {
    endormirCapteurs();
    return ERREUR;
  }

  // La dernière mesure faite devient la référence du prochain cycle
  if (nbConfirmations > 0) mesurePrecedente = mesuresConfirmation[nbConfirmations - 1];
  else                     mesurePrecedente = mesureActuelle;

  endormirCapteurs();
  Serial.print("Masse total mesurée : ");
  Serial.println(mesureActuelle.totalKg);
  return sens;
}

// Fonction de test
Variation testerCapteursEtVariations() {
  Serial.println("\n----------------------------------------");
  Serial.println("=== DIAGNOSTIC EN DIRECT ===");
  
  reveillerCapteurs();

  // Lecture directe des poids capteur par capteur
  if (!lireMasses(mesureActuelle)) {
    Serial.println("ERREUR : un capteur ne répond pas");
    endormirCapteurs();
    return ERREUR;
  }

  Serial.print("Masse AG  : "); Serial.println(mesureActuelle.masseKg[0]);
  Serial.print("Masse ArG : "); Serial.println(mesureActuelle.masseKg[1]);
  Serial.print("Masse ArD : "); Serial.println(mesureActuelle.masseKg[2]);
  Serial.print("Masse AD  : "); Serial.println(mesureActuelle.masseKg[3]);
  Serial.print("TOTAL     : "); Serial.println(mesureActuelle.totalKg);

  Variation sens = detecterVariation(mesurePrecedente, mesureActuelle);
  
  Serial.print("Etat détecté : ");
  if (sens == STABLE) Serial.println("STABLE (0)");
  else if (sens == HAUSSE) Serial.println("HAUSSE (1)");
  else if (sens == BAISSE) Serial.println("BAISSE (2)");

  mesurePrecedente = mesureActuelle;
  endormirCapteurs();
  return sens;
}