#include <Arduino.h>
#include <config.h>
#include <modem.h>
#include <scale.h>
#include <commands.h>
#include <users.h>


void setup() {
  Serial.begin(115200);
  Serial.println(F("=== Initialisation Arduino ==="));
  initialiserCapteursMasse();
  
}

void loop() {
  testerCapteursEtVariations();
  delay(5000);
}