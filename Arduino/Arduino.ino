/*Program written by JelleWho
  TODO:  https://github.com/jellewie/Arduino-Smart-light/issues
*/
#if !defined(ESP32)
#error "Please check if the 'DOIT ESP32 DEVKIT V1' board is selected, which can be downloaded at https://dl.espressif.com/dl/package_esp32_index.json"
#endif

//#define WiFiManager_DNS                           //is still causing some troubles with (some?) ESP32s

//#define SerialEnabled
#ifdef SerialEnabled
#define     WiFiManager_SerialEnabled           //WM:
#define     Server_SerialEnabled                //SV:
#define     Task_SerialEnabled                  //TK:
#define     Time_SerialEnabled                  //TM:
#define     SetupTime_SerialEnabled             //ST:
#define     OTA_SerialEnabled                   //OTA:
#define     RGBL_SerialEnabled                  //RG:
//#define     LoopTime_SerialEnabled              //LT:
//#define     TimeExtra_SerialEnabled             //TME:
//#define     UpdateLEDs_SerialEnabled            //UL:
//#define     Convert_SerialEnabled               //CV:
//#define     LEDstatus_SerialEnabled             //LS:
#endif //SerialEnabled

#define LED_TYPE WS2812B                          //WS2812B for 5V leds, WS2813 for 12V leds
const char* ntpServer = "pool.ntp.org";           //The server where to get the time from
const byte PAO_LED = 25;                          //To which pin the <LED strip> is connected to
const byte PAI_R = 32;                            //               ^ <Red potmeter> ^
const byte PAI_G = 33;                            //
const byte PAI_B = 34;                            //
const byte PAI_Brightness = 35;                   //
const byte PDI_Button = 26;                       //Pulled down with 10k to GND
const byte PAI_LIGHT = 39;                        //Pulled down with a GL5528 to GND, and pulled up with 10k, This sensor is for AutoBrightness
const byte PAI_DisablePOTs = 4;                   //Intern pulled up, when pulled down disableds reading of POTS
const byte PAI_OtherDefault  = 5;                 //Intern pulled up, when pulled down disableds reading of POTS

#include "Structs.h"
byte BootMode = OFF;                              //SOFT_SETTING In which mode to start in
byte HourlyAnimationS = 10;                       //SOFT_SETTING If we need to show an animation every hour if we are in CLOCK mode, defined in time in seconds where 0=off
byte DoublePressMode = RAINBOW;                   //SOFT_SETTING What mode to change to if the button is double pressed
bool AutoBrightness = true;                       //SOFT_SETTING If the auto brightness is enabled
float AutoBrightnessP = 1.04;                     //SOFT_SETTING Brightness = Y=P*(X-N)+O [255/(255-AutoBrightnessN)]
byte AutoBrightnessN = 10;                        //SOFT_SETTING ^                        [Just the lowest raw sensor value you can find]
byte AutoBrightnessO = 5;                         //SOFT_SETTING ^                        [Just an brigtness offset, so it can be set to be globaly more bright]
byte ClockHourLines = 0;                          //SOFT_SETTING how bright each hour mark needs to be (0 for off)
bool ClockHourAnalog = false;                     //SOFT_SETTING If the clock needs to display the hour with 60 steps instead of 12 full hour steps
byte LEDOffset = 0;                             //SOFT_SETTING Number of LEDs to offset/rotate the clock, so 12 o'clock would be UP. Does NOT work in Animations
bool ClockAnalog = false;                         //SOFT_SETTING Makes it so the LEDs dont step, but smootly transition
long gmtOffset_sec = 0;                           //SOFT_SETTING Set to you GMT offset (in seconds)
int  daylightOffset_sec = 0;                      //SOFT_SETTING Set to your daylight offset (in seconds)
byte PotMinChange = 2;                            //SOFT_SETTING How much the pot_value needs to change before we process it
byte PotStick = PotMinChange + 1;                 //SOFT_SETTING If this close to HIGH or LOW stick to it
byte PotMin = PotMinChange + 2;                   //SOFT_SETTING On how much pot_value_change need to change, to set mode to manual
char Name[16] = "smart-clock";                    //SOFT_SETTING The mDNS, WIFI APmode SSID name. This requires a restart to apply, can only be 16 characters long, and special characters are not recommended.
bool UpdateLEDs;                                  //If we need to physically update the LEDs
bool TimeSet = false;                             //If the time has been set or synced, is used to tasked based on time
byte Mode;                                        //Holds in which mode the light is currently in
byte LastMode = -1;                               //Just to keep track if we are stepping into a new mode, and need to init that mode. -1 to force init
const byte TotalLEDs = 60;                        //The total amounts of LEDs in the strip
int AnimationCounter;                             //Time in seconds that a AnimationCounter Animation needs to be played
TimeS TimeCurrent;                                //Where we save the time to
byte ModeBeforeOff = ON;
extern byte TotalAnimations;                      //^ Required for Clock.h
extern void StartAnimation(byte ID, int Time);    //^ Required for Clock.h

#define WiFiManagerUser_Set_Value_Defined         //Define we want to hook into WiFiManager
#define WiFiManagerUser_Get_Value_Defined         //^
#define WiFiManagerUser_Status_Start_Defined      //^
#define WiFiManagerUser_Status_Done_Defined       //^
#define WiFiManagerUser_Status_Blink_Defined      //^
#define WiFiManagerUser_Status_StartAP_Defined    //^
#define WiFiManagerUser_HandleAP_Defined          //^
#define WiFiManagerUser_Name_Defined
#define WiFiManagerUser_VariableNames_Defined
const String WiFiManager_VariableNames[] = {"SSID", "Password", "BootMode", "HourlyAnimationS", "DoublePressMode", "AutoBrightness", "AutoBrightnessN", "AutoBrightnessP", "AutoBrightnessO", "ClockHourLines", "ClockHourAnalog", "LEDOffset", "ClockAnalog", "gmtOffset_sec", "daylightOffset_sec", "PotMinChange", "PotStick", "PotMin", "Name", "Task0", "Task1", "Task2", "Task3", "Task4", "Task5", "Task6", "Task7"};
const byte EEPROM_size = 255;
#define WiFiManager_mDNS
#define WiFiManager_OTA
#define WiFiManagerUser_UpdateWebpage_Defined
const String UpdateWebpage = "https://github.com/jellewie/Arduino-Smart-light/releases";
#include "WiFiManager/WiFiManager.h"              //Includes <WiFi> and <WebServer.h> and setups up 'WebServer server(80)' if needed      https://github.com/jellewie/Arduino-WiFiManager

#include <FastLED.h>                              //Include the libary FastLED (If you get a error here, make sure it's installed!)
CRGB LEDs[TotalLEDs];
#include "StableAnalog.h"
#include "Button.h"
Button ButtonsA = buttons({PDI_Button, PAO_LED});
StableAnalog RED   = StableAnalog(PAI_R);
StableAnalog GREEN = StableAnalog(PAI_G);
StableAnalog BLUE  = StableAnalog(PAI_B);
StableAnalog BRIGH = StableAnalog(PAI_Brightness);
StableAnalog LIGHT = StableAnalog(PAI_LIGHT);
#include "Functions.h"
#include "time.h"                                 //We need this for the clock function to get the time (Time library)
#include "Task.h"
#include "WiFiManagerUser.h"                      //Define custon functions to hook into WiFiManager
#include "Clock.h"
#include "Animation.h"

void setup() {
#ifdef SerialEnabled
  Serial.begin(115200);
  Serial.setTimeout(1);
#endif //SerialEnabled
  //===========================================================================
  //Init LED and let them shortly blink
  //===========================================================================
  FastLED.addLeds<LED_TYPE, PAO_LED, GRB>(LEDs, TotalLEDs);
  FastLED.setBrightness(1);     //Set start brightness to be amost off
  for (int i = 255; i >= 0; i = i - 255) { //Blink on boot
    fill_solid(&(LEDs[0]), TotalLEDs, CRGB(i, i, i));
    FastLED.show();                                   //Update
    FastLED.delay(1);
  }
  //===========================================================================
  //Set and attach pins
  //===========================================================================
  attachInterrupt(ButtonsA.Data.PIN_Button, ISR_ButtonsA, CHANGE);
  pinMode(PAI_DisablePOTs, INPUT_PULLUP);             //Pull the pin up, so the pin is by default HIGH if not attached
  //===========================================================================
  //Set default settings
  //===========================================================================
  pinMode(PAI_OtherDefault, INPUT_PULLUP);            //Pull the pin up, so the pin is by default HIGH if not attached
  if (digitalRead(PAI_OtherDefault) == LOW) {         //If the pin is pulled low by hardware, Change the default of some SOFT_SETTINGs
    BootMode = CLOCK;
    DoublePressMode = CLOCK;
  }
  //===========================================================================
  //Set up all server UrlRequest stuff
  //===========================================================================
  server.on("/",            handle_OnConnect);        //Call the 'handleRoot' function when a client requests URL "/"
  server.on("/ip",          WiFiManager_handle_Connect);
  server.on("/get",         handle_Getcolors);
  server.on("/set",         handle_Set);
  server.on("/gettasks",    Tasks_handle_GetTasks);
  server.on("/settask",     Tasks_handle_Settings);
  server.on("/time",        handle_UpdateTime);
  server.on("/info",        handle_Info);
  server.on("/reset",       handle_Reset);
  server.onNotFound(        handle_NotFound);         //When a client requests an unknown URI
  //===========================================================================
  //Set AnalogResolution, and init the potmeters
  //===========================================================================
  analogReadResolution(AnalogResolution);
  for (int i = 0; i < AverageAmount + 2; i++) {
    UpdateColor(false);                               //Trash some measurements, so we get a good average on start
    UpdateBrightness(false);
  }
  FastLED.setBrightness(8);                           //Set boot Brightness
  //===========================================================================
  //Add a SYNCTIME Task, this is just to initialise (will be overwritten with user data)
  //===========================================================================
  TASK TempTask;                                      //Create a space to put a new Task in
  TempTask.Type       = SYNCTIME;                     //Set the ID of the task to SYNCTIME
  TempTask.ExectuteAt = TimeS{4, 0, 0, 0};            //Set the init value to auto sync time at 04:00:00
  AddTask(TempTask);                                  //Add the Task command to the task list
  //===========================================================================
  //Load data from EEPROM, so we can apply the set bootmode
  //===========================================================================
  WiFiManager.LoadData();
  Mode = BootMode;                                    //Set the startup mode
#ifdef SerialEnabled
  Serial.println("SE: Booting up in mode " + String(Mode) + "=" + ConvertModeToString(Mode));
#endif //SerialEnabled
  loopLEDS();
  UpdateBrightness(true);                             //Force Update the brightness
#ifdef SetupTime_SerialEnabled                        //Just a way to measure setup speed, so the performance can be checked
  Serial.println("ST: Setup took ms:\t" + String(millis()));
#endif //SetupTime_SerialEnabled
}
void loop() {
#ifdef LoopTime_SerialEnabled                         //Just a way to measure loop speed, so the performance can be checked
  static unsigned long LoopLast = 0;
  unsigned long LoopNow = micros();
  float LoopMs = (LoopNow - LoopLast) / 1000.0;
  LoopLast = LoopNow;
  Serial.println("LT: Loop took ms:\t" + String(LoopMs));
#endif //LoopTime_SerialEnabled
  WiFiManager.RunServer();                            //Do WIFI server stuff if needed
  if (TimeSet and Mode != CLOCK) UpdateAndShowClock(false); //If we are not in clock mode but the time has been set, update the internal time before ExecuteTask
  ExecuteTask();
  EVERY_N_MILLISECONDS(1000 / 60) {                   //Limit to 60FPS
    Button_Time Value = ButtonsA.CheckButton();       //Read buttonstate
#ifdef SerialEnabled                                  //DEBUG, print button state to serial
    if (Value.StartPress)       Serial.println("StartPress");
    if (Value.StartLongPress)   Serial.println("StartLongPress");
    if (Value.StartDoublePress) Serial.println("StartDoublePress");
    if (Value.StartRelease)     Serial.println("StartRelease");
#endif //SerialEnabled
    if (Value.StartPress) {
      if (Mode == OFF) {
        Mode = ModeBeforeOff == OFF ? ON : ModeBeforeOff; //If (ModeBeforeOff is OFF) then {set mode to be ON} else {set to ModeBeforeOff}
      } else {
        ModeBeforeOff = Mode;
        Mode = OFF;
      }
    }
    if (Value.StartDoublePress)
      Mode = DoublePressMode;                         //Cool RGB color palet mode
    if (Value.StartLongPress) {
      Mode = WIFI;
      if (WiFi.status() == WL_CONNECTED) {            //If WIFI was already started
        ShowIP();
        LastMode = Mode;
      }
    };
    if (Value.PressedLong) {                          //If it is/was a long press
      if (Value.PressedTime > Time_ESPrestartMS - 1000) //If we are almost resetting
        Mode = RESET;
    }
    UpdateBrightness(false);                          //Check if manual input potmeters has changed, if so flag the update
    UpdateColor(false);                               //Check if manual input potmeters has changed, if so flag the update
    loopLEDS();
  }
}
void loopLEDS() {
  if (AnimationCounter != 0)                          //Animation needs to be shown
    ShowAnimation(false);
  switch (Mode) {
    case OFF: {
        if (LastMode != Mode) {
          AnimationCounter = 0;
          FastLED.clear();
          UpdateLEDs = true;
        }
      } break;
    case ON: {
        if (LastMode != Mode) {                         //If mode changed
          AnimationCounter = 0;
          UpdateColor(true);
        }
      } break;
    case WIFI: {
        if (LastMode != Mode) {                         //If mode changed
          AnimationCounter = 0;
          WiFiManager.Start();                          //Start WIFI if we haven't
        }
      } break;
    case RESET: {
        if (LastMode != Mode) {                         //If mode changed
          AnimationCounter = 0;
        }
        EVERY_N_MILLISECONDS(50) {
          LED_Flash(0, TotalLEDs, CRGB(255, 0, 0));     //Toggle the leds RED/OFF to show we are restarting soon
          UpdateLEDs = true;
        }
      } break;
    case CLOCK: {
        if (LastMode != Mode) {                         //If mode changed
          AnimationCounter = 0;
          WiFiManager.Start();                          //Start WIFI if we haven't
        }
        if (AnimationCounter == 0)                      //If no (hourly) animation is playing
          UpdateAndShowClock(true);
      } break;
    case BLINK:       if (LastMode != Mode) StartAnimation(0,  -2); break;
    case BPM:         if (LastMode != Mode) StartAnimation(1,  -2); break;
    case CONFETTI:    if (LastMode != Mode) StartAnimation(2,  -2); break;
    case FLASH:       if (LastMode != Mode) StartAnimation(3,  -2); break;
    case GLITTER:     if (LastMode != Mode) StartAnimation(4,  -2); break;
    case JUGGLE:      if (LastMode != Mode) StartAnimation(5,  -2); break;
    case MOVE:        if (LastMode != Mode) StartAnimation(6,  -2); break;
    case RAINBOW:     if (LastMode != Mode) StartAnimation(7,  -2); break;
    case SINELON:     if (LastMode != Mode) StartAnimation(8,  -2); break;
    case SINELON2:    if (LastMode != Mode) StartAnimation(9,  -2); break;
    case SMILEY:      if (LastMode != Mode) StartAnimation(10, -2); break;
    case FLASH2:      if (LastMode != Mode) StartAnimation(11, -2); break;
    case PACMAN:      if (LastMode != Mode) StartAnimation(12, -2); break;
    default:
#ifdef SerialEnabled
      Serial.println("mode with ID " + String(Mode) + " not found");
#endif //SerialEnabled
      Mode = ON;
      AnimationCounter = 0;                           //Stop animation
      break;
  }
  LastMode = Mode;
  UpdateLED();
}
//ISR must return nothing and take no arguments, so we need this sh*t
void ISR_ButtonsA() {
  ButtonsA.Pinchange();
}
