#pragma once
#include <Arduino.h>
#include "scale.h"

void initialiserRapport();
void noterMesure(long masseG, int heure);
void noterAlerteEnvoyee();
void archiverRapportQuotidien();

void construireRapport(char *texte, size_t taille, int batteriePct, int batterieMv);
void construireDetails(char *texte, size_t taille, int signal);
void construireAlerte(char *texte, size_t taille, Variation sens, int batteriePct);

void envoyerRapport(bool quotidien);
bool rapportDejaEnvoye();
void memoriserRapportEnvoye(bool envoye);