/*
  Program written by JelleWho as a school project for the self driving car

  =====TODO=====
  TODO FIXME fix the OverFlowError handler, we could lose up to 5000ms every 50 days
  TODO Make stripes work with less brightness values

*/

#define ShowStripes                                                 //Uncomment to hide the 12 stripes (only visable with full brightness)
//#define AnalogHours                                               //Uncomment to have 60 stands for hours (so 60 places the arrow can be instead of just 12)
//#define POD_LED = LED_BUILTIN;                                    //Uncomment to have this pin blink each cycle
//#define SerialEnabled                                             //Uncomment to have debug feedback, else the code runs smoother

const int TotalLEDs = 60;                                           //The total amounts of LEDs in the strip
const int LedStartOffset = 0;                                      //Offset of the LEDS, First LED (+This offset) = 12 o'clock
const byte ColorS[3] {255, 0, 0};                                   //RGB color of Seconds
const byte ColorM[3] {0, 255, 0};                                   //RGB color of Minutes
const byte ColorH[3] {0, 0, 255};                                   //RGB color of Hours

byte AnimationCounter;                                              //Time in seconds that a AnimationCounter Animation needs to be played

const byte POA_LED = 5;                                             //Where the <LED strip> is connected to
const byte PID_EditTimeM = 2;                                       //
const byte PID_EditTimeH = 3;                                       //
const byte PID_EditBrigh = 4;                                       //
const byte PIA_Analog = A0;                                         //

#include <FastLED.h>                                                //Include the FastLED library to control the LEDs in a simple fashion
CRGB LEDs[TotalLEDs];                                               //This is an array of LEDs. One item for each LED in your strip.
int Correction;                                                     //Howmuch ms we have shifted and need to corect for
unsigned long NextTime;                                             //Next time we should update the LEDS
byte Time[3] {0, 0, 0};                                             //This stores the current time in hh,mm,ss
bool UpdateLED;
int8_t HouAdd = 100;                                                //Hour overwrite, give hour led this amount of offset, handy for more manual feedback
unsigned long Now;                                                  //Stored current time (so the time in a single loop is the same)

void setup() {
  pinMode(POA_LED,        OUTPUT);                                  //Sometimes the Arduino needs to know what pins are OUTPUT and what are INPUT,
  pinMode(PID_EditTimeM,  INPUT);                                   //^
  pinMode(PID_EditTimeH,  INPUT);                                   //^
  pinMode(PID_EditBrigh,  INPUT);                                   //^
  pinMode(PIA_Analog,     INPUT);                                   //^
  randomSeed(analogRead(5));                                        //Set where we get the AnimationCounter seed from
  Time[0] = String(__TIME__).substring(0, 2).toInt();               //Get upload time and set it as time
  Time[1] = String(__TIME__).substring(3, 5).toInt();               //^
  Time[2] = String(__TIME__).substring(6, 8).toInt();               //^
  FastLED.addLeds<WS2812B, POA_LED, GRB>(LEDs, TotalLEDs);          //Set the LED type and such
  FastLED.setBrightness(255);                                       //Set brightness (really just scaling)
  fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 0, 0));                 //Set the whole LED strip to be off (Black)
  FastLED.show();                                                   //Update
#ifdef SerialEnabled                                                //Serial is enabled
  Serial.begin(9600);                                               //Opens Serial port, sets data rate to 9600 bps
  Serial.setTimeout(5);                                             //Set the timeout to be short so we can responce faster to the host
  ShowTime();                                                       //Print out the time to the host
#endif //SerialEnabled
}

void loop() {
  Now = millis();
#ifdef POD_LED
  digitalWrite(POD_LED, !digitalRead(POD_LED));                     //Let the Blink LED blink each cycle
#endif //POD_LED
  if (digitalRead(PID_EditTimeM) == HIGH)                           //Adjust minute button is pushed
    ChangeTime(1, 100);
  else if (digitalRead(PID_EditTimeH) == HIGH)                      //Adjust hour button is pushed
    ChangeTime(0, 500);
  else if (digitalRead(PID_EditBrigh) == HIGH)                      //Adjust brightness button is pushed
    ChangeBrightness(250);
  else
    HouAdd = 100;                                                   //Reset overrule hours
#ifdef SerialEnabled                                                //Serial is enabled
  if (Serial.available() > 0)                                       //https://www.arduino.cc/en/Reference/ASCIIchart to see the asci chart to know what numbers are what
    HandleSerial();
#endif //SerialEnabled
  if (Now < NextTime - 1500 and NextTime > 1500)                    //OverFlow is detected
    OverFlowError();
  if (AnimationCounter > 0) {                                       //Animation needs to be shown
    ShowAnimation(false);
  } else {
    if (Now >= NextTime) {                                          //If it's time for an update
      Correction = Now - NextTime;                                  //Check if we are on time
      NextTime = Now + 1000 - Correction ;                          //Set when we need an update again (also take in the count the 'we where to late' time)
      Time[2] = Time[2] + 1;                                        //Add 1 second to the counter
      if (Correction > 0)                                           //If we did a correction
        Serialprintln("Correction -" + String(Correction) + "ms");
      ClampTime(true);
    }
  }
  if (UpdateLED) {
    UpdateLED = false;
    UpdateTime();
  }
}
void UpdateTime() {
  byte TimeLED[3] {
    5 * Time[0] + LedStartOffset,                                 //We need to multiply with 5 to get led position of the hour
    Time[1] + LedStartOffset,
    Time[2] + LedStartOffset
  };
#ifdef AnalogHours
  TimeLED[0] = TimeLED[0] + TimeLED[2] / 5;                       //instead of 12 states, create more states for the hour in between
#endif //AnalogHours
  if (HouAdd < 60)                                                //overrule hours is active
    TimeLED[0] = TimeLED[0] + HouAdd;
  for (byte i = 0; i < 3; i++)                                         //LED loopardound protection
    while (TimeLED[0] >= TotalLEDs)
      TimeLED[0] = TimeLED[0] - TotalLEDs;
  fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 0, 0));               //Set the whole display to blank, we will overwrite this with the LEDS that needs to be on before updating
#ifdef ShowStripes
  for (int i = 0; i <= 60 - 5; i = i + 5)                         //Create the (12) hourly stripes
    LEDs[i] += CRGB(1, 1, 1);
#endif //ShowStripes
  LEDs[TimeLED[0]] += CRGB(ColorS[0], ColorS[1], ColorS[2]);
  LEDs[TimeLED[1]] += CRGB(ColorM[0], ColorM[1], ColorM[2]);
  LEDs[TimeLED[2]] += CRGB(ColorH[0], ColorH[1], ColorH[2]);
  FastLED.show();                                                 //Update the LEDs
}
void ClampTime(bool Animation) {
  if (Time[2] >= 195)                                               //If we had an overflow
    Time[2] = Time[2] - 196;                                        //Correct for it
  if (Time[1] >= 195) {                                             //If we had an overflow
    Time[1] = Time[1] - 196;                                        //Correct for it
    Time[0] = Time[0] - 1;                                          //Remove 1 hour to the HOUR counter
  }
  if (Time[0] >= 195)                                               //If we had an overflow
    Time[0] = Time[0] - 196;                                        //correct for it

  while (Time[2] >= 60) {                                           //If the SECOND counter has reached 60
    Time[2] = Time[2] - 60;                                         //Reset counter
    Time[1] = Time[1] + 1;                                          //Add 1 minute to the MINUTE counter
  }
  while (Time[1] >= 60) {                                           //If the MINUTE counter has reached 60
    Time[1] = Time[1] - 60;                                         //Reset counter
    Time[0] = Time[0] + 1;                                          //Add 1 hour to the HOUR counter
    if (Animation)                                                  //If we need an Animation
      StartAnimation(random(0, 7));                                 //Start a random Animation
  }
  while (Time[0] >= 24)                                             //If the counter has reached 12 hours
    Time[0] = Time[0] - 24;                                         //Reset counter
  UpdateLED = true;
}
void ChangeTime(byte TimeI, int MinTime) {
  static unsigned long LastTime = Now;
  byte Analog = analogRead(PIA_Analog);
  if (HouAdd == 100) {                                              //if starting
    HouAdd = 101;                                                   //flag we have started
    LastTime = Now;                                                 //Set the LastTime as now
  }
  int value = MinTime + Analog;
  if (Now >= LastTime + value) {                                    //If it's time to update
    unsigned long Correction2 = Now - (LastTime + value);           //If we overshoot
    LastTime = Now - Correction2;                                   //Set new LastTime updated
    if (digitalRead(PID_EditBrigh) == HIGH)                         //If inverse is pressed
      Time[TimeI] = Time[TimeI] - 1;
    else
      Time[TimeI] = Time[TimeI] + 1;
    HouAdd = 100;                                                   //Reset overrule hours
    ClampTime(false);
    Serialprintln("Add 1 to " + String(TimeI));
  } else if (TimeI == 0) {                                          //Just some fancy stuff trying to add a manual hour Animation
    int8_t HouAddOld = HouAdd;
    HouAdd = 5 - (5 * (value - abs(Now - LastTime)) / value);
    if (digitalRead(PID_EditBrigh) == HIGH)                         //If inverse is pressed
      HouAdd = -HouAdd;                                             //Inverse value; we are going backwards
    if (HouAddOld != HouAdd)
      ClampTime(false);
  }
}
void ShowTime() {
  Serialprintln("Time set: " + String(Time[0]) + ":" + String(Time[1]) + ":" + String(Time[2]) + " counter= " + String(Now));
}
void ChangeBrightness(int Delay) {
  EVERY_N_MILLISECONDS(Delay) {
    byte Brightness = 255 - analogRead(PIA_Analog) / 4;
    if (Brightness < 10)
      Brightness = 1;
    if (Brightness > 245)
      Brightness = 255;
    FastLED.setBrightness(Brightness);                            //Set brightness (really just scaling)
    UpdateLED = true;
    Serialprintln("Change brightness to " + String(Brightness));
  }
}
