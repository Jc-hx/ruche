#include <Arduino.h>
#include <config.h>

void mesureBatterie() {
  Serial.println("=== Mesure de batterie ===");
  long somme = 0;
  for (int i = 0; i < nombreMesuresBat; i++) {
    somme += analogRead(batteriePin);
    delay(5);
  }
  int valeurBrute = somme / nombreMesuresBat;

  // Conversion en pourcentage
  int batterie = map(valeurBrute, BATT_ADC_0, BATT_ADC_100, 0, 100);
  
  // Empêcher de dépasser 0% ou 100%
  batterie = constrain(batterie, 0, 100);

  Serial.println("Pourcentage de batterie mesuré : ");
  Serial.print(batterie);
  Serial.println(" %");
}