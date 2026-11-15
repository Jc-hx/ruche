#pragma once

/*===========================*/
/* Fichier de configuration */
/*===========================*/

/* ---------- Mode de test (un seul à 1 à la fois) ---------- */
#define TEST_CAPTEURS   0   // affiche les masses toutes les 5 s
#define TEST_BATTERIE   0   // affiche le pourcentage de batterie
#define TEST_HORLOGE    1   // synchro NTP puis affichage de l'heure
#define TEST_RAPPORT    0   // affiche le texte des SMS (sans les envoyer)
#define TEST_SMS        0   // envoi d'un sms à tout le monde

/* Bouton */
#define buttonPin 2

/* Capteurs de masse */
#define CLK 4
#define DT1 5
#define DT2 6
#define DT3 7
#define DT4 8
// Paramètres de mesure de masse
#define nombreMesuresMasse 5          // Nombre de mesure pour estimer la masse
#define ALERT_DELTA_KG 0.1            // Seuil de variation : 0.1 kg = 100 g
#define ALERT_CONFIRMATIONS 2         // Nombre de mesures de confirmation
#define ALERT_CONFIRM_DELAI 5000      // Délai entre confirmations (ms)
#define DELAI_STABILISATION 3000      // Attente après allumage
#define MASSE_RUCHE_VIDE_KG 0.0       // masse du corps + toit + hausses, à mesurer
#define HEURE_MATIN 6                 // première mesure après cette heure = "masse du matin"
#define MESURE_PERIODE_S 900          // entre deux mesures (utile pour "en X min" dans l'alerte)
#define AVANCE_SYNCHRO_S 900          // delai avant l'envoi du SMS à partir duquel on synchro l'heure
#define EEPROM_ADDR_HISTORIQUE 0      // occupe 32 octets, les utilisateurs commence à 64
#define EEPROM_ADDR_RAPPORT_ENVOYE 32 // 1 octet, juste après l'historique (0 à 31)
// Facteurs de calibration
#define CAL_AG 103550.2959
#define CAL_ARG 104807.6923
#define CAL_ARD 101849.1124
#define CAL_AD 104068.0473
// Offset d'ajustement
#define OFFSET_AG 8262549
#define OFFSET_ARG 8450057
#define OFFSET_ARD 8394223
#define OFFSET_AD 8400000          // ajusté à la main (8441515 à l'origine)

/* Module A7670E */
#define MODEM_RX 10
#define MODEM_TX 9
#define MODEM_PWR 3
#define MODEM_BAUD 115200
#define MODEM_APN "free"
#define SECONDES_PAR_JOUR 86400L
#define SECONDES_PAR_HEURE 3600L
#define SYNC_INTERVALLE_S 2 * SECONDES_PAR_HEURE    // resynchroniser toutes les 2h
#define SYNC_RETRY_S 3600UL                         // après un échec, réessayer dans 1h
#define HEURE_SMS_QUOTIDIEN 18                      // Heure d'envoi du SMS
#define MINUTE_SMS_QUOTIDIEN 0
#define SECOURS_DELAI_S 86400L                      // sans heure valide, envoyer le rapport toutes les 24 h

/* Buzzer */
#define buzzerPin 11

/* Batterie */
#define batteriePin A0
#define BATT_ADC_0 649      // 0 %
#define BATT_ADC_100 802      // 100 %
#define nombreMesuresBat 10   // Nombre de mesure de l'adc pour estimer le pourcentage de la batterie