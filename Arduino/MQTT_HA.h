#include <ArduinoHA.h>                                          //https://github.com/dawidchyrzynski/arduino-home-assistant/tree/main
bool HA_MQTT_Enabled = false;
bool HA_MQTT_Enabled_old = HA_MQTT_Enabled;
bool HA_MQTT_Enabled_On_Boot = false;
IPAddress HA_BROKER_ADDR = IPAddress(0, 0, 0, 0);
String HA_BROKER_USERNAME = "";
String HA_BROKER_PASSWORD = "";

#define HA_deviceSoftwareVersion "1.0"                          //Device info - Firmware:
#define HA_deviceManufacturer "JelleWho"                        //Manufacturer
#define HA_deviceModel "Smart-light"                            //Model
#define HA_lightName1 "All"                                     //Entity ID
#define HA_lightName2 "Outer"                                   //Entity ID
byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4B};
WiFiClient client;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);
HALight light1("Smart-all", HALight::BrightnessFeature | HALight::RGBFeature); //unique LighID
HALight light2("Smart-Outer", HALight::BrightnessFeature | HALight::RGBFeature); //unique LighID

void onBrightnessCommand(uint8_t brightness, HALight* sender) {
  FastLED.setBrightness(brightness);
  UpdateLEDs = true;
  sender->setBrightness(brightness); // report brightness back to the Home Assistant
#ifdef HomeAssistant_SerialEnabled                              //Just a way to measure setup speed, so the performance can be checked
  Serial.println("HA: Change light1 brightness = " + String(brightness));
#endif //HomeAssistant_SerialEnabled
}
void onStateCommand1(bool state, HALight* sender) {
  if (state) {
    LED_Fill(0, TotalLEDs, RGBColor);                           //Change the whole LED strip
    Mode = WIFI;
  } else {
    LED_Fill(0, TotalLEDs, CRGB(0, 0, 0));                      //Change the whole LED strip
    //Mode = LastMode;
    //LastMode = -1;
     Mode = CLOCK;
     AutoBrightness = true;
  }
  UpdateLEDs = true;
  sender->setState(state);                                      //Report state back to the Home Assistant
#ifdef HomeAssistant_SerialEnabled                              //Just a way to measure setup speed, so the performance can be checked
  Serial.println("HA: Change light1 = " + String(state));
#endif //HomeAssistant_SerialEnabled
}
void onStateCommand2(bool state, HALight* sender) {
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

void HaLoop() {
  mqtt.loop();
}
void HaSetup() {
  device.setName(Name);
  device.setSoftwareVersion(HA_deviceSoftwareVersion);
  device.setManufacturer(HA_deviceManufacturer);
  device.setModel(HA_deviceModel);
  //device.setConfigurationUrl(IpAddress2String(WiFi.localIP()).c_str());

  light1.setName(HA_lightName1);
  light1.onStateCommand(onStateCommand1);
  light1.onBrightnessCommand(onBrightnessCommand);
  light1.onRGBColorCommand(onRGBColorCommand1);

  light2.setName(HA_lightName2);
  light2.onStateCommand(onStateCommand2);
  light2.onBrightnessCommand(onBrightnessCommand);
  light2.onRGBColorCommand(onRGBColorCommand2);

  mqtt.begin(HA_BROKER_ADDR, HA_BROKER_USERNAME.c_str(), HA_BROKER_PASSWORD.c_str());
  HaLoop();
  light1.setState(LEDs[TotalLEDs - 1] == CRGB(0, 0, 0) ? false : true); //When booting, inform HA about the light state
  light2.setState(LEDs[TotalLEDs - 1] == CRGB(0, 0, 0) ? false : true); //When booting, inform HA about the light state
#ifdef HomeAssistant_SerialEnabled                              //Just a way to measure setup speed, so the performance can be checked
  Serial.println("HA: Informed HA about our pressence");
#endif //HomeAssistant_SerialEnabled
}
