/******************************************************************************
Arduino MKR WAN 1300 (LoRa connectivity)
SparkFun Si7021 Breakout 
Auteur : Cédric Goby
Licence : MIT
Versioning : https://github.com/CedricGoby/Arduino-MKR-WAN-1300

This sketch prints the temperature and humidity from a Si7021 Breakout sensor to the Serial port
and send the data to a LoRa gateway.

Hardware Connections for Si7021:
HTU21D ------------- Photon
(-) ------------------- GND
(+) ------------------- 3.3V (VCC)
CL ------------------- D1/SCL
DA ------------------- D0/SDA

/******************************************************************************
Si7021 Breakout
*******************************************************************************/
#include "SparkFun_Si7021_Breakout_Library.h"
#include <Wire.h>

float humidity = 0;
float tempf = 0;

int power = A3;
int GND = A2;

//Create Instance of HTU21D or SI7021 temp and humidity sensor and MPL3115A2 barrometric sensor
Weather sensor;

// declare three Strings:
String stringT, stringH, msg;

/******************************************************************************
MKR WAN 1300 LoRa module
*******************************************************************************/
#include <MKRWAN.h>
LoRaModem modem;

#include "arduino_secrets.h"
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;

//---------------------------------------------------------------
void setup()
{
    Serial.begin(115200);   // open serial over USB at 115200 baud
    
// LoRa    
    while (!Serial);
    // change this to your regional band (eg. US915, AS923, ...)
    if (!modem.begin(EU868)) {
      Serial.println("Failed to start module");
      while (1) {}
    };
    Serial.print("Your module version is: ");
    Serial.println(modem.version());
    Serial.print("Your device EUI is: ");
    Serial.println(modem.deviceEUI());

    int connected = modem.joinOTAA(appEui, appKey);
    if (!connected) {
      Serial.println("Something went wrong; are you indoor? Move near a window and retry");
      while (1) {}
  }

    // Set poll interval to 60 secs.
    modem.minPollInterval(60);
    // NOTE: independently by this setting the modem will
    // not allow to send more than one message every 2 minutes,
    // this is enforced by firmware and can not be changed.

// Si7021
    pinMode(power, OUTPUT);
    pinMode(GND, OUTPUT);

    digitalWrite(power, HIGH);
    digitalWrite(GND, LOW);

    //Initialize the I2C sensors and ping them
    sensor.begin();
}

//---------------------------------------------------------------
void loop()
{
    //Get readings from all sensors
    getWeather();
    //Print datas to serial port
    printInfo();
    //Send datas to LoRaWAN network
    sendInfo();
    delay(10000);
}

//---------------------------------------------------------------
void getWeather()
{
  // Measure Relative Humidity from the HTU21D or Si7021
  humidity = sensor.getRH();

  // Measure Temperature from the HTU21D or Si7021
  tempf = sensor.getTempF();
  // Temperature is measured every time RH is requested.
  // It is faster, therefore, to read it from previous RH
  // measurement with getTemp() instead with readTemp()
}

//---------------------------------------------------------------
void printInfo()
{
//This function prints the weather data out to the default Serial Port

  Serial.print("Temp:");
  Serial.print(tempf);
  Serial.print("F, ");

  Serial.print("Humidity:");
  Serial.print(humidity);
  Serial.println("%");
}

//---------------------------------------------------------------
void sendInfo()
{
  stringT = "T:";
  stringH = "H:";
  msg = stringT + tempf + " " + stringH + humidity;
  
  Serial.println();
  Serial.print("Sending: " + msg + " - ");
  for (unsigned int i = 0; i < msg.length(); i++) {
    Serial.print(msg[i] >> 4, HEX);
    Serial.print(msg[i] & 0xF, HEX);
    Serial.print(" ");
  }
  Serial.println();

  int err;
  modem.beginPacket();
  modem.print(msg);
  err = modem.endPacket(true);
  if (err > 0) {
    Serial.println("Message sent correctly!");
  } else {
    Serial.println("Error sending message :(");
    Serial.println("(you may send a limited amount of messages per minute, depending on the signal strength");
    Serial.println("it may vary from 1 message every couple of seconds to 1 message every minute)");
  }
}

