#include <Arduino.h>
#include <LEDModeFireflies.h>

/*
 * LED Fireflies mode
 */
LEDModeFireflies::LEDModeFireflies(struct CRGB *data) : ledString(data){};

void LEDModeFireflies::setMinimumFlashDelay(unsigned int value)
{
    minimumFlashDelay = value;
}

void LEDModeFireflies::setMaximumFlashDelay(unsigned int value)
{
    maximumFlashDelay = value;
}

void LEDModeFireflies::setFlashLength(int value)
{
    flashLength = constrain(value, 0, 32767);
}

void LEDModeFireflies::setHue(int value)
{
    hue = constrain(value, 0, 255);
}

void LEDModeFireflies::setBrightness(int value)
{
    brightness = constrain(value, 0, 255);
}

void LEDModeFireflies::reset()
{
    unsigned long now = millis();
    for (int i = 0; i < NUM_LEDS; i++)
    {
        nextFlash[i] = now + flashDelay();
    }
}

void LEDModeFireflies::loop()
{
    unsigned long now = millis();
    long halfFlashLength = flashLength / 2;
    long flashTime;
    int value;
    for (int i = 0; i < NUM_LEDS; i++)
    {
        flashTime = now - nextFlash[i];
        if (flashTime < 0)
        {
            continue;
        }
        else if (flashTime > flashLength)
        {
            nextFlash[i] = now + flashDelay();
            value = 0;
        }
        else if (flashTime > halfFlashLength)
        {
            value = ease8InOutApprox(255 * (flashLength - flashTime) / halfFlashLength);
        }
        else
        {
            // Brightness peaks in the very middle
            //int value = cos(2 * PI * flashTime / flashLength) * -127 + 127;

            // Bright peaks at 33% of the duration then falls off slowly
            //int value = cos(2 * PI * pow(flashLength - flashTime, 2) / pow(flashLength, 2)) * -127 + 127;

            // Ease Curve which peaks at the end
            value = ease8InOutApprox(255 * flashTime / halfFlashLength);
        }
        ledString[i] = CHSV(hue, 255, value);
    }
    FastLED.setBrightness(brightness);
}

unsigned int LEDModeFireflies::flashDelay()
{
    return random16(minimumFlashDelay, maximumFlashDelay);
}