#ifndef _WIFI_H_
#define _WIFI_H_

#include <Arduino.h>
#include <IPAddress.h>
#include <ESP8266WiFi.h>
#include "AbstractModule.h"

class WifiModule : public AbstractModule
{
private:
    bool wifiStarting;
    bool softApStarted;
    IPAddress accessPointIP;
    WiFiEventHandler stationConnectedHandler;
    WiFiEventHandler stationDisconnectedHandler;

    void onWifiConnected(const WiFiEventStationModeGotIP &event);
    void onWifiDisconnected(const WiFiEventStationModeDisconnected &event);

public:
    WifiModule();
    virtual void setup();
    virtual void loop();

    // Visible until a better solution is implemented

    /**
     * Hard-coded default values
     */
    String SSID = "";
    String Password = "";

    String programmedSSID;
    String programmedPassword;
};
#endif