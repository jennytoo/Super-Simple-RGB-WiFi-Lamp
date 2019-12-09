// Included Libraries
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <Arduino.h>
//#include <FastLED.h>
#include "FS.h"
//#include <ESP8266WiFi.h>
//#include "IPAddress.h"
//#include <DNSServer.h>
//#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
//#include <ESPAsyncTCP.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
//#include <TimeLib.h>
//#include <ESPAsyncUDP.h>
//#include "lwip/inet.h"
//#include "lwip/dns.h"

#include "globals.h"
#include "NTPModule.h"

String Name = DEFAULT_NAME; // The default Name of the Device

bool spiffsCorrectSize = false;

WifiModule *wifiModule = new WifiModule(DEFAULT_NAME);
NTPModule *ntpModule = new NTPModule(UTC_OFFSET * 3600);

// Setup Method - Runs only once before the main loop. Useful for setting things up
void setup()
{
  // Add a short delay on start
  delay(1000);

  // Start Serial
  //Serial.begin(115200);
  Serial.begin(9600);
  Serial.println();

  // Check if the flash has been set up correctly
  spiffsCorrectSize = checkFlashConfig();
  if (spiffsCorrectSize)
  {
    // Init the LED's
    ledStringInit();

    // Get saved settings
    getConfig();

    // Start Wifi
    wifiModule->setup();

    // Setup Webserver
    webServerInit();

    // Setup websockets
    websocketsInit();
  }
  else
    Serial.println("[setup] -  Flash configuration was not set correctly. Please check your settings under \"tools->flash size:\"");
}

// The Main Loop Method - This runs continuously
void loop()
{
  // Check if the flash was correctly setup
  if (spiffsCorrectSize)
  {
    // Serial.println("Loop Time: " + String(millis()));

    // Handle the webserver
    restServer.handleClient();

    // Handle Websockets
    webSocket.loop();

    // Get the time when needed
    ntpModule->loop();

    // Update WS clients when needed
    updateClients();

    // Handle the wifi connection
    wifiModule->loop();

    // Update the LED's
    handleMode();
  }
  else
  {
    delay(10000);
    Serial.println("[loop] - Flash configuration was not set correctly. Please check your settings under \"tools->flash size:\"");
  }
}
