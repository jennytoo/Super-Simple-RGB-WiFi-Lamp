#include <Arduino.h>
#include "WifiModule.h"
#include "globals.h"

WifiModule::WifiModule(String defaultName) : accessPointIP(192, 168, 1, 1), captivePortalDNS()
{
  wifiStarting = false;
  softApStarted = false;
  this->defaultName = defaultName;
  programmedSSID = SSID;
  programmedPassword = Password;
}

void WifiModule::setup()
{
  // Make sure the wifi does not autoconnect but always reconnects
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(true);

  // Set callbacks for connection and disconnection of wifi
  stationConnectedHandler = WiFi.onStationModeGotIP(std::bind(&WifiModule::onWifiConnected, this, std::placeholders::_1));
  stationDisconnectedHandler = WiFi.onStationModeDisconnected(std::bind(&WifiModule::onWifiDisconnected, this, std::placeholders::_1));
};

void WifiModule::loop()
{
  if (!WiFi.isConnected() && !wifiStarting && SSID != "")
  {
    // Set the Host name to the device name
    String hostName = Name != "" ? Name : defaultName;
    hostName.replace(" ", "-");

    // Disconnect the WS if connected
    webSocket.disconnect();

    // Disconnect the wifi - for stability
    WiFi.disconnect();
    WiFi.softAPdisconnect(false);

    // Start in softAP and Station Mode
    WiFi.mode(WIFI_AP_STA);

    // Setup the softap
    WiFi.softAPConfig(accessPointIP, accessPointIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(hostName);

    // Setup the station
    WiFi.hostname(hostName);
    (Password != "") ? WiFi.begin(SSID, Password) : WiFi.begin(SSID);

    // Set the starting boolean
    wifiStarting = true;

    // Set up captive DNS
    captivePortalDNS.start(53, "*", accessPointIP);
    captivePortalDNS.setErrorReplyCode(DNSReplyCode::NoError);

    // Debug
    Serial.println("[handleWifiConnection] - Attempting connection to \"" + SSID + "\"");
  }
  else if (!softApStarted && SSID == "")
  {
    // Set the Host name to the device name
    String hostName = Name != "" ? Name : defaultName;
    hostName.replace(" ", "-");

    // Debug
    Serial.println("[handleWifiConnection] - No SSID given starting the software AP");

    // Disconnect the WS if connected
    webSocket.disconnect();

    // Start the wifi in soft AP mode only
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(accessPointIP, accessPointIP, IPAddress(255, 255, 255, 0));
    if (Password != "")
    {
      WiFi.softAP(hostName, Password);
    }
    else
    {
      WiFi.softAP(hostName);
    }
    softApStarted = true;

    // Set up captive DNS
    captivePortalDNS.start(53, "*", accessPointIP);
    captivePortalDNS.setErrorReplyCode(DNSReplyCode::NoError);
  }

  // Handle the captive portal
  captivePortalDNS.processNextRequest();

  // Handle mDNS
  MDNS.update();
};

// mDNS Methods
void WifiModule::mDNSInit()
{
  // Create host name
  String hostName = Name != "" ? Name : DEFAULT_NAME;
  hostName.replace(" ", "-");

  // Try start the mDNS host
  if (MDNS.begin(hostName))
  {
    // Debug
    Serial.println("[startMdns] - Started MDNS responder at http://" + hostName + ".local/");

    // Add an mDNS service to the mDNS host
    if (!mdnsService)
    {
      mdnsService = MDNS.addService(0, "http", "tcp", 80);
      if (mdnsService)
        MDNS.addServiceTxt(mdnsService, "name", Name.c_str());
    }
    else
      Serial.println("[startMdns] - mDNS Service already started");
  }
  else
    Serial.println("[startMdns] - Failed to start MDNS responder as " + hostName + ".local");
}

void WifiModule::onWifiConnected(const WiFiEventStationModeGotIP &event)
{
  // Debug
  Serial.println();
  Serial.println("[onWifiConnected] - Connected to \"" + SSID + "\" as \"" + Name + "\" with an ip of " + WiFi.localIP().toString());
  Serial.println("[onWifiConnected] - Webserver avaialble at http://" + WiFi.localIP().toString() + "/");

  // unset the boolean
  wifiStarting = false;
  softApStarted = false;

  // Stop the capative DNS
  captivePortalDNS.stop();

  // Start mDNS here
  mDNSInit();

  // Stop the softAP
  WiFi.softAPdisconnect(true);
}

void WifiModule::onWifiDisconnected(const WiFiEventStationModeDisconnected &event)
{
  // Debug
  Serial.println("[onWifiConnected] - Disconnected from \"" + SSID + "\"");

  // unset the boolean
  wifiStarting = false;
}