#include <Arduino.h>
#include <LowPower.h>
#include "config.h"
#include "utils.h"

volatile bool boutonAppuye = false;

static void reveilBouton() {
  detachInterrupt(digitalPinToInterrupt(buttonPin));
  boutonAppuye = true;
}

void dormir8s() {
  Serial.flush();
  attachInterrupt(digitalPinToInterrupt(buttonPin), reveilBouton, LOW);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  detachInterrupt(digitalPinToInterrupt(buttonPin));
}

void bipper() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(50);
    digitalWrite(buzzerPin, LOW);
    delay(50);
  }
}

int mesureBatterie() {
  long somme = 0;
  for (int i = 0; i < nombreMesuresBat; i++) {
    somme += analogRead(batteriePin);
    delay(5);
  }
  int valeurBrute = somme / nombreMesuresBat;

  int batterie = map(valeurBrute, BATT_ADC_0, BATT_ADC_100, 0, 100);
  batterie = constrain(batterie, 0, 100);

  Serial.print("Batterie : ");
  Serial.print(batterie);
  Serial.println(" %");
  return batterie;
}

AlerteBatterie verifierBatterie(int pct) {
  static int etat = 0;                 // 0 = rien envoyé, 1 = alerte 30 % faite, 2 = alerte 10 % faite
  if (pct > 50) etat = 0;              // batterie rechargée : on réarme
  if (pct <= 10 && etat < 2) { etat = 2; return BATT_10; }
  if (pct <= 30 && etat < 1) { etat = 1; return BATT_30; }
  return BATT_OK;
}