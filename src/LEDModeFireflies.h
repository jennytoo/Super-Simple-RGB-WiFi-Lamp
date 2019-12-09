#ifndef _LEDMODEFIREFLIES_H_
#define _LEDMODEFIREFLIES_H_

#include <Arduino.h>
#include <FastLED.h>
#include <globals.h>

class LEDModeFireflies
{
private:
    struct CRGB *ledString;
    unsigned long nextFlash[NUM_LEDS];

    unsigned int minimumFlashDelay = 1000; // in milliseconds
    unsigned int maximumFlashDelay = 5000; // in milliseconds
    int flashLength = 2500;                // in milliseconds
    uint8_t brightness = 255;
    uint8_t hue = 160;

    unsigned int flashDelay();

public:
    LEDModeFireflies(struct CRGB *ledString);
    void reset();
    void loop();

    // Temporary access to set values
    void setMinimumFlashDelay(unsigned int value);
    void setMaximumFlashDelay(unsigned int value);
    void setFlashLength(int value);
    void setHue(int value);
    void setBrightness(int value);
};

#endif