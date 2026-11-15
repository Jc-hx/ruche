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
bool envoyerSms(const char *const numeros[], int nombre, const char *texte);