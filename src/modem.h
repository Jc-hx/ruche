#pragma once
#include <Arduino.h>

void modemAllumer();
void modemEteindre();
bool synchroniserHeure();
bool modemSynchroniserHeure();        // modem allumé requis, renvoie true si NTP ok
void horlogeAjouterSecondes(unsigned long s);  // à appeler après chaque sommeil
bool horlogeValide();                 // false tant qu'aucune synchro n'a réussi
long horlogeSecondesDepuisMinuit();
bool horlogeSynchroNecessaire();      // true si l'heure est inconnue ou trop ancienne
bool modemOuvrirSms();                                    // allume, attend le réseau, mode texte
bool modemEnvoyerSms(const char *numero, const char *texte);
void modemFermerSms();                                    // éteint