#ifndef _NTPMODULE_H_
#define _NTPMODULE_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncUDP.h>
#include <IPAddress.h>
#include <TimeLib.h>
#include "AbstractModule.h"

class NTPModule : public AbstractModule
{
private:
    AsyncUDP udpClient;
    String ntpHostName = "pool.ntp.org";
    unsigned long utcOffset = 0;           // in seconds
    unsigned long collectionPeriod = 3600; // in seconds
    unsigned long nextSyncTimeMillis = 0;  // millis since start
    time_t lastSyncTime = 0;               // UNIX Epoch

    void buildNTPRequest(byte *_packetBuffer);
    bool sendNTPRequest();
    void onNTPResponse(AsyncUDPPacket packet);
    void parseNTPResponse(uint8_t *_ntpData);

public:
    NTPModule(unsigned long utcOffset);
    virtual void setup();
    virtual void loop();
};

bool getHostByName(const char *_ntpServerName, IPAddress &_ntpServerIp);
String get12hrAsString();
#endif