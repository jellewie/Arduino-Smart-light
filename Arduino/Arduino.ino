/*Written by JelleWho https://github.com/jellewie
  TODO:  https://github.com/jellewie/Arduino-Smart-light/issues

  2023-07-24 PAO_MIC is on the SAME pin as PAI_B, so both can not be active at the same time. We need ADC1 if we also use WiFi
*/
#if !defined(ESP32)
#error "Please check if the 'DOIT ESP32 DEVKIT V1' board is selected, which can be downloaded at https://dl.espressif.com/dl/package_esp32_index.json"
#endif

//==============================================================//Note spacer

//#define SerialEnabled
#ifdef SerialEnabled
#define     WiFiManager_SerialEnabled                           //WM:
#define     Server_SerialEnabled                                //SV:
#define     Task_SerialEnabled                                  //TK:
#define     Time_SerialEnabled                                  //TM:
#define     SetupTime_SerialEnabled                             //ST:
#define     OTA_SerialEnabled                                   //OTA:
#define     RGBL_SerialEnabled                                  //RG:
#define     Animation_SerialEnabled                             //AN
#define     HomeAssistant_SerialEnabled                         //HA
//#define     Audio_SerialEnabled                               //AU:
//#define     LoopTime_SerialEnabled                            //LT:
//#define     TimeExtra_SerialEnabled                           //TME:
//#define     UpdateLEDs_SerialEnabled                          //UL:
//#define     Convert_SerialEnabled                             //CV:
//#define     LEDstatus_SerialEnabled                           //LS:
#endif //SerialEnabled

#define LED_TYPE WS2812B                                        //WS2812B for 5V LEDs, WS2813 for 12V LEDs
const char* ntpServer = "pool.ntp.org";                         //The server where to get the time from
const byte PAO_LED = 25;                                        //To which pin the <LED strip> is connected to
const byte PAO_MIC = 34;                                        //To which pin the <LED strip> is connected to
const byte PAI_R = 32;                                          //               ^ <Red potmeter> ^
const byte PAI_G = 33;                                          //
const byte PAI_B = 34;                                          //
const byte PAI_Brightness = 35;                                 //
const byte PDI_Button = 26;                                     //Pulled down with 10k to GND
const byte PAI_LIGHT = 39;                                      //Pulled down with a GL5528 to GND, and pulled up with 10k, This sensor is for AutoBrightness
const byte PAI_DisablePOTs = 4;                                 //Intern pulled up, when pulled down disableds reading of POTS
const byte PAI_OtherDefault  = 5;                               //Intern pulled up, when pulled down disableds reading of POTS

#include "Structs.h"
byte BootMode = OFF;                                            //SOFT_SETTING In which mode to start in
byte HourlyAnimationS = 10;                                     //SOFT_SETTING If we need to show an animation every hour if we are in CLOCK mode, defined in time in seconds where 0=off
byte DoublePressMode = RAINBOW;                                 //SOFT_SETTING What mode to change to if the button is double pressed
bool AutoBrightness = true;                                     //SOFT_SETTING If the auto brightness is enabled
bool AudioLink = false;                                         //SOFT_SETTING If the AudioLink is enabled
bool StandAlone = false;                                        //SOFT_SETTING If the StandAlone is enabled
float AutoBrightnessP = 1.04;                                   //SOFT_SETTING Brightness = y=255-(P*(x-N)-O) https://www.desmos.com/calculator/lmezlpkwsp
byte AutoBrightnessN = 10;                                      //SOFT_SETTING ^                        [Just the lowest raw sensor value you can find]
byte AutoBrightnessO = 5;                                       //SOFT_SETTING ^                        [Just an brigtness offset, so it can be set to be globaly more bright]
byte ClockHourLines = 0;                                        //SOFT_SETTING how bright each hour mark needs to be (0 for off)
bool ClockHourAnalog = false;                                   //SOFT_SETTING If the clock needs to display the hour with 60 steps instead of 12 full hour steps
byte LEDOffset = 0;                                             //SOFT_SETTING Number of LEDs to offset/rotate the clock, so 12 o'clock would be UP. Does NOT work in Animations
bool ClockAnalog = false;                                       //SOFT_SETTING Makes it so the LEDs dont step, but smootly transition
String timeZone = "CET-1CEST,M3.5.0,M10.5.0/3";                 //SOFT_SETTING Set to you Timezone, see https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
byte PotMinChange = 2;                                          //SOFT_SETTING How much the pot_value needs to change before we process it
byte PotStick = PotMinChange + 1;                               //SOFT_SETTING If this close to HIGH or LOW stick to it
byte PotMin = PotMinChange + 2;                                 //SOFT_SETTING On how much pot_value_change need to change, to set mode to manual
char Name[16] = "smart-clock";                                  //SOFT_SETTING The mDNS, WIFI APmode SSID name. This requires a restart to apply, can only be 16 characters long, and special characters are not recommended.
float AudioMultiplier = 2;                                      //SOFT_SETTING Howmuch to amplify the input signal (Idealy when loud the ouput should be 255)
int AudioAddition = 0;                                          //SOFT_SETTING Howmuch to add to the input signal
byte MinAudioBrightness = 1;                                    //SOFT_SETTING Minimum amount of brighness that will be applied with Audiolink
byte MaxAudioBrightness = 255;                                  //SOFT_SETTING Maximum amount of brighness that can be applied with Audiolink
byte AmountAudioAverageEnd = 6;                                 //SOFT_SETTING howmuch to smooth the LEDs responoce (Its after the math)(Max 64! recomended to keep low like 3-5)
byte AudioRawLog[255];                                          //Used to log raw audio sensory data in
const byte AudioLog_Amount = sizeof(AudioRawLog) / sizeof(AudioRawLog[0]);//Why filling this in if we can automate that? :)
byte AudioLog[AudioLog_Amount];                                 //Used to log audio sensory data in
int EveryXmsMic = 200;                                          //SOFT_SETTING Pulling rate of MIC, Delay the amount where the average is based on, this results in 16*200ms= the last 3.2 seconds
bool UpdateLEDs;                                                //If we need to physically update the LEDs
bool TimeSet = false;                                           //If the time has been set or synced, is used to tasked based on time
byte Mode;                                                      //Holds in which mode the light is currently in
byte LastMode = -1;                                             //Just to keep track if we are stepping into a new mode, and need to init that mode. -1 to force init
const int TotalLEDs = 60;                                       //The total amounts of LEDs in the strip
#define LEDSections TotalLEDs / 60                              //Amount of sections of LEDs for the Clock (2=each step is 2 LEDs so 12h=120+121 LED, 1=Default 12h=60th LED)
const int TotalLEDsClock = LEDSections * 60;                    //The amount of LEDs in the clock
int AnimationCounter;                                           //Time in seconds that a AnimationCounter Animation needs to be played
TimeS TimeCurrent;                                              //Where we save the time to
byte ModeBeforeOff = ON;
extern byte TotalAnimations;                                    //^ Required for Clock.h
extern void StartAnimation(byte ID, int Time);                  //^ Required for Clock.h
struct tm timeinfo;                                             //https://www.cplusplus.com/reference/ctime/tm/ (Blobal here to make accesable for debug '/info' page)

#include "WiFiManagerBefore.h"                                  //Define what options to use/include or to hook into WiFiManager
#include "WiFiManager/WiFiManager.h"                            //Includes <WiFi> and <WebServer.h> and setups up 'WebServer server(80)' if needed      https://github.com/jellewie/Arduino-WiFiManager

#include <FastLED.h>                                            //Include the libary FastLED (If you get a error here, make sure it's installed!)
CRGB LEDs[TotalLEDs];
CRGB RGBColor;

#include "Button/Button.h"
Button ButtonsA = {PDI_Button, INPUT, LED_BUILTIN};

#define StableAnalog_AnalogResolution 10                        //ESP32 does not support 8bit (byte) analog readings, to set to 10 and scale down to 8
#include "StableAnalog/StableAnalog.h"
StableAnalog RED   = StableAnalog(PAI_R);
StableAnalog GREEN = StableAnalog(PAI_G);
StableAnalog BLUE  = StableAnalog(PAI_B);
StableAnalog BRIGH = StableAnalog(PAI_Brightness);
StableAnalog LIGHT = StableAnalog(PAI_LIGHT);
StableAnalog AUDIO = StableAnalog(PAO_MIC);

extern void HAUpdateLED(bool Force = false);                    //We declair this funcion in "MQTT_HA.h"
#include "Functions.h"
#include "MQTT_HA.h"
#include "time.h"                                               //We need this for the clock function to get the time (Time library)
#include "Task.h"
#include "WiFiManagerLater.h"                                   //Define options of WiFiManager (can also be done before), but WiFiManager can also be called here (example for DoRequest)
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
  FastLED.setBrightness(1);                                     //Set start brightness to be amost off
  for (int i = 255; i >= 0; i = i - 255) {                      //Blink on boot
    fill_solid(&(LEDs[0]), TotalLEDs, CRGB(i, i, i));
    FastLED.show();                                             //Update
    FastLED.delay(1);
  }
  //===========================================================================
  //Set and attach pins
  //===========================================================================
  attachInterrupt(ButtonsA.PIN_Button, ISR_ButtonsA, CHANGE);
  pinMode(PAI_DisablePOTs, INPUT_PULLUP);                       //Pull the pin up, so the pin is by default HIGH if not attached
  if (digitalRead(PAI_DisablePOTs) == HIGH)                     //If the POTs are enabled with hardware
    AudioLink = false;                                          //Do not allow AudioLink if Pots are enabled
  //===========================================================================
  //Set default settings
  //===========================================================================
  pinMode(PAI_OtherDefault, INPUT_PULLUP);                      //Pull the pin up, so the pin is by default HIGH if not attached
  if (digitalRead(PAI_OtherDefault) == LOW) {                   //If the pin is pulled low by hardware, Change the default of some SOFT_SETTINGs
    BootMode = CLOCK;
    DoublePressMode = CLOCK;
  }
  //===========================================================================
  //Set up all server UrlRequest stuff
  //===========================================================================
  server.on("/",            handle_OnConnect);                  //Call the 'handleRoot' function when a client requests URL "/"
  server.on("/main",        handle_Main);
  server.on("/get",         handle_Getcolors);
  server.on("/set",         handle_Set);
  server.on("/gettasks",    Tasks_handle_GetTasks);
  server.on("/settask",     Tasks_handle_Settings);
  server.on("/time",        handle_UpdateTime);
  server.on("/info",        handle_Info);
  server.on("/reset",       handle_Reset);
  server.onNotFound(        handle_NotFound);                   //When a client requests an unknown URI
  //===========================================================================
  //Init the potmeters (This trashed their first values)
  //===========================================================================
  //analogReadResolution(StableAnalog_AnalogResolution);
  for (int i = 0; i < StableAnalog_AverageAmount + 2; i++) {
    UpdateColor(false);                                         //Trash some measurements, so we get a good average on start
    UpdateBrightness(false);
    AUDIO.ReadStable();
  }
  FastLED.setBrightness(8);                                     //Set boot Brightness
  //===========================================================================
  //Add a SYNCTIME Task, this is just to initialise (will be overwritten with user data)
  //===========================================================================
  TASK TempTask;                                                //Create a space to put a new Task in
  TempTask.Type       = SYNCTIME;                               //Set the ID of the task to SYNCTIME
  TempTask.ExectuteAt = TimeS{4, 0, 0, 0};                      //Set the init value to auto sync time at 04:00:00
  AddTask(TempTask);                                            //Add the Task command to the task list
  //===========================================================================
  //Load data from EEPROM, so we can apply the set bootmode
  //===========================================================================
  WiFiManager.LoadData();
  Mode = BootMode;                                              //Set the startup mode
#ifdef SerialEnabled
  Serial.println("SE: Booting up in mode " + String(Mode) + "=" + ConvertModeToString(Mode));
#endif //SerialEnabled
  loopLEDS();
  UpdateBrightness(true);                                       //Force Update the brightness
  if (HA_MQTT_Enabled_On_Boot)
    HA_MQTT_Enabled = true;
#ifdef SetupTime_SerialEnabled                                  //Just a way to measure setup speed, so the performance can be checked
  Serial.println("ST: Setup took ms:\t" + String(millis()));
#endif //SetupTime_SerialEnabled
}
void loop() {
#ifdef LoopTime_SerialEnabled                                   //Just a way to measure loop speed, so the performance can be checked
  static unsigned long LoopLast = 0;
  unsigned long LoopNow = micros();
  float LoopMs = (LoopNow - LoopLast) / 1000.0;
  LoopLast = LoopNow;
  Serial.println("LT: Loop took ms:\t" + String(LoopMs));
#endif //LoopTime_SerialEnabled
  WiFiManager.RunServer();                                      //Do WIFI server stuff if needed
  if (StandAlone) StandAloneAPMode();
  if (TimeSet and Mode != CLOCK) UpdateAndShowClock(false);     //If we are not in clock mode but the time has been set, update the internal time before ExecuteTask
  ExecuteTask();
  if (AnimationCounter != 0)                                    //Animation needs to be shown
    ShowAnimation(false);
  if (HA_MQTT_Enabled) {                                        //If we need to talk to MQTT
    if (HA_MQTT_Enabled_old != HA_MQTT_Enabled)                 //If this is the first time
      HaSetup();
    HaLoop();
  }
  HA_MQTT_Enabled_old = HA_MQTT_Enabled;
  EVERY_N_MILLISECONDS(1000 / 60) {                             //Limit to 60FPS
    Button_Time Value = ButtonsA.CheckButton();                 //Read buttonstate
#ifdef SerialEnabled                                            //DEBUG, print button state to serial
    if (Value.StartPress)       Serial.println("StartPress");
    if (Value.StartLongPress)   Serial.println("StartLongPress");
    if (Value.StartDoublePress) Serial.println("StartDoublePress");
    if (Value.StartRelease)     Serial.println("StartRelease");
#endif //SerialEnabled
    if (Value.StartPress) {
      if (Mode == OFF) {
        if (ModeBeforeOff == OFF)                               //If (ModeBeforeOff is OFF) then {set mode to be ON} else {set to ModeBeforeOff}
          Mode = ON;
        else
          Mode = ModeBeforeOff;
      } else {
        ModeBeforeOff = Mode;
        Mode = OFF;
      }
    }
    if (Value.StartDoublePress)
      Mode = DoublePressMode;                                   //Cool RGB color palet mode
    if (Value.StartLongPress) {
      Mode = WIFI;
      if (WiFi.status() == WL_CONNECTED) {                      //If WIFI was already started
        ShowIP();
        LastMode = Mode;
      }
    };
    if (Value.PressedLong) {                                    //If it is/was a long press
      if (Value.PressedTime > Time_ESPrestartMS - 1000)         //If we are almost resetting
        Mode = RESET;
    }
    UpdateBrightness(false);                                    //Check if manual input potmeters has changed, if so flag the update
    UpdateAudio(false);                                         //Check if the AudioLink is needed
    UpdateColor(false);                                         //Check if manual input potmeters has changed, if so flag the update
    loopLEDS();
  }
}
void loopLEDS() {
  switch (Mode) {
    case OFF: {
        if (LastMode != Mode) {
          AnimationCounter = 0;
          FastLED.clear();
          UpdateLEDs = true;
        }
      } break;
    case ON: {
        if (LastMode != Mode) {                                 //If mode changed
          AnimationCounter = 0;
          UpdateColor(true);
        }
      } break;
    case WIFI: {
        if (LastMode != Mode) {                                 //If mode changed
          AnimationCounter = 0;
          WiFiManager.Start();                                  //Start WIFI if we haven't
        }
      } break;
    case RESET: {
        if (LastMode != Mode) {                                 //If mode changed
          AnimationCounter = 0;
        }
        EVERY_N_MILLISECONDS(50) {
          LED_Flash(0, TotalLEDs, CRGB(255, 0, 0));             //Toggle the leds RED/OFF to show we are restarting soon
          UpdateLEDs = true;
        }
      } break;
    case CLOCK: {
        if (LastMode != Mode) {                                 //If mode changed
          AnimationCounter = 0;
          if (WiFi.status() != WL_CONNECTED) {                  //If no WiFi has been set-up
            if (!TimeSet)                                       //If the time has not been set before
              WiFiManager.Start();                              //Start WIFI if we haven't
            else
              WiFiManager.CheckAndReconnectIfNeeded(false);     //Try to connect to WiFi, but dont start ApMode
          }
        }
        if (AnimationCounter == 0)                              //If no (hourly) animation is playing
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
    case PHYSICS:     if (LastMode != Mode) StartAnimation(13, -2); break;
    case STANDALONE:
      if (LastMode != Mode)
        StandAlone = true;
      WiFiManager.EnableSetup(true);                            //Enable setup page
      Mode = RAINBOW;
      StartAnimation(7,  -2);
      break;
    default:
#ifdef SerialEnabled
      Serial.println("mode with ID " + String(Mode) + " not found");
#endif //SerialEnabled
      Mode = ON;
      AnimationCounter = 0;                                     //Stop animation
      break;
  }
  LastMode = Mode;
  UpdateLED();
}
//ISR must return nothing and take no arguments, so we need this sh*t
void ISR_ButtonsA() {
  ButtonsA.Pinchange();
}
