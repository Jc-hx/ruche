#include <Arduino.h>
#include "users.h"
#include "modem.h"
#include "secrets.h"

void envoyerATous(const char *texte) {
  Serial.println(F("--- SMS ---"));
  Serial.println(texte);
  envoyerSms(NUMEROS, NB_NUMEROS, texte);
}