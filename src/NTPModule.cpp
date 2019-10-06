#include <Arduino.h>
#include "NTPModule.h"

/**
 * Minimal NTP Client.
 */

/**
 * NTP Client Module
 *
 * @param offset UTC Offset in seconds
 */
NTPModule::NTPModule(unsigned long offset) : utcOffset(offset){};

void NTPModule::setup()
{
  // No setup required; all work is done in #loop
}

void NTPModule::loop()
{
  // If enough time has passed we need to perform a resync
  if (millis() > nextSyncTimeMillis)
  {
    // Only attempt to update the time if the Wifi is connected
    if (WiFi.isConnected())
    {
      sendNTPRequest();
    }
    else
    {
      // Check again in 1 second
      nextSyncTimeMillis += 1000;
    }
  }
}

/**
 * Build an NTP Request Packet.
 *
 * Warning: The pointer passed is not verified. If insufficient memory is
 * allocated this will overwrite random memory. (buffer overflow)
 *
 * @param Pointer to byte array; must be at least 48 bytes long.
 */
void NTPModule::buildNTPRequest(byte *_packetBuffer)
{
  memset(_packetBuffer, 0, 48);
  // NTP is detailed in RFC5905 (https://tools.ietf.org/html/rfc5905)

  // Byte 0 is Leap Indicator, Protocol Version, and Move
  //   Bits 0-1 =  11 (3 = Leap Second Unknown; Clock Unsynchronized)
  //   Bits 2-4 = 100 (4 = NTPv4)
  //   Bits 5-7 = 011 (3 = NTP Client)
  _packetBuffer[0] = 0b11100011;
  // 1 is Stratum (0 = invalid, 1 = primary, 2-15 = econdary, 16 = unsynchronized, 17+ = reserved)
  _packetBuffer[1] = 0;    // Stratum, or type of clock
  _packetBuffer[2] = 6;    // Polling Interval
  _packetBuffer[3] = 0xEC; // System Clock Precision
  // 4-7 are Root Delay and may remain 0
  // 8-11 are Root Dispersion and may remain 0
  // 12-15 are Reference ID; a four character string identifying the time source.
  _packetBuffer[12] = 0x31;
  _packetBuffer[13] = 0x4E;
  _packetBuffer[14] = 0x31;
  _packetBuffer[15] = 0x34;
  // The remaining bytes are various timestamps and may be left at 0
  // 16-23 are Reference Timestamp (when the system clock was last set)
  // 24-31 are Origin Timestamp (time when sent by the client)
  // 32-39 are Receive Timestamp (time when received by the server)
  // 40-47 are Transmit Timestamp (time when sent by the server)

  // This request is not entirely in line with the RFC, but it's close enough.
  // Stratum should be 16 for "Unsynchronized"; 0 is unspecified/invalid but treated as 16.
  // The Reference ID is "1N14" which is undefined, and appears to be lost to history
  // for the reasoning, but the field is relatively free-form.
  // We also don't populate the Reference or Origin Timestamps, but they aren't needed
  // as no attempt is made to adjust for internet latency.
}

bool NTPModule::sendNTPRequest()
{
  IPAddress ipAddress;
  bool ipLookupSucceeded = getHostByName(ntpHostName.c_str(), ipAddress);

  // Schedule a sync in 5 seconds (after DNS lookup so we don't include DNS delays in the wait)
  nextSyncTimeMillis = millis() + 5000;

  if (ipLookupSucceeded)
  {
    // Attempt to connect to NTP Server
    if (udpClient.connect(ipAddress, 123))
    {
      // Initialize values needed to form NTP request
      byte packetBuffer[48];
      buildNTPRequest(packetBuffer);

      // Send packet to NTP server and setup response handler
      if (udpClient.write(packetBuffer, 48))
      {
        // Create a handler for the received packet
        udpClient.onPacket(std::bind(&NTPModule::onNTPResponse, this, std::placeholders::_1));

        // Return true on send message
        return true;
      }
      else
      {
        udpClient.close();
        Serial.println("[sendNTPRequest] - Message was not sent to NTP Server");
      }
    }
    else
      Serial.println("[sendNTPRequest] - Could not connect to NTP Server");
  }

  // Always return
  return false;
}

/**
 * Callback to handle the NTP response
 */
void NTPModule::onNTPResponse(AsyncUDPPacket packet)
{
  if (packet.length() >= 48)
  {
    parseNTPResponse(packet.data());
  }
  else
    Serial.println("[handleNTPResponse] - Received short NTP response");
}

/**
 * Parse an NTP response packet and set the current time.
 *
 * @param _ntpData Pointer to array of byte; must be at least 44 bytes long.
 */
void NTPModule::parseNTPResponse(uint8_t *_ntpData)
{
  // Bytes 40-43 are the remote server's Transmit Time in seconds since 1 Jan 1900, in network byte order.
  unsigned long secsSince1900;
  secsSince1900 = (unsigned long)_ntpData[40] << 24;
  secsSince1900 |= (unsigned long)_ntpData[41] << 16;
  secsSince1900 |= (unsigned long)_ntpData[42] << 8;
  secsSince1900 |= (unsigned long)_ntpData[43];

  // UNIX Epoch starts 1 Jan 1970 so we need to subtract 2208988800 seconds (70 years)
  lastSyncTime = secsSince1900 + utcOffset - 2208988800UL;

  // Set time in library
  setTime(lastSyncTime);

  // Schedule the next sync
  nextSyncTimeMillis = millis() + (collectionPeriod * 1000);

  // Close the connection
  udpClient.close();

  // Debug
  Serial.println("[parseNTPResponse] - Current time set to: " + get12hrAsString());
}

/**
 * Convenience function to lookup the IP of a host with debug logging.
 *
 * @param _hostName Hostname to be resolved
 * @param _hostIPAddress IPAddress structure to store the results
 * @return true on success, false otherwise
 */
bool getHostByName(const char *_hostName, IPAddress &_hostIPAddress)
{
  // Probe the DNS for the IP Address of the host. This call blocks up to 10s until a response arrives.
  if (!WiFi.hostByName(_hostName, _hostIPAddress))
  {
    Serial.println("[getHostByName] - Failed to lookup DNS results for host \"" + String(_hostName) + "\"");
    return false;
  }
  else
  {
    Serial.println("[getHostByName] - DNS result for \"" + String(_hostName) + "\" is " + _hostIPAddress.toString());
    return true;
  }
}

/**
 * Get time as a string
 *
 * @return Time string in the format HH:MM:SS AM/PM
 */
String get12hrAsString()
{
  String hrStr = hourFormat12() < 10 ? "0" + String(hourFormat12()) : String(hourFormat12());
  String minStr = minute() < 10 ? "0" + String(minute()) : String(minute());
  String secStr = second() < 10 ? "0" + String(second()) : String(second());
  return hrStr + ":" + minStr + ":" + secStr + " " + String(isAM() ? "AM" : "PM");
}
