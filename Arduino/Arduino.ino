/*Program written by JelleWho
  Board: https://dl.espressif.com/dl/package_esp32_index.json DOIT ESP32 DEVKIT V1

  TODO
  -Sort mode on ABCF
  -IDEA?? Make it so mode CLOCK doesn't return RGB?
  -ERROR Clock animation needs to 'clear' the LEDs when staring, now the HHMMSS shines though for a bit
         UpdateAndShowClock is called one after DoHourlyAnimation has aleady started??
  -ERROR RGB not set after change while we where in BPM mode?

  -CHECK IF DONE
    If mode change to WIFI, send RGB values
*/
//#define SerialEnabled

#ifdef SerialEnabled
#define     WiFiManager_SerialEnabled
#define     Server_SerialEnabled
#define     Time_SerialEnabled
//#define     TimeExtra_SerialEnabled
//#deinfe     UpdateLEDs_SerialEnabled
#endif //SerialEnabled

#include <FastLED.h>
#define AnalogResolution 9                //howmany bits an analog read is
#define AnalogScaler ((AnalogResolution - 8)*2)
#include "StableAnalog.h"
#include "Button.h"
#include "functions.h"
#include "time.h"                         //We need this for the clock function to get the time
#include <WiFi.h>                         //we need this for wifi stuff (duh)
#include <WebServer.h>
WebServer server(80);

const byte TotalLEDs = 60;                //The total amounts of LEDs in the strip
const byte PAO_LED = 25;                  //To which pin the <LED strip> is connected to
const byte PAI_R = 32;                    //               ^ <Red potmeter> ^
const byte PAI_G = 33;                    //
const byte PAI_B = 34;                    //
const byte PAI_Brightness = 35;           //
const byte PDI_Button = 26;               //pulled down with 10k to GND

#define PotMinChange 2                    //Howmuch the pot_value needs to change before we process it 
#define PotStick PotMinChange + 1         //if this close to HIGH or LOW stick to it
#define PotMin PotMinChange + 2           //On howmuch fast pot_value_change to change mode to manual. 0-255. 

const char* ntpServer = "pool.ntp.org";   //The server where to get the time from
const long  gmtOffset_sec = 3600;         //Set to you GMT offset (in minutes)
const int   daylightOffset_sec = 3600;    //Set to your daylight offset (in minutes)
TimeS TimeCurrent = {4};                  //Where we save the time to, set to H=4 so it time syncs on startup
byte ClockOffset = 30;                    //Amount of LEDs to offset/rotate the clock, so 12 hours can be UP
int AnimationCounter;                     //Time in seconds that a AnimationCounter Animation needs to be played
bool DoHourlyAnimation = true;            //If we need to show an animation every hour if we are in CLOCK mode
bool WIFIconnected;
byte WIFI_Started;
Modes BootMode;

Button ButtonsA = buttons({PDI_Button, LED_BUILTIN});
CRGB LEDs[TotalLEDs];
StableAnalog RED   = StableAnalog(PAI_R);
StableAnalog GREEN = StableAnalog(PAI_G);
StableAnalog BLUE  = StableAnalog(PAI_B);
StableAnalog BRIGH = StableAnalog(PAI_Brightness);
bool UpdateLEDs;                          //Holds if we need to physically update the LEDs
byte Mode;                                //Holds the currentmode the light is in
byte LastMode = -1;                       //Just to keep track if we are stepping into a new mode, and need to init that mode. -1 to force init

void setup() {
#ifdef SerialEnabled
  Serial.begin(115200);
#endif //SerialEnabled
  analogReadResolution(AnalogResolution);
  attachInterrupt(ButtonsA.Data.PIN_Button, ISR_ButtonsA, CHANGE);
  //==============================
  //Init LED and let them shortly blink
  //==============================
  pinMode(PAO_LED, OUTPUT);
  FastLED.addLeds<WS2812, PAO_LED, GRB>(LEDs, TotalLEDs);
  FastLED.setBrightness(1);     //Set start brightness to be amost off
  for (int i = 255; i >= 0; i = i - 255) { //Blink on boot
    fill_solid(&(LEDs[0]), TotalLEDs, CRGB(i, i, i));
    FastLED.show();                                     //Update
    FastLED.delay(1);
  }
  //==============================
  //Set up all server UrlRequest stuff
  //==============================
  server.on("/ip",        WiFiManager_handle_Connect);  //Must be declaired before "WiFiManager_Start()" for APMode
  server.on("/setup",     WiFiManager_handle_Settings); //Must be declaired before "WiFiManager_Start()" for APMode
  server.on("/",            handle_OnConnect);          //Call the 'handleRoot' function when a client requests URI "/"
  server.on("/set",         handle_Set);
  server.on("/get",         handle_Getcolors);
  server.on("/ota",         handle_EnableOTA);
  server.on("/time",        handle_UpdateTime);
  server.onNotFound(        handle_NotFound);           //When a client requests an unknown URI
  //==============================
  //Load data from EEPROM, so we can apply the set bootmode
  //==============================
  LoadData();
  Mode = BootMode;                                      //Set the startup mode
#ifdef SerialEnabled
  Serial.println("Booting up in mode " + String(Mode) + "=" + ConvertModeToString(Mode));
#endif //SerialEnabled
}

void loop() {
  OTA_loop();                                         //Do OTA stuff if needed
  if (WIFIconnected) server.handleClient();           //Do WIFI server stuff if needed
  EVERY_N_MILLISECONDS(1000 / 60) {                   //Limit to 60FPS
    Button_Time Value = ButtonsA.CheckButton();       //Read buttonstate
#ifdef SerialEnabled        //DEBUG, print button state to serial
    if (Value.StartPress)   Serial.println("StartPress");
    if (Value.StartPress)   Serial.println("StartLongPress");
    if (Value.StartPress)   Serial.println("StartDoublePress");
    if (Value.StartRelease) Serial.println("StartRelease");
#endif //SerialEnabled
    if (Value.StartPress) {
      if (Mode == OFF)
        Mode = ON;
      else
        Mode = OFF;
    }
    if (Value.StartDoublePress) Mode = RAINBOW;         //Cool RGB color palet mode
    if (Value.StartLongPress) {
      Mode = WIFI;
      if (WIFI_Started)                               //If WIFI was already started
        ShowIP();
    };
    if (Value.PressedLong) {                          //If it is/was a long press
      if (Value.PressedTime > Time_ESPrestartMS - 1000) //If we are almost resetting
        Mode = RESET;
    }
    UpdateBrightness(false);      //Check if manual input potmeters has changed, if so flag the update
    UpdateColor(false);           //Check if manual input potmeters has changed, if so flag the update
    loopLEDS();
  }
}

void loopLEDS() {
  if (AnimationCounter != 0)   //Animation needs to be shown
    ShowAnimation(false);

  switch (Mode) {
    case OFF:
      if (LastMode != Mode) {
        AnimationCounter = 0;
        FastLED.clear();
        UpdateLEDs = true;
      }
      break;
    case ON:
      if (LastMode != Mode) {    //If mode changed
        AnimationCounter = 0;
        UpdateColor(true);
      }
      break;
    case WIFI:
      if (LastMode != Mode) {    //If mode changed
        AnimationCounter = 0;
        StartWIFIstuff();
      }
      break;
    case RESET:
      if (LastMode != Mode) {    //If mode changed
        AnimationCounter = 0;
      }
      if (TickEveryMS(50)) {
        if (LEDs[0] != CRGB(0, 0, 0))
          FastLED.clear();
        else
          fill_solid(&(LEDs[0]), TotalLEDs, CRGB(255, 0, 0));
        UpdateLEDs = true;
      }
    case CLOCK:
      if (LastMode != Mode) {    //If mode changed
        AnimationCounter = 0;
        StartWIFIstuff();                     //Start WIFI if we haven't
      }
      if (AnimationCounter == 0)            //If no (hourly) animation is playing
        UpdateAndShowClock();
      break;

    //Animations
    case BLINK:
      if (LastMode != Mode)     //If mode changed
        StartAnimation(2, -2);
      UpdateLEDs = true;
      break;
    case BPM:
      if (LastMode != Mode)     //If mode changed
        StartAnimation(8, -2);
      UpdateLEDs = true;
      break;
    case CONFETTI:
      if (LastMode != Mode)     //If mode changed
        StartAnimation(6, -2);
      UpdateLEDs = true;
      break;
    case FLASH:
      if (LastMode != Mode)     //If mode changed
        StartAnimation(1, -2);
      UpdateLEDs = true;
      break;
    case GLITTER:
      if (LastMode != Mode)     //If mode changed
        StartAnimation(7, -2);
      UpdateLEDs = true;
      break;
    case JUGGLE:
      if (LastMode != Mode)     //If mode changed
        StartAnimation(4, -2);
      UpdateLEDs = true;
      break;
    case MOVE:
      if (LastMode != Mode)     //If mode changed
        StartAnimation(0, -2);
      UpdateLEDs = true;
      break;
    case RAINBOW:
      if (LastMode != Mode)     //If mode changed
        StartAnimation(3, -2);
      UpdateLEDs = true;
      break;
    case SINELON:
      if (LastMode != Mode)     //If mode changed
        StartAnimation(5, -2);
      UpdateLEDs = true;
      break;
  }
  LastMode = Mode;
  if (UpdateLEDs) {
#ifdef UpdateLEDs_SerialEnabled
    Serial.println("Update LEDs");
#endif //UpdateLEDs_SerialEnabled
    UpdateLEDs = false;
    FastLED.show();                         //Update
  }
}
//ISR must return nothing and take no arguments, so we need this sh*t
void ISR_ButtonsA() {
  ButtonsA.Pinchange();
}
