#pragma once
#include "config.h"

#define NB_CAPTEURS 4

struct Mesure {
  float masseKg[NB_CAPTEURS];
  float totalKg;
};

enum Variation { STABLE, HAUSSE, BAISSE };

extern Mesure mesureActuelle;
extern Mesure mesurePrecedente;
extern Mesure mesuresConfirmation[ALERT_CONFIRMATIONS];

// Fonctions principales du module
void initialiserCapteursMasse(); // Dans le setup en sortie de deep sleep
Variation mesureMasse(); // Retourne STABLE, HAUSSE, BAISSE et maj totalKg et masseKg[NB_CAPTEURS]
Variation testerCapteursEtVariations(); // Fonction de test des capteurs
