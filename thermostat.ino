/* 
  Project de pilotage de themrostat :
  - Ordres par SMS avec accusee de reception
  - Contrele de bonne connexion, reception et envoi par diode RGB
  
 Elements necessaires:
 - 4X 220 ohm resistor
 - 1X 4N35 optocoupler
 - 1X TMP36 : capteur de temperature
 - 

  Note: Les pins suivants doivent etre utilises par la SIM800L
  pin 11   // tx pin
  pin 10   // rx pin

 Changelog:
 - V1 - 21/08/2017 - JGN: initiation du code, possibilite unique de monter la temperature (bouton "^" uniquement hacker)
 */

// Importation des librairies
#include <Sim800L.h>
#include <Debug.h>
#include <SoftwareSerial.h>

#define DEBUG

// -------> Hard part
// Optocoupleur
const int optoPin = 8; // the pin the optocoupler is connected to

// Diode RGB
const int greenLEDPin = 6;
const int redLEDPin = 3;
const int blueLEDPin = 5;

// Capteur de temperatur
const int sensorPin = A0;
// temperature de reference (a calibrer)
const float baselineTemp = 20.0;

Sim800L GSM(10, 11);
//SoftwareSerial Serial;

// -------> Soft part
const uint8_t INDEX = 1;       // index du message a recuperer
const uint8_t FREQ_APPUI = 15; // delai entre chaque appui sur le bouton du thermostat
const String SIM_PIN_CODE = String("0000");
const String TEL_NUMBER = "+33670565262"; // format internationnal, numero de la carte SIM embarquee
const char SEPARATOR = ':';
// definition des ordres SMS acceptables
const String GET_TEMP_ORDER = "recup temp";
const String SET_TEMP_ORDER = "augm temp " + SEPARATOR;
// definition des messages de ACK
char *ACK_GET_TEMP = "Demande de recup de temperature bien recue";
String RESP_GET_TEMP = "Temperature actuelle : ";
char *ACK_SET_TEMP = "Demande d'augmentation de temperature bien recue";
char *CDE_KO = "Demande non comprise";

// ----------------------
// --------------> SETUP
// ----------------------

void setup()
{
  DBG_PRINT("Debut setup...");
  // ---> Hard part
  // make the pin with the optocoupler an output
  pinMode(optoPin, OUTPUT);

  // set the digital pins as outputs
  pinMode(greenLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(blueLEDPin, OUTPUT);
  // initialisation a LOW des pins
  digitalWrite(greenLEDPin, LOW);
  digitalWrite(redLEDPin, LOW);
  digitalWrite(blueLEDPin, LOW);
  // ---> Soft part
  // Demarrage du modem
  GSM.begin(4800);
  GSM.reset();
  //don't forget to catch the return of the function delAllSms!
  bool error = GSM.delAllSms(); //clean memory of sms;
  DBG_PRINT("suppression de tous les sms : " + error);
  delay(500); // Delai pour la connexion
  // specification du code pin
  bool pinSetted = GSM.setPIN(SIM_PIN_CODE);
  DBG_PRINT("Pin set ok :" + pinSetted);
  // Initialisation de la communication serie std
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  DBG_PRINT("Fin du setup.")
}

// ----------------------
// --------------> LOOP
// ----------------------

void loop()
{
  DBG_PRINT("Debut loop...");
  while (Serial.available() > 0)
  {
    // affichage dans le moniteur serie des commandes AT + reponse
    String contenu;
#ifndef DEBUG
    // On lit le sms restant
    contenu = GSM.readSms(INDEX);
#else
    // pour emuler via le monitor
    Serial.print("Donner le contenu du SMS : ");
    contenu = Serial.readStringUntil('\n');
#endif
    if (contenu.indexOf("OK") != -1) //first we need to know if the messege is correct. NOT an ERROR
    {
      if (contenu.length() > 7) // optional you can avoid SMS empty
      {
        // On parse le message
        parseSMS(contenu, GSM.getNumberSms(INDEX));
      }
      GSM.delAllSms();
    }
  }
  delay(1000); // attente de 1 sec
  DBG_PRINT("Fin loop...");
}

// ---------------------------
// ---------------> FONCTIONS
// ---------------------------

// TODO: ajouter un ordre d'ajout de numero autorise (callee) avec stockage EEPROM
void parseSMS(String message, String telNum)
{
  // Attention on alloue dynamiquement la memoire : desalloc obligatoire en fin de fonction
  char *tel = copy(telNum.c_str());
  bool retour = true;
  // On parse le contenu du sms
  message.toLowerCase();
  message.trim();
  if (message.indexOf(GET_TEMP_ORDER) != -1)
  {
    DBG_PRINT("Cde recup temp recue:" + message);
    int currentTemp = readTemp();
    DBG_PRINT("Temperature captee = " + currentTemp);
    char *repTemp = copy((RESP_GET_TEMP + currentTemp + "°C").c_str());
    retour = GSM.sendSms(tel, repTemp);
    delete[] repTemp;
    DBG_PRINT("Retour de l'envoi SMS : " + retour);
  }
  else if (message.indexOf(SET_TEMP_ORDER) != -1)
  {
    DBG_PRINT("Cde augment temp recue:" + message);
    // ordre d'augmentation de temperature
    // TODO: conversion du nbre de degres d'augment en nbre appuis de touche "+" a inserer
    // TODO: mettre du debug sur serials
    int depart = message.lastIndexOf(SEPARATOR);
    int longueur = message.length();
    message.substring(depart, longueur - 1).trim();
    int augment = message.toInt();
    pushButton(augment);
    retour = GSM.sendSms(tel, ACK_SET_TEMP);
    DBG_PRINT("Retour de l'envoi SMS : " + retour);
  }
  else
  {
    // commande SMS non prise en charge
    DBG_PRINT("Cde non reconnue:" + message);
    retour = GSM.sendSms(tel, CDE_KO);
    DBG_PRINT("Retour de l'envoi SMS : " + retour);
  }
  if (!retour)
    DBG_PRINT("Pb envoi SMS : " + message);
  delete[] tel;
}

// TODO: implementer un vrai filtre de N° autorises avec liste dans EEPROM
boolean authorizedCallee()
{
  return true;
}

int readTemp()
{
  // read the value on AnalogIn pin 0
  // and store it in a variable
  int sensorVal = analogRead(sensorPin);

  // send the 10-bit sensor value out the serial port
  DBG_PRINT("sensor Value: ");
  DBG_PRINT(sensorVal);

  // convert the ADC reading to voltage
  float voltage = (sensorVal / 1024.0) * 5.0;

  // Send the voltage level out the Serial port
  DBG_PRINT(", Volts: ");
  DBG_PRINT(voltage);

  // convert the voltage to temperature in degrees C
  // the sensor changes 10 mV per degree
  // the datasheet says there's a 500 mV offset
  // ((volatge - 500mV) times 100)
  DBG_PRINT(", degrees C: ");
  float temperature = (voltage - .5) * 100;
  DBG_PRINT(temperature);
  return temperature;
}

void pushButton(int nbAppuis)
{
  for (int iter = 0; iter < nbAppuis; iter++)
  {
    // Simulation d'appuis sur le bouton du thermostat
    digitalWrite(optoPin, HIGH); // pull pin 2 HIGH, activating the optocoupler
    delay(FREQ_APPUI);           // give the optocoupler a moment to activate
    digitalWrite(optoPin, LOW);  // pull pin 2 low until you're ready to activate again
    delay(FREQ_APPUI);
  }
}

char *copy(const char *orig)
{
  char *res = new char[strlen(orig) + 1];
  strcpy(res, orig);
  return res;
}

/*
      
      String readSms()
      {
        initRecepSms();
        String rawContent = sim800l.read() return rawContent;
      }

      void initRecepSms()
      {
        sim800l.print("AT+CMGF=1\r"); // Configure le mode SMS
        // Affiche tous les messages
        // pour le mode debug
        //sim800l.print("AT+CMGL=\"REC UNREAD\"\r");
        sim800l.print("AT+CMGL=\"ALL\"\r");
        delay(1000);
        sim800l.println();
      }

      void initSendSms()
      {
        Serial.println("Sending text message...");
        sim800l.print("AT+CMGF=1\r"); // Lance le mode SMS
        delay(100);
        // Entrez votre numero de telephone
        sim800l.print("AT+CMGS=\"" + TEL_NUMBER + "\"\r");
        delay(100);
        // Entrez votre message ici
        sim800l.print("Message ici \r");
        // CTR+Z en langage ASCII, indique la fin du message
        sim800l.print(char(26));
        delay(100);
        sim800l.println();
        Serial.println("Text send"); // Le message est envoye.
      }
*/

#ifdef DEBUG
#else
#endif
