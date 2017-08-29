# Application de thermostat connecté

## Objectifs
Permettre de piloter le réseau basse tension distant d'un thermostat de chaudière via une connection distante 2G (SMS)

## Composantes du projet
- **SIM800L** : émetteur récepteur 2G GPRS sur port série
- **LED monochrome** : permet de signaler le bon comportement du montage ou des éventuels problèmes
- **2x Résistances 220 ohms**
- **TMP36** : capteur de température
- **Optocoupleur 4N35** : Permet de faire le pilotage du réseau basse tension du thermostat à partir du montage. Permet l'isolation galvanique des deux circuits.
- **Arduino UNO** : coeur du montage en attendant une version ATtiny85 (raison de coût)

## Schéma du montage
[![](https://www.tinkercad.com/things/bOKf7TTl9bY-thermostat/editel?sharecode=k6dUCqKOuqLPY_ALAoaU46QItPdpttJL33RTVWE1nWM=)](https://www.tinkercad.com/things/bOKf7TTl9bY-thermostat/editel?sharecode=k6dUCqKOuqLPY_ALAoaU46QItPdpttJL33RTVWE1nWM=)

[//]: # (Un commentaire)