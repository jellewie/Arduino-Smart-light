#include <ArduinoHA.h>                                          //https://github.com/dawidchyrzynski/arduino-home-assistant/tree/main
bool HA_MQTT_Enabled = false;
bool HA_MQTT_Enabled_old = HA_MQTT_Enabled;
bool HA_MQTT_Enabled_On_Boot = false;
IPAddress HA_BROKER_ADDR = IPAddress(0, 0, 0, 0);
String HA_BROKER_USERNAME = "";
String HA_BROKER_PASSWORD = "";
int8_t RestoreToMode = Mode;
unsigned long HAEveryXmsReconnect = 60 * 60 * 1000;             //On which interfall to check if WiFi still works

#define HA_deviceSoftwareVersion "1.0"                          //Device info - Firmware:
#define HA_deviceManufacturer "JelleWho"                        //Manufacturer
#define HA_deviceModel "Smart-light"                            //Model
#define HA_lightName1 "All"                                     //Entity ID
#define HA_lightName2 "Outer"                                   //Entity ID
#define HA_ModeName "Mode"                                      //Entity ID
byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4B};
WiFiClient client;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);
HALight light1("Smart-all", HALight::BrightnessFeature | HALight::RGBFeature); //unique LighID
HALight light2("Smart-Outer", HALight::BrightnessFeature | HALight::RGBFeature); //unique LighID
HASelect mySelect("Mode");                                      //Dropdown menu to select mode

HALight::RGBColor HAConvertColor(const CRGB& in) {
  return HALight::RGBColor(in[0], in[1], in[2]);
}
void HAUpdateLED(bool Force) {
  if (!HA_MQTT_Enabled) return;                                 //Don't update if we don't need to
  light1.setCurrentBrightness(FastLED.getBrightness());
  light1.setCurrentRGBColor(HAConvertColor(LEDs[TotalLEDs - 1]));
  light1.setState(LEDs[TotalLEDs - 1] == CRGB(0, 0, 0) ? false : true, Force);
  light2.setCurrentBrightness(FastLED.getBrightness());
  light2.setCurrentRGBColor(HAConvertColor(LEDs[TotalLEDs - 1]));
  light2.setState(LEDs[TotalLEDs - 1] == CRGB(0, 0, 0) ? false : true, Force);
}
void onBrightnessCommand(uint8_t brightness, HALight* sender) {
  if (brightness == 0)
    AutoBrightness = true;
  FastLED.setBrightness(brightness);
  UpdateLEDs = true;
  sender->setBrightness(brightness);                            //report brightness back to the Home Assistant
#ifdef HomeAssistant_SerialEnabled                              //Just a way to measure setup speed, so the performance can be checked
  Serial.println("HA: Change light1 brightness = " + String(brightness));
#endif //HomeAssistant_SerialEnabled
}
void onStateCommand1(bool state, HALight* sender) {
  if (RGBColor == CRGB(0, 0, 0)) RGBColor = CRGB(255, 255, 255);//Set default color
  static bool OLDstate = state;
  if (state) {
    LED_Fill(0, TotalLEDs, RGBColor);                           //Change the whole LED strip
    if (OLDstate != state)
      RestoreToMode = Mode;
    //    RestoreToAutoBrightness = AutoBrightness;
    Mode = WIFI;
  } else {
    LED_Fill(0, TotalLEDs, CRGB(0, 0, 0));                      //Change the whole LED strip
    if (OLDstate != state)
      Mode = RestoreToMode;
    //    AutoBrightness = RestoreToAutoBrightness;
  }
  OLDstate = state;
  UpdateLEDs = true;
  sender->setState(state);                                      //Report state back to the Home Assistant
#ifdef HomeAssistant_SerialEnabled                              //Just a way to measure setup speed, so the performance can be checked
  Serial.println("HA: Change light1 = " + String(state));
#endif //HomeAssistant_SerialEnabled
}
void onStateCommand2(bool state, HALight* sender) {
  if (RGBColor == CRGB(0, 0, 0)) RGBColor = CRGB(255, 255, 255);//Set default color
  if (state) {
    LED_Fill(TotalLEDsClock, TotalLEDs - TotalLEDsClock, RGBColor);//Change all NON-Clock LEDs
  } else {
    LED_Fill(TotalLEDsClock, TotalLEDs - TotalLEDsClock, CRGB(0, 0, 0));//Clear all NON-Clock LEDs
  }
  sender->setState(state);                                      //Report state back to the Home Assistant
#ifdef HomeAssistant_SerialEnabled                              //Just a way to measure setup speed, so the performance can be checked
  Serial.println("HA: Change light2 = " + String(state));
#endif //HomeAssistant_SerialEnabled
}
void onRGBColorCommand1(HALight::RGBColor color, HALight* sender) {
  RGBColor = CRGB(color.red, color.green, color.blue);
  LED_Fill(0, TotalLEDs, RGBColor);                             //Change the whole LED strip
  UpdateLEDs = true;
  sender->setRGBColor(color);                                   //Report color back to the Home Assistant
#ifdef HomeAssistant_SerialEnabled                              //Just a way to measure setup speed, so the performance can be checked
  Serial.println("HA: Change light1 color = " + String(color.red) + "," + String(color.green) + "," + String(color.blue));
#endif //HomeAssistant_SerialEnabled
}
void onRGBColorCommand2(HALight::RGBColor color, HALight* sender) {
  RGBColor = CRGB(color.red, color.green, color.blue);
  LED_Fill(TotalLEDsClock, TotalLEDs - TotalLEDsClock, RGBColor);//Change all NON-Clock LEDs
  UpdateLEDs = true;
  sender->setRGBColor(color);                                   //Report color back to the Home Assistant
#ifdef HomeAssistant_SerialEnabled                              //Just a way to measure setup speed, so the performance can be checked
  Serial.println("HA: Change light2 color = " + String(color.red) + "," + String(color.green) + "," + String(color.blue));
#endif //HomeAssistant_SerialEnabled
}
void onSelectCommand(int8_t index, HASelect* sender) {
  if (index < 0 or index >= Modes_Amount)                        //Sanity check
    return;
  LastMode = -1;                                                //Make sure we init the new mode
  Mode = index;
  sender->setState(index);                                      //report the selected option back to the HA
}
extern void HaSetup(bool LoopAfter = false);
void HaLoop() {
  mqtt.loop();
  static unsigned long LastTime;
  if (TickEveryXms(&LastTime, HAEveryXmsReconnect)) {
    if (WiFiManager.CheckAndReconnectIfNeeded(false))           //Try to connect to WiFi, but dont start ApMode
      light1.setState(LEDs[TotalLEDs - 1] == CRGB(0, 0, 0) ? false : true, true);
    HaSetup();
  static int8_t HALastMode = -1;
  if (HALastMode != Mode) {                                     //If the HA mode is not the same as the current mode
    HALastMode = Mode;
    mySelect.setState(Mode);
  }
}
void HaSetup(bool LoopAfter) {
  device.setName(Name);
  device.setSoftwareVersion(HA_deviceSoftwareVersion);
  device.setManufacturer(HA_deviceManufacturer);
  device.setModel(HA_deviceModel);
  //String URL = "http://" + IpAddress2String(WiFi.localIP());
  //static String URL = "http://192.168.50.205";
  //static char configUrl[30];  // Adjust size as needed, large enough to hold the URL
  //URL.toCharArray(configUrl, sizeof(configUrl));
  //device.setConfigurationUrl(configUrl);

  light1.setName(HA_lightName1);
  light1.onStateCommand(onStateCommand1);
  light1.onBrightnessCommand(onBrightnessCommand);
  light1.onRGBColorCommand(onRGBColorCommand1);

  light2.setName(HA_lightName2);
  light2.onStateCommand(onStateCommand2);
  light2.onBrightnessCommand(onBrightnessCommand);
  light2.onRGBColorCommand(onRGBColorCommand2);

  mySelect.setName(HA_ModeName);
  mySelect.setOptions("OFF;ON;WIFI;RESET;CLOCK;BLINK;BPM;CONFETTI;FLASH;GLITTER;JUGGLE;MOVE;RAINBOW;SINELON;SINELON2;SMILEY;FLASH2;PACMAN;PHYSICS;STANDALONE"); // use semicolons as separator of options
  mySelect.onCommand(onSelectCommand);

  HAUpdateLED(true);

  mqtt.begin(HA_BROKER_ADDR, HA_BROKER_USERNAME.c_str(), HA_BROKER_PASSWORD.c_str());
  if (LoopAfter)
    HaLoop();
#ifdef HomeAssistant_SerialEnabled                              //Just a way to measure setup speed, so the performance can be checked
  Serial.println("HA: Informed HA about our pressence");
#endif //HomeAssistant_SerialEnabled
}
