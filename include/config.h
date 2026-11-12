#pragma once

/*===========================*/
/* Fichier de configuration */
/*===========================*/

/* Bouton */
#define buttonPin 2

/* Capteurs de masse */
#define CLK 4
#define DT1 5
#define DT2 6
#define DT3 7
#define DT4 8
// Paramètres de mesure
#define nombreMesuresMasse 5 // Nombre de mesure pour estimer la masse
#define ALERT_DELTA_KG       0.1   // Seuil de variation : 0.1 kg = 100 g
#define ALERT_CONFIRMATIONS  2     // Nombre de mesures de confirmation
#define ALERT_CONFIRM_DELAY  5000 // Délai entre confirmations (ms)
#define DELAI_STABILISATION 3000 // Attente après allumage
// Facteurs de calibration
#define CAL_AG   103550.2959
#define CAL_ARG  104807.6923
#define CAL_ARD  101849.1124
#define CAL_AD   104068.0473
// Offset d'ajustement
#define OFFSET_AG   8262549
#define OFFSET_ARG  8450057
#define OFFSET_ARD  8394223
#define OFFSET_AD   8400000  // ajusté à la main (8441515 à l'origine)

/* Module A7670E */
#define MODEM_RX 10
#define MODEM_TX 9
#define MODEM_PWR 3
#define MODEM_BAUD 115200

/* Buzzer */
#define buzzerPin 11

/* Batterie */
#define batteriePin A0
#define BATT_ADC_0   649.56  // 0 %
#define BATT_ADC_100 802.39  // 100 %
#define nombreMesuresBat 10 // Nombre de mesure de l'adc pour estimer le pourcentage de la batterie