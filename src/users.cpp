#include <Arduino.h>
#include "config.h"
#include "users.h"
#include "modem.h"
#include "secrets.h"

static const char *salutation() {
  if (horlogeValide() && horlogeSecondesDepuisMinuit() >= HEURE_BONSOIR * SECONDES_PAR_HEURE) return "Bonsoir";
  return "Bonjour";
}

void envoyerATous(const char *texte) {
  Serial.println(F("--- SMS ---"));
  Serial.println(texte);

  if (modemOuvrirSms()) {
    for (int i = 0; i < NB_NUMEROS; i++) {
      char message[161];
      snprintf(message, sizeof(message), "%s %s,\n%s", salutation(), PRENOMS[i], texte);
      modemEnvoyerSms(NUMEROS[i], message);
    }
  }
  modemFermerSms();
}