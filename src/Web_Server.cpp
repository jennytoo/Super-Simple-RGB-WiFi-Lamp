#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "globals.h"
#include "Website.h"

void otaInit();
void servePage();

// Webserver and OTA Objects
ESP8266WebServer restServer(80);
ESP8266HTTPUpdateServer OTAServer;

void webServerInit()
{
  // Set the URI's of the server
  restServer.onNotFound(servePage);
  restServer.on("/", servePage);
  restServer.begin();

  // Set up OTA on the server
  otaInit();

  // Debug
  Serial.println("[webServerInit] - Webserver was set up correctly");
}

void servePage()
{
  /*
   * This file is generated from Website.html
   *
   * Two methods to generate this file:
   *   1. Perform the search and replace below on Website.html
   *   2. Run Website.sh
   *
   * Search and replace:
   *   find " and replace   \"
   *   find ^ and replace     restServer.sendContent_P(PSTR("
   *   find $ and replace   \n"));
   *
   * As regex:
   *   s/"/\\"/g
   *   s/^/  restServer.sendContent_P(PSTR("/
   *   s/$/\\n"));/
   */

  // Debug
  Serial.println("[servePage] - Serving webpage");

  restServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  restServer.sendHeader("Pragma", "no-cache");
  restServer.sendHeader("Expires", "-1");
  restServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  restServer.send(200, "text/html", String(""));
  sendStaticBody(restServer);
  restServer.client().stop();

  // Scan for the networks in the area
  scanForNetworks();
}

void scanForNetworks()
{
  WiFi.scanNetworksAsync([&](int _networksFound) {
    updateWifiConfigTable(_networksFound);
  },
                         false);
}

void updateWifiConfigTable(int _numNetworks)
{
  // Insert the RSSI into map if it is high enough (Map automatically sorts ascending)
  std::map<int32_t, int> orderedRSSI;
  for (int i = 0; i < _numNetworks; ++i)
  {
    if (i == _numNetworks)
      Serial.println("[updateWifiConfigTable] - No Access Points Found");
    // else if	(WiFi.RSSI(i) < -90) Serial.println("[updateWifiConfigTable] - RSSI of " + WiFi.SSID(i) + " is too low at: " + String(WiFi.RSSI(i)));
    else
    {
      // Place access point reference into ordered map
      // Serial.println("[updateWifiConfigTable] - Adding " + WiFi.SSID(i) + " to RSSI Map");
      orderedRSSI[WiFi.RSSI(i)] = i;
    }
  }

  // Loop through each of the found networks and put in HTML table
  String wsMessage = "{\"Wifi\":{\"ScanResults\":[";
  for (std::map<int32_t, int>::reverse_iterator _orderedRSSI = orderedRSSI.rbegin(); _orderedRSSI != orderedRSSI.rend(); ++_orderedRSSI)
  {
    wsMessage += "{\"SSID\":\"" + WiFi.SSID(_orderedRSSI->second) + "\",";
    wsMessage += "\"BSSID\":\"" + WiFi.BSSIDstr(_orderedRSSI->second) + "\",";
    wsMessage += "\"CHANNEL\":\"" + String(WiFi.channel(_orderedRSSI->second)) + "\",";
    wsMessage += "\"RSSI\":\"" + String(WiFi.RSSI(_orderedRSSI->second)) + "\",";
    wsMessage += "\"ENCRYPT\":\"" + String((WiFi.encryptionType(_orderedRSSI->second) == ENC_TYPE_NONE) ? "No" : "Yes") + "\"";
    wsMessage += "}";
    wsMessage += _orderedRSSI != --orderedRSSI.rend() ? "," : "";
  }
  wsMessage += "]}}";

  // Send if the client is connected
  if (webSocket.connectedClients(false))
    webSocket.broadcastTXT(wsMessage);

  // Debug
  Serial.println("[updateWifiConfigTable] - Number of Valid Networks Sent was: " + String(orderedRSSI.size()));
}

void otaInit()
{
  OTAServer.setup(&restServer, "/update");
}
