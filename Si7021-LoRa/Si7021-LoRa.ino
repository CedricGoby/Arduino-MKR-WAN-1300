/******************************************************************************
Si7021-LoRa
Auteur : Cédric Goby
Licence : MIT
Versioning : https://github.com/CedricGoby/Arduino-MKR-WAN-1300

Ce croquis recueille les données d'un capteur Si7021 connecté à un Arduino MKR WAN1300.
Les données sont affichées via le port série et envoyées vers un réseau LoRa.

Connexions matériel pour le Si7021:
HTU21D ------------- Photon
(-) ------------------- GND
(+) ------------------- 3.3V (VCC)
CL ------------------- D1/SCL
DA ------------------- D0/SDA

/******************************************************************************
SparkFun Si7021 Breakout - https://github.com/sparkfun/Si7021_Breakout 
*******************************************************************************/
#include "SparkFun_Si7021_Breakout_Library.h"
#include <Wire.h>

float humidity = 0;
float tempf = 0;

int power = A3;
int GND = A2;

// Crée une instance de HTU21D ou SI7021 et MPL3115A2
Weather sensor;

// Déclaration de trois Strings pour la mise en forme du message
String stringT, stringH, msg;

/******************************************************************************
Module LoRa Arduino MKR WAN 1300 - https://www.arduino.cc/en/Tutorial/MKRWANFirstConfiguration
*******************************************************************************/
#include <MKRWAN.h>
LoRaModem modem;
// Décommentez si vous utilisez une puce Murata comme module
// LoRaModem modem(Serial1);

// Fichier contenant les identifiants LoRa sous la forme
//#define SECRET_APP_EUI "Votre EUI"
//#define SECRET_APP_KEY "Votre APP KEY"
#include "arduino_secrets.h"
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;

//---------------------------------------------------------------
void setup()
{
    Serial.begin(115200);   // ouvre le port série USB à 115200 baud
    
// Setup LoRa    
    while (!Serial);
    // Vérification du démarrage du module LoRa.
    if (!modem.begin(EU868)) {
      Serial.println("Echec du démarrage du module");
      while (1) {}
    };
    
    // Affichage de la version du firmware (MKRWAN) et EUI
    Serial.print("La version de votre module est: ");
    Serial.println(modem.version());
    Serial.print("L'EUI de l'appareil est: ");
    Serial.println(modem.deviceEUI());

    // Connexion au réseau LoRa en mode OTAA
    int connected = modem.joinOTAA(appEui, appKey);
    if (!connected) {
      Serial.println("Un problème est survenu; êtes vous à l'intérieur? Déplacez-vous près d'une fenêtre et réessayez");
      while (1) {}
  }

// Setup Si7021
    pinMode(power, OUTPUT);
    pinMode(GND, OUTPUT);

    digitalWrite(power, HIGH);
    digitalWrite(GND, LOW);

    // Initialise les capteurs I2C et les ping
    sensor.begin();
}

//---------------------------------------------------------------
void getWeather()
{
  // Mesure l'humidité relative depuis le HTU21D ou le Si7021
  humidity = sensor.getRH();

  // Mesure la température depuis le HTU21D ou le Si7021
  tempf = sensor.getTempF();
  // La température est mesurée chaque fois que l'humidité relative est demandée.
  // Il est plus rapide, toutefois, de la lire depuis la mesure d'humidité relative précédente
  // avec getTemp() plutôt qu'avec readTemp()
}

//---------------------------------------------------------------
void printInfo()
{
// Cette fonction envoie les mesures vers le port série par défaut

  Serial.print("Temp:");
  Serial.print(tempf);
  Serial.print("F, ");

  Serial.print("Hum:");
  Serial.print(humidity);
  Serial.println("%");
}

//---------------------------------------------------------------
void sendInfo()
{
// Cette fonction envoie les mesures vers un réseau LoRa

  // Mise en forme du message
  stringT = "T:";
  stringH = "H:";
  msg = stringT + tempf + " " + stringH + humidity;

  // Envoi du message vers le port série
  Serial.println();
  Serial.print("Envoi: " + msg + " - ");
  // Conversion hexadecimale
  for (unsigned int i = 0; i < msg.length(); i++) {
    Serial.print(msg[i] >> 4, HEX);
    Serial.print(msg[i] & 0xF, HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Envoi du message vers le réseau LoRa
  int err;
  modem.beginPacket();
  modem.print(msg);
  err = modem.endPacket(true);
  // Vérification
  if (err > 0) {
    Serial.println("Message bien envoyé :-)");
  } else {
    Serial.println("Erreur lors de l'envoi du message :-(");
    Serial.println("(Pensez à envoyer un nombre limité de messages par minute en fonction de la force du signal");
    Serial.println("la fréquence d'envoi peut varier d'un message toutes les quelques secondes à un message par minute.)");
  }
}

//---------------------------------------------------------------
void loop()
{
// Programme principal

    // Lit les valeurs de tous les capteurs
    getWeather();
    
    // Ecrit les données sur le port série
    printInfo();
    
    // Envoie les données sur le réseau LoRa
    sendInfo();
    
    // Pause de X millisecondes
    delay(30000);
}


