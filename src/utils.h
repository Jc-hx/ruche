#pragma once
#include <Arduino.h>

extern volatile bool boutonAppuye;   // passe à true quand le bouton réveille le Nano

void dormir8s();   // dort 8 s, ou moins si le bouton est pressé
void bipper();     // 3 bips

// Batterie
enum AlerteBatterie { BATT_OK, BATT_30, BATT_10 };
int mesureBatterie();                      // pourcentage 0 à 100
AlerteBatterie verifierBatterie(int pct);  // renvoie chaque alerte une seule fois