#ifndef _GLOBALS_H_
#define _GLOBALS_H_

// Included Libraries
//#define FASTLED_ESP8266_RAW_PIN_ORDER
//#include <Arduino.h>
//#include <FastLED.h>
//#include "FS.h"
//#include <ESP8266WiFi.h>
//#include "IPAddress.h"
//#include <DNSServer.h>
//#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
//#include <ESPAsyncTCP.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
#include <ESPAsyncUDP.h>
//#include "lwip/inet.h"
//#include "lwip/dns.h"

#include "WifiModule.h"
extern WifiModule *wifiModule;

void scanForNetworks();
void websocketSend(JsonDocument &jsonMessage);
void mdnsInit();
bool checkFlashConfig();
void ledStringInit();
void getConfig();
void websocketsInit();
void webServerInit();
void handleMode();
void handleNTP();
void updateClients();
void updateWifiConfigTable(int _numNetworks);
void parseConfig(JsonDocument &jsonMessage, bool sendViaWebsockets);
bool sendConfigViaWS();

// ############################################################# Sketch Variables #############################################################
// All variables at the top of this sketch need to be defined correctly for your light. Read the comments around each one for moe details on
// what each of them are.

#define DEFAULT_NAME "Super Simple RGB Wifi Lamp"

// Set Your Data pin -This is the pin on your ESP8266 that is connected to the LED's. Be careful as on the NodeMCU the D pin does not map to
// pin number. For this example pin D1 on the NodeMCU is actually pin 5 in software.
#define DATA_PIN 5

// Set the number of LED's - Simply count how many there are on your string and enter the number here
#define NUM_LEDS 66

// Set your UTC offset - This is the time zone you are in. for example +10 for Sydney or -4 for NYC
#define UTC_OFFSET +10

// Set up LED's for each side - These arrays hold which leds are on what sides. For the basic rectangular shape in the example this relates to 4
// sides and 4 arrays. You must subract 1 off the count of the LED when entering it as the array is 0 based. For example the first LED on the
// string is entered as 0.
extern int topLeds[];
extern int bottomLeds[];
extern int leftLeds[];
extern int rightLeds[];

// mDNS Objects
//extern MDNSResponder::hMDNSService mdnsService;

// Webserver and OTA Objects
extern ESP8266WebServer restServer;
extern ESP8266HTTPUpdateServer OTAServer;

// Web Sockets Variabels and Objects
extern WebSocketsServer webSocket;

// NTP Variables and Objects
extern bool ntpTimeSet;
extern String ntpHostName;
extern IPAddress ntpIpAddress;
extern unsigned long utcOffset;
extern unsigned long collectionPeriod;
extern unsigned long currentEpochTime;
extern unsigned long lastNTPCollectionTime;

// LED string object and Variables
extern int topNumLeds;
extern int bottomNumLeds;
extern int leftNumLeds;
extern int rightNumLeds;

// Base Variables of the Light
extern String Name;
extern String Mode;
extern bool State;
extern int FadeTime;
extern String currentMode;
extern String previousMode;
extern bool previousState;
extern float modeChangeFadeAmount;

// Colour Mode Variables
extern int colourRed;
extern int colourGreen;
extern int colourBlue;

// Rainbow Mode Variables
extern int rainbowStartHue;
extern int rainbowSpeed;
extern int rainbowBri;
extern float rainbowAddedHue;

// Clock Mode Variables
extern int clockHourRed;
extern int clockHourGreen;
extern int clockHourBlue;
extern int clockMinRed;
extern int clockMinGreen;
extern int clockMinBlue;
extern int clockOnPauseBrightness;

// Bell Curve Mode Variables
extern int bellCurveRed;
extern int bellCurveGreen;
extern int bellCurveBlue;

// Night Rider Mode Variables
extern int nightRiderTopLedNumber;
extern int nightRiderBottomLedNumber;
extern int nightRiderTopIncrement;
extern int nightRiderBottomIncrement;
#endif