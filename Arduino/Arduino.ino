/*Program written by JelleWho
  Board: https://dl.espressif.com/dl/package_esp32_index.json DOIT ESP32 DEVKIT V1

  TODO:  https://github.com/jellewie/Arduino-Smart-light/issues
*/
#if !defined(ESP32)
#error "Please check if the 'DOIT ESP32 DEVKIT V1' board is selected, which can be downloaded at https://dl.espressif.com/dl/package_esp32_index.json"
#endif
//#define SerialEnabled
#ifdef SerialEnabled
#define     WiFiManager_SerialEnabled
#define     Server_SerialEnabled
#define     Task_SerialEnabled
#define     Time_SerialEnabled
//#define     TimeExtra_SerialEnabled
//#define     UpdateLEDs_SerialEnabled
//#define     Convert_SerialEnabled
#endif //SerialEnabled

#define LED_TYPE WS2813                   //WS2812B for 5V leds, WS2813 for newer 12V leds
const char* ntpServer = "pool.ntp.org";   //The server where to get the time from
const byte PAO_LED = 25;                  //To which pin the <LED strip> is connected to
const byte PAI_R = 32;                    //               ^ <Red potmeter> ^
const byte PAI_G = 33;                    //
const byte PAI_B = 34;                    //
const byte PAI_Brightness = 35;           //
const byte PDI_Button = 26;               //Pulled down with 10k to GND
const byte PAI_LIGHT = 39;                //Pulled down with a GL5528 to GND, and pulled up with 10k, This sensor is for AutoBrightness

#include "functions.h"
byte BootMode = OFF;                      //SOFT_SETTING In which mode to start in
bool DoHourlyAnimation = true;            //SOFT_SETTING If we need to show an animation every hour if we are in CLOCK mode
byte DoublePressMode = RAINBOW;           //SOFT_SETTING What mode to change to if the button is double pressed
bool AutoBrightness = false;              //SOFT_SETTING If the auto brightness is enabled
float AutoBrightnessP = 1.7;              //SOFT_SETTING Brightness = Y=(X-N)*P+O         [1-(AutoBrightnessN/255)] https://www.desmos.com/calculator/gpr6bwjleg
byte AutoBrightnessN = 105;               //SOFT_SETTING ^ (signed byte so -127 to 128)   [Just the lowest raw sensor value you can find]
byte AutoBrightnessO = 8;                 //SOFT_SETTING ^                                [Just an brigtness offset, so it can be set to be globaly more bright]
byte ClockHourLines = 0;                  //SOFT_SETTING how bright each hour mark needs to be (0 for off)
bool ClockHourAnalog = false;             //SOFT_SETTING If the clock needs to display the hour with 60 steps instead of 12 full hour steps
byte ClockOffset = 30;                    //SOFT_SETTING Number of LEDs to offset/rotate the clock, so 12 o'clock would be UP. Does NOT work in Animations
long gmtOffset_sec = 3600;                //SOFT_SETTING Set to you GMT offset (in seconds)
int  daylightOffset_sec = 3600;           //SOFT_SETTING Set to your daylight offset (in seconds)
byte PotMinChange = 2;                    //SOFT_SETTING How much the pot_value needs to change before we process it
byte PotStick = PotMinChange + 1;         //SOFT_SETTING If this close to HIGH or LOW stick to it
byte PotMin = PotMinChange + 2;           //SOFT_SETTING On how much pot_value_change need to change, to set mode to manual
char Name[16] = "smart-clock";            //SOFT_SETTING The mDNS, WIFI APmode SSID, and OTA name of the device. This requires a restart to apply, can only be 16 characters long, and special characters are not recommended.
bool UpdateLEDs;                          //If we need to physically update the LEDs
bool WiFiManager_connected;               //If the ESP is connected to WIFI
bool TimeSet = false;                     //If the time has been set or synced, is used to tasked based on time
byte Mode;                                //Holds in which mode the light is currently in
byte LastMode = -1;                       //Just to keep track if we are stepping into a new mode, and need to init that mode. -1 to force init
const byte TotalLEDs = 60;                //The total amounts of LEDs in the strip
int AnimationCounter;                     //Time in seconds that a AnimationCounter Animation needs to be played
TimeS TimeCurrent = {4};                  //Where we save the time to, set to HH=4 so it time syncs on startup

#include <FastLED.h>
CRGB LEDs[TotalLEDs];
#include "StableAnalog.h"
#include "Button.h"
#include "time.h"                         //We need this for the clock function to get the time (Time library)
#include "Task.h"
#include <WiFi.h>                         //we need this for WIFI stuff (duh)
#include <WebServer.h>
#include <ESPmDNS.h>
WebServer server(80);

Button ButtonsA = buttons({PDI_Button, LED_BUILTIN});
StableAnalog RED   = StableAnalog(PAI_R);
StableAnalog GREEN = StableAnalog(PAI_G);
StableAnalog BLUE  = StableAnalog(PAI_B);
StableAnalog BRIGH = StableAnalog(PAI_Brightness);
StableAnalog LIGHT = StableAnalog(PAI_LIGHT);

void setup() {
#ifdef SerialEnabled
  Serial.begin(115200);
#endif //SerialEnabled
  attachInterrupt(ButtonsA.Data.PIN_Button, ISR_ButtonsA, CHANGE);
  //==============================
  //Init LED and let them shortly blink
  //==============================
  pinMode(PAO_LED, OUTPUT);
  FastLED.addLeds<LED_TYPE, PAO_LED, GRB>(LEDs, TotalLEDs);
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
  server.on("/task",        handle_GetTasks);
  server.on("/info",        handle_Info);
  server.onNotFound(        handle_NotFound);           //When a client requests an unknown URI
  //==============================
  //Set AnalogResolution, and init the potmeters
  //==============================
  analogReadResolution(AnalogResolution);
  for (int i = 0; i < AverageAmount + 2; i++) {
    UpdateColor(false);                                 //Trash some measurements, so we get a good average on start
    UpdateBrightness(false);
  }
  FastLED.setBrightness(1);                             //Set boot Brightness
  //==============================
  //Load data from EEPROM, so we can apply the set bootmode
  //==============================
  LoadData();
  Mode = BootMode;                                      //Set the startup mode
  UpdateBrightness(true);                               //Force Update the brightness
#ifdef SerialEnabled
  Serial.println("Booting up in mode " + String(Mode) + "=" + ConvertModeToString(Mode));
#endif //SerialEnabled
  //==============================
  //Set up mDNS responder     //https://github.com/espressif/arduino-esp32/blob/master/libraries/ESPmDNS/src/ESPmDNS.cpp
  //==============================
  bool MDNSStatus = MDNS.begin(Name);               //Start mDNS with the given domain name
  if (MDNSStatus) MDNS.addService("http", "tcp", 80);   //Add service to MDNS-SD
#ifdef SerialEnabled
  if (MDNSStatus)
    Serial.println("mDNS responder started");
  else
    Serial.println("Error setting up MDNS responder!");
#endif
}
void loop() {
  OTA_loop();                                         //Do OTA stuff if needed
  WiFiManager_RunServer();                            //Do WIFI server stuff if needed
  if (TimeSet and Mode != CLOCK) UpdateAndShowClock(false); //If we are not in clock mode but the time has been set, update the internal time before ExecuteTask
  ExecuteTask();
  EVERY_N_MILLISECONDS(1000 / 60) {                   //Limit to 60FPS
    Button_Time Value = ButtonsA.CheckButton();       //Read buttonstate
#ifdef SerialEnabled        //DEBUG, print button state to serial
    if (Value.StartPress)       Serial.println("StartPress");
    if (Value.StartLongPress)   Serial.println("StartLongPress");
    if (Value.StartDoublePress) Serial.println("StartDoublePress");
    if (Value.StartRelease)     Serial.println("StartRelease");
#endif //SerialEnabled
    if (Value.StartPress) {
      if (Mode == OFF)
        Mode = ON;
      else
        Mode = OFF;
    }
    if (Value.StartDoublePress)
      Mode = DoublePressMode;         //Cool RGB color palet mode
    if (Value.StartLongPress) {
      Mode = WIFI;
      if (WiFiManager_connected) {    //If WIFI was already started
        ShowIP();
        LastMode = Mode;
      }
    };
    if (Value.PressedLong) {          //If it is/was a long press
      if (Value.PressedTime > Time_ESPrestartMS - 1000) //If we are almost resetting
        Mode = RESET;
    }
    UpdateBrightness(false);          //Check if manual input potmeters has changed, if so flag the update
    UpdateColor(false);               //Check if manual input potmeters has changed, if so flag the update
    loopLEDS();
  }
}

void loopLEDS() {
  if (AnimationCounter != 0)      //Animation needs to be shown
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
      if (LastMode != Mode) {     //If mode changed
        AnimationCounter = 0;
        UpdateColor(true);
      }
      break;
    case WIFI:
      if (LastMode != Mode) {     //If mode changed
        AnimationCounter = 0;
        StartWIFIstuff();         //Start WIFI if we haven't
      }
      break;
    case RESET:
      if (LastMode != Mode) {     //If mode changed
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
      if (LastMode != Mode) {     //If mode changed
        AnimationCounter = 0;
        StartWIFIstuff();         //Start WIFI if we haven't
      }
      if (AnimationCounter == 0)            //If no (hourly) animation is playing
        UpdateAndShowClock(true);
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
    case SINELON2:
      if (LastMode != Mode)     //If mode changed
        StartAnimation(9, -2);
      UpdateLEDs = true;
      break;
  }
  LastMode = Mode;
  UpdateLED();
}
void UpdateLED() {
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
