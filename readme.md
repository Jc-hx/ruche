# Ruche - Novembre 2026

Mesure de la masse d'une ruche et envoi de sms quotidien.

## 1. Squelette du projet
- [ ] Dépôt git initialisé
- [ ] `platformio.ini` (carte, framework, `lib_deps`, vitesse du moniteur série)
- [ ] `.gitignore` (dont `secrets.h` et `.pio/`)
- [ ] `include/config.h` : broches, heure d'envoi, seuil d'alerte (~100 g), fenêtre de temps, seuil batterie, calibrations en `TODO`
- [ ] `include/secrets.h` + `secrets.h.example` (numéro admin)
- [ ] Fichiers vides créés : `main`, `scale`, `modem`, `users`, `commands`
- [ ] Le projet compile à vide
- [ ] Premier commit

## 2. Balance (`scale`)
- [ ] Lecture des 4 HX711
- [ ] Offset et facteur d'échelle par cellule (constantes de `config.h`)
- [ ] Masse totale en grammes
- [ ] Filtrage (médiane ou moyenne de plusieurs lectures)
- [ ] Tare
- [ ] Mise en veille des HX711 (`power_down` / `power_up`)
- [ ] Test : la masse s'affiche sur le moniteur série

## 3. Modem (`modem`)
- [ ] Initialisation et attente de l'enregistrement réseau
- [ ] `sendSms(numéro, texte)`
- [ ] Lecture d'un SMS entrant (numéro + texte) et suppression après lecture
- [ ] Qualité du signal
- [ ] Gestion des erreurs et timeouts (pas de blocage infini)
- [ ] Test : envoi d'un SMS à ton téléphone, puis réception d'un SMS

## 4. Utilisateurs (`users`)
- [ ] Structure d'un utilisateur : numéro, admin, alertes actives, fin de pause
- [ ] Sauvegarde et lecture en EEPROM
- [ ] Ajout, suppression, liste
- [ ] « Doit-on alerter ce numéro maintenant ? » (tient compte de la pause)
- [ ] Numéro admin par défaut chargé depuis `secrets.h` au premier démarrage
- [ ] Test : les données survivent à un redémarrage

## 5. Commandes SMS (`commands`)
- [ ] Numéros inconnus ignorés
- [ ] `STATUS` : masse, batterie, signal
- [ ] `PAUSE 24H` / `REPRISE`
- [ ] `AJOUT` / `SUPPR` / `LISTE` (admin uniquement)
- [ ] `TARE` (tous les utilisateurs enregistrés)
- [ ] Commande inconnue : réponse d'aide courte
- [ ] Réponse envoyée uniquement à l'expéditeur
- [ ] Test de chaque commande depuis un numéro admin et un numéro simple

## 6. Logique principale (`main`)
- [ ] Mesure de la batterie (pont diviseur) et pourcentage
- [ ] Bouton : anti-rebond et rapport manuel
- [ ] Rapport (masse, batterie, signal), envoyé aux utilisateurs concernés
- [ ] Alerte de variation ~100 g dans la fenêtre de temps (historique des masses)
- [ ] Alerte de batterie faible
- [ ] Délai anti-répétition des alertes
- [ ] Test : poser et retirer une masse de ~100 g déclenche l'alerte

## 7. Heure fixe (sans RTC)
- [ ] Activation de la mise à jour de l'heure réseau (`AT+CTZU=1`) et lecture (`AT+CCLK?`)
- [ ] Conversion de l'heure en secondes depuis minuit
- [ ] Calcul du temps restant avant l'heure d'envoi
- [ ] Sommeil par tranches de ~80 % du temps restant, puis relecture de l'heure
- [ ] Fin de cycle en tranches courtes jusqu'à l'envoi
- [ ] Date du dernier envoi en EEPROM (pas de doublon après reboot)
- [ ] Repli si l'heure réseau est indisponible (estimation locale)
- [ ] Test : le SMS part à l'heure voulue, plusieurs jours de suite

## 8. Deep sleep
- [ ] Sommeil `power-down` via watchdog (`LowPower` ou `avr/sleep.h`)
- [ ] Réveil par bouton (D2) et par broche RI du modem (D3)
- [ ] Réveil watchdog pour la mesure périodique
- [ ] Mise en veille du modem entre deux usages
- [ ] Identification de la cause du réveil
- [ ] Test : consommation mesurée en veille

## Finition
- [ ] Test de bout en bout : un cycle complet (rapport quotidien, alerte, SMS entrant, bouton)
- [ ] Test de coupure de courant : les utilisateurs et la date du dernier envoi sont conservés
- [ ] Test de réseau absent : pas de blocage, nouvelle tentative plus tard
- [ ] Calibration réelle des 4 cellules : valeurs renseignées dans `config.h`
- [ ] README : câblage, commandes SMS, installation
- [ ] Tag git `v1.0`
