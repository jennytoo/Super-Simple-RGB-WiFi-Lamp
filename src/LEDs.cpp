#include <Arduino.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include "globals.h"

// Set up LED's for each side - These arrays hold which leds are on what sides. For the basic rectangular shape in the example this relates to 4
// sides and 4 arrays. You must subract 1 off the count of the LED when entering it as the array is 0 based. For example the first LED on the
// string is entered as 0.
int topLeds[] = {18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};
int bottomLeds[] = {14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51};
int leftLeds[] = {48, 49, 50};
int rightLeds[] = {15, 16, 17};

// LED string object and Variables
CRGB ledString[NUM_LEDS];
int topNumLeds = sizeof(topLeds) / sizeof(*topLeds);
int bottomNumLeds = sizeof(bottomLeds) / sizeof(*bottomLeds);
int leftNumLeds = sizeof(leftLeds) / sizeof(*leftLeds);
int rightNumLeds = sizeof(rightLeds) / sizeof(*rightLeds);

String Mode = "";               // The default Mode of the Device
bool State = true;              // The Default Mode of the Light
int FadeTime = 200;             // Fading time between states in ms
String currentMode = Mode;      // Placeholder variable for changing mode
String previousMode = "";       // Placeholder variable for changing mode
bool previousState = false;     // Placeholder variable for changing state
float modeChangeFadeAmount = 0; // Place holder for global brightness during mode change

// Colour Mode Variables
int colourRed = 128;
int colourGreen = 128;
int colourBlue = 128;

// Rainbow Mode Variables
int rainbowStartHue = 0;
int rainbowSpeed = 10;
int rainbowBri = 100;
float rainbowAddedHue = 0;

// Clock Mode Variables
int clockHourRed = 128;
int clockHourGreen = 128;
int clockHourBlue = 128;
int clockMinRed = 128;
int clockMinGreen = 128;
int clockMinBlue = 128;
int clockOnPauseBrightness = 255;
//unsigned long lastClockExecution  = 0;

// Bell Curve Mode Variables
int bellCurveRed = 128;
int bellCurveGreen = 128;
int bellCurveBlue = 128;

// Night Rider Mode Variables
int nightRiderTopLedNumber = 0;
int nightRiderBottomLedNumber = 0;
int nightRiderTopIncrement = 1;
int nightRiderBottomIncrement = 1;

void setColour(int red, int green, int blue);
void setRainbow(int startHue, int speed, int brightness);
void setClock();
void setBellCurve();
void setNightRider();

void ledStringInit()
{
  // add the leds to fast led and clear them
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(ledString, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  // Set the maximum power draw
  // FastLED.setMaxPowerInVoltsAndMilliamps(5,1000);

  // Debug
  Serial.println("[handleMode] - LED string was set up correctly");
}

void handleMode()
{
  // Adapt the leds to the current mode
  if (currentMode == "Colour")
  {
    setColour(colourRed, colourGreen, colourBlue);
  }
  else if (currentMode == "Rainbow")
  {
    setRainbow(rainbowStartHue, rainbowSpeed, rainbowBri);
  }
  else if (currentMode == "Clock")
  {
    setClock();
  }
  else if (currentMode == "Bell Curve")
  {
    setBellCurve();
  }
  else if (currentMode == "Night Rider")
  {
    setNightRider();
  }

  // Adjust the brightness depending on the mode
  if (Mode != currentMode)
  {
    // Dim lights off first
    if (modeChangeFadeAmount > 0)
    {
      // Set the dimming variables and apply
      EVERY_N_MILLISECONDS(20)
      {
        modeChangeFadeAmount -= (FadeTime > 0) ? (255 / ((float)FadeTime / 20)) : 255;
      };
    }
    else
    {
      // Debug
      Serial.println("[handleMode] - Mode changed to: " + Mode);

      // Clear the LEDs
      FastLED.clear();

      // Set the currentMode to Mode
      currentMode = Mode;
      modeChangeFadeAmount = 0;
    }
  }
  else if (currentMode != previousMode)
  {
    // On mode change dim lights up
    if (modeChangeFadeAmount < 255)
    {
      EVERY_N_MILLISECONDS(20)
      {
        modeChangeFadeAmount += (FadeTime > 0) ? (255 / ((float)FadeTime / 20)) : 255;
      };
    }
    else
    {
      // Set the currentMode to Mode
      previousMode = currentMode;
    }
  }

  // Adjust the brightness depending on the state
  if (!State && previousState)
  {
    // Turn Lights off slowly
    if (modeChangeFadeAmount > 0)
    {
      EVERY_N_MILLISECONDS(20)
      {
        modeChangeFadeAmount -= (FadeTime > 0) ? (255 / ((float)FadeTime / 20)) : 255;
      };
    }
    else
    {
      // Debug
      Serial.println("[handleMode] - LED's turned off");

      // Set the previous state
      previousState = false;
    }
  }
  else if (State && !previousState)
  {
    // Turn on light slowly
    if (modeChangeFadeAmount < 255)
    {
      EVERY_N_MILLISECONDS(20)
      {
        modeChangeFadeAmount += (FadeTime > 0) ? (255 / ((float)FadeTime / 20)) : 255;
      };
    }
    else
    {
      // Debug
      Serial.println("[handleMode] - LED's turned on");

      // Set the previous values
      previousState = true;
    }
  }
  else

    // Globally Scale the brightness of all LED's
    modeChangeFadeAmount = constrain(modeChangeFadeAmount, 0, 255);
  nscale8(ledString, NUM_LEDS, (int)modeChangeFadeAmount);

  // Handle Fast LED
  FastLED.show();
  //  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

void setColour(int red, int green, int blue)
{
  fill_solid(ledString, NUM_LEDS, CRGB(red, green, blue));
}

void setRainbow(int startHue, int speed, int brightness)
{
  // Constrain the variables before using
  startHue = constrain(startHue, 0, 255);
  speed = speed > 0 ? speed : 0;
  brightness = constrain(brightness, 0, 255);

  // Update the hue by 1 every 360th of the allocated time
  if (speed > 0)
  {
    float rainbowDeltaHue = (255 / ((float)speed * 1000)) * 50;
    EVERY_N_MILLISECONDS(50)
    {
      rainbowAddedHue += rainbowDeltaHue;
      rainbowAddedHue = (rainbowAddedHue > 255) ? rainbowAddedHue - 255 : rainbowAddedHue;
    };

    startHue += (int)rainbowAddedHue;
  }

  // Calculate the rainbow so it lines up
  float deltaHue = (float)255 / (float)NUM_LEDS;
  float currentHue = startHue;
  for (int i = 0; i < NUM_LEDS; i++)
  {
    currentHue = startHue + (float)(deltaHue * i);
    currentHue = (currentHue < 255) ? currentHue : currentHue - 255;
    ledString[i] = CHSV(currentHue, 255, 255);
  }

  FastLED.setBrightness(brightness);
}

void setClock()
{
  if (ntpTimeSet)
  {
    // Get the number of seconds between each LED
    int hourLedDeltaT = 43200 / (topNumLeds);
    int minuteLedDeltaT = 3600 / (bottomNumLeds);

    // Get the current time modulated to hours and mins
    int currentHour = now() % 43200;
    int currentMinute = now() % 3600;

    // Get the current percentage the time is between 2 LEDS
    int hourGapTime = currentHour % hourLedDeltaT;
    int minuteGapTime = currentMinute % minuteLedDeltaT;
    float hourPercentOfGap = (float)hourGapTime / (float)hourLedDeltaT;
    float minutePercentOfGap = (float)minuteGapTime / (float)minuteLedDeltaT;

    // Calculate the current and next LED to turn on
    int hourLEDNumber = floor(currentHour / hourLedDeltaT);
    int hourCurrentLED = topLeds[hourLEDNumber];
    int hourNextLED = (hourLEDNumber == topNumLeds - 1) ? topLeds[0] : topLeds[hourLEDNumber + 1];
    int minuteLEDNumber = floor(currentMinute / minuteLedDeltaT);
    int minuteCurrentLED = bottomLeds[minuteLEDNumber];
    int minuteNextLED = (minuteLEDNumber == bottomNumLeds - 1) ? bottomLeds[0] : bottomLeds[minuteLEDNumber + 1];

    // Calculate the brightness of the current and next LED based on the percentage
    int hourCurrentLEDBrightness = 255 * (1 - hourPercentOfGap);
    int hourNextLEDBrightness = 255 * (hourPercentOfGap);
    int minuteCurrentLEDBrightness = 255 * (1 - minutePercentOfGap);
    int minuteNextLEDBrightness = 255 * (minutePercentOfGap);

    // Clear all the LED's
    FastLED.clear();

    // Set the colour of the LED
    ledString[hourCurrentLED] = CRGB(clockHourRed, clockHourGreen, clockHourBlue);
    ledString[hourNextLED] = ledString[hourCurrentLED];
    ledString[minuteCurrentLED] = CRGB(clockMinRed, clockMinGreen, clockMinBlue);
    ledString[minuteNextLED] = ledString[minuteCurrentLED];

    // Dim the led correctly
    ledString[hourCurrentLED].nscale8(hourCurrentLEDBrightness);
    ledString[hourNextLED].nscale8(hourNextLEDBrightness);
    ledString[minuteCurrentLED].nscale8(minuteCurrentLEDBrightness);
    ledString[minuteNextLED].nscale8(minuteNextLEDBrightness);
  }
  else
  {
    // Set each of the lights colours
    for (int i = 0; i < topNumLeds; i++)
    {
      ledString[topLeds[i]] = CRGB(clockHourRed, clockHourGreen, clockHourBlue);
    }
    for (int i = 0; i < topNumLeds; i++)
    {
      ledString[bottomLeds[i]] = CRGB(clockMinRed, clockMinGreen, clockMinBlue);
    }

    // Set the brightness up and down
    // Serial.println(sin8(clockOnPauseBrightness));
    nscale8(ledString, NUM_LEDS, triwave8(clockOnPauseBrightness));
    clockOnPauseBrightness += 1;
  }
}

void setBellCurve()
{
  // Set the top brightness
  for (int i = 0; i < topNumLeds; i++)
  {
    int ledNrightness = cubicwave8((255 / (float)topNumLeds) * i);
    ledString[topLeds[i]] = CRGB(bellCurveRed, bellCurveGreen, bellCurveBlue);
    ledString[topLeds[i]] %= ledNrightness;
  }

  // Set the Bottom brightness
  for (int i = 0; i < bottomNumLeds; i++)
  {
    int ledNrightness = cubicwave8((255 / (float)bottomNumLeds) * i);
    ledString[bottomLeds[i]] = CRGB(bellCurveRed, bellCurveGreen, bellCurveBlue);
    ledString[bottomLeds[i]] %= ledNrightness;
  }
}

void setNightRider()
{
  int delayTime = 500 / topNumLeds;
  EVERY_N_MILLISECONDS(delayTime)
  {
    // Set the current LED to Red
    ledString[topLeds[nightRiderTopLedNumber]] = CRGB(255, 0, 0);
    ledString[bottomLeds[nightRiderBottomLedNumber]] = CRGB::Red;
    // Serial.println(nightRiderTopLedNumber);
    // Serial.println(ledString[topLeds[0]]);

    //  Increment the LED number
    nightRiderTopLedNumber = nightRiderTopLedNumber + nightRiderTopIncrement;
    nightRiderBottomLedNumber = nightRiderBottomLedNumber + nightRiderBottomIncrement;
    if (nightRiderTopLedNumber >= topNumLeds - 1 || nightRiderTopLedNumber <= 0)
      nightRiderTopIncrement = -nightRiderTopIncrement;
    if (nightRiderBottomLedNumber >= bottomNumLeds - 1 || nightRiderBottomLedNumber <= 0)
      nightRiderBottomIncrement = -nightRiderBottomIncrement;

    // Start fading all lit leds
    fadeToBlackBy(ledString, NUM_LEDS, 10);
  };
}
