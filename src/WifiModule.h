#ifndef _WIFI_H_
#define _WIFI_H_

#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <IPAddress.h>
#include "AbstractModule.h"

class WifiModule : public AbstractModule
{
private:
    bool wifiStarting;
    bool softApStarted;
    IPAddress accessPointIP;
    WiFiEventHandler stationConnectedHandler;
    WiFiEventHandler stationDisconnectedHandler;
    DNSServer captivePortalDNS;
    MDNSResponder::hMDNSService mdnsService; // Belongs in the service module, not here

    void onWifiConnected(const WiFiEventStationModeGotIP &event);
    void onWifiDisconnected(const WiFiEventStationModeDisconnected &event);
    void mDNSInit(void);

public:
    WifiModule(String defaultName);
    virtual void setup();
    virtual void loop();

    // Visible until a better solution is implemented

    String defaultName;

    /**
     * Hard-coded default values
     */
    String SSID = "";
    String Password = "";

    String programmedSSID;
    String programmedPassword;
};
#endif