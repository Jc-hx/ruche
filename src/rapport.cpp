#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "scale.h"
#include "rapport.h"
#include "users.h"
#include "utils.h"

#define NB_JOURS            7
#define MARQUEUR_EEPROM     0xA5
#define MASSE_RUCHE_VIDE_G  ((long)(MASSE_RUCHE_VIDE_KG * 1000))

struct Historique {
  byte marqueur;
  byte nbJours;                  // nombre de rapports déjà archivés (0 à 7)
  int  nbRedemarrages;
  long masseSoirG[NB_JOURS];     // [0] = dernier rapport, [6] = 7 rapports
};
static Historique historique;

static long masseActuelleG = 0;
static long masseMatinG = 0;
static bool matinPris = false;
static long masseMinG = 0, masseMaxG = 0;
static int  heureMin = 0, heureMax = 0;
static bool minMaxConnus = false;
static int  nbAlertes = 0;

static void ajouter(char *texte, size_t taille, const char *ligne) {
  strncat(texte, ligne, taille - strlen(texte) - 1);
}

static void formaterKg(char *sortie, size_t taille, long masseG, bool avecSigne) {
  long centiemesKg = (labs(masseG) + 5) / 10;
  char signe = (masseG < 0) ? '-' : '+';
  if (avecSigne) snprintf(sortie, taille, "%c%ld,%02ld", signe, centiemesKg / 100, centiemesKg % 100);
  else           snprintf(sortie, taille, "%ld,%02ld", centiemesKg / 100, centiemesKg % 100);
}

void initialiserRapport() {
  EEPROM.get(EEPROM_ADDR_HISTORIQUE, historique);
  if (historique.marqueur != MARQUEUR_EEPROM) {
    memset(&historique, 0, sizeof(historique));
    historique.marqueur = MARQUEUR_EEPROM;
  }
  historique.nbRedemarrages++;
  EEPROM.put(EEPROM_ADDR_HISTORIQUE, historique);
}

void noterMesure(long masseG, int heure) {
  masseActuelleG = masseG;

  if (heure < HEURE_MATIN) matinPris = false;
  else if (!matinPris) {
    masseMatinG = masseG;
    matinPris = true;
  }

  if (!minMaxConnus || masseG < masseMinG) { masseMinG = masseG; heureMin = heure; }
  if (!minMaxConnus || masseG > masseMaxG) { masseMaxG = masseG; heureMax = heure; }
  minMaxConnus = true;
}

void noterAlerteEnvoyee() {
  nbAlertes++;
}

void archiverRapportQuotidien() {
  for (int i = NB_JOURS - 1; i > 0; i--) {
    historique.masseSoirG[i] = historique.masseSoirG[i - 1];
  }
  historique.masseSoirG[0] = masseActuelleG;
  if (historique.nbJours < NB_JOURS) historique.nbJours++;
  EEPROM.put(EEPROM_ADDR_HISTORIQUE, historique);

  minMaxConnus = false;
  nbAlertes = 0;
}

void construireRapport(char *texte, size_t taille, int batteriePct) {
  char ligne[48], kg1[12], kg2[12];
  texte[0] = '\0';

  formaterKg(kg1, sizeof(kg1), masseActuelleG, false);
  if (MASSE_RUCHE_VIDE_G > 0) {
    formaterKg(kg2, sizeof(kg2), masseActuelleG - MASSE_RUCHE_VIDE_G, false);
    snprintf(ligne, sizeof(ligne), "RUCHE\nMasse %s kg (nette %s)\n", kg1, kg2);
  } else {
    snprintf(ligne, sizeof(ligne), "RUCHE\nMasse %s kg\n", kg1);
  }
  ajouter(texte, taille, ligne);

  if (historique.nbJours >= 1 && historique.masseSoirG[0] > 0) {
    long ecartG = masseActuelleG - historique.masseSoirG[0];
    long dixiemesPct = ecartG * 1000 / historique.masseSoirG[0];
    formaterKg(kg1, sizeof(kg1), ecartG, true);
    snprintf(ligne, sizeof(ligne), "Depuis dernier rapport %s kg (%c%ld,%ld%%)\n",
             kg1, dixiemesPct < 0 ? '-' : '+', labs(dixiemesPct) / 10, labs(dixiemesPct) % 10);
    ajouter(texte, taille, ligne);
  }

  if (historique.nbJours == NB_JOURS) {
    formaterKg(kg1, sizeof(kg1), masseActuelleG - historique.masseSoirG[NB_JOURS - 1], true);
    snprintf(ligne, sizeof(ligne), "7 jours %s kg\n", kg1);
    ajouter(texte, taille, ligne);
  }

  if (matinPris && historique.nbJours >= 1) {
    formaterKg(kg1, sizeof(kg1), masseMatinG - historique.masseSoirG[0], true);
    formaterKg(kg2, sizeof(kg2), masseActuelleG - masseMatinG, true);
    snprintf(ligne, sizeof(ligne), "Nuit %s Jour %s\n", kg1, kg2);
    ajouter(texte, taille, ligne);
  }

  snprintf(ligne, sizeof(ligne), "Batt %d%%", batteriePct);
  ajouter(texte, taille, ligne);
}

void construireDetails(char *texte, size_t taille, int signal) {
  char ligne[48], kg1[12], kg2[12];
  texte[0] = '\0';

  if (minMaxConnus) {
    formaterKg(kg1, sizeof(kg1), masseMinG, false);
    formaterKg(kg2, sizeof(kg2), masseMaxG, false);
    snprintf(ligne, sizeof(ligne), "Min %s kg a %dh\nMax %s kg a %dh\n", kg1, heureMin, kg2, heureMax);
    ajouter(texte, taille, ligne);
  }

  int partPct[NB_CAPTEURS] = {};
  if (mesureActuelle.totalKg > 0) {
    for (int i = 0; i < NB_CAPTEURS; i++) {
      partPct[i] = (int)(mesureActuelle.masseKg[i] * 100 / mesureActuelle.totalKg + 0.5);
    }
  }
  snprintf(ligne, sizeof(ligne), "AG %d%% ArG %d%% ArD %d%% AD %d%%\n",
           partPct[0], partPct[1], partPct[2], partPct[3]);
  ajouter(texte, taille, ligne);

  if (signal >= 0 && signal <= 31) snprintf(ligne, sizeof(ligne), "Signal %d/31\n", signal);
  else                             snprintf(ligne, sizeof(ligne), "Signal inconnu\n");
  ajouter(texte, taille, ligne);

  snprintf(ligne, sizeof(ligne), "Alertes %d\nRedemarrages %d", nbAlertes, historique.nbRedemarrages);
  ajouter(texte, taille, ligne);
}

void construireAlerte(char *texte, size_t taille, Variation sens, int batteriePct) {
  if (sens == ERREUR) {
    snprintf(texte, taille, "ALERTE\nUn capteur de masse ne repond plus\nBatt %d%%", batteriePct);
    return;
  }
  char kg[12];
  long ecartG = lround((mesureActuelle.totalKg - mesureAvantAlerte.totalKg) * 1000);
  formaterKg(kg, sizeof(kg), masseActuelleG, false);
  snprintf(texte, taille, "ALERTE %s\n%c%ld g en %d min\nMasse %s kg\nBatt %d%%",
           sens == HAUSSE ? "hausse" : "baisse",
           ecartG < 0 ? '-' : '+', labs(ecartG), MESURE_PERIODE_S / 60,
           kg, batteriePct);
}

void envoyerRapport(bool quotidien) {
  char texte[161];
  construireRapport(texte, sizeof(texte), mesureBatterie());
  envoyerATous(texte);
  construireDetails(texte, sizeof(texte), -1);   // -1 = signal inconnu pour l'instant
  envoyerATous(texte);
  if (quotidien) archiverRapportQuotidien();
}

bool rapportDejaEnvoye() {
  return EEPROM.read(EEPROM_ADDR_RAPPORT_ENVOYE) == 1;
}

void memoriserRapportEnvoye(bool envoye) {
  EEPROM.update(EEPROM_ADDR_RAPPORT_ENVOYE, envoye ? 1 : 0);
}