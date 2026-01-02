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
#define HA_deviceModel "smart-tree"                             //Model
#define HA_EveryXmsUpdate 60 * 1000                             //How often to send updates to HA
byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4D};              //We need to tel HA we are a new device
WiFiClient client;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);

HALight HAlight1("smart-clock-all", HALight::BrightnessFeature | HALight::RGBFeature); //unique LighID
HALight HAlight2("smart-clock-Outer", HALight::BrightnessFeature | HALight::RGBFeature); //unique LighID
HASensorNumber HALDR("smart-clock-ldr");                        //unique SensorNumberID used to send the LDR data to HA
HASelect HAMode("smart-clock-Mode");                            //Dropdown menu to select mode
HASelect HABootMode("smart-clock-BootMode");                    //Dropdown menu to select bootmode
HASelect HADoublepressMode("smart-clock-doublepressmode");     //Dropdown menu to select doublepress mode
HASwitch HAAutoBrightness("smart-clock-autobrightness");
HASwitch HAAnalogHours("smart-clock-analoghours");
HASwitch HAClockAnalog("smart-clock-analogclock");
HANumber HAHourlyAnimation("smart-clock-hourlyanimation");
HANumber HAHourlyLines("smart-clock-hourlylines");

HALight::RGBColor HAConvertColor(const CRGB& in) {
  return HALight::RGBColor(in[0], in[1], in[2]);
}
void HAUpdateLED(bool Force) {
  if (!HA_MQTT_Enabled) return;                                 //Don't update if we don't need to
  HAlight1.setCurrentBrightness(FastLED.getBrightness());
  HAlight1.setCurrentRGBColor(HAConvertColor(LEDs[TotalLEDs - 1]));
  HAlight1.setState(LEDs[TotalLEDs - 1] == CRGB(0, 0, 0) ? false : true, Force);
  HAlight2.setCurrentBrightness(FastLED.getBrightness());
  HAlight2.setCurrentRGBColor(HAConvertColor(LEDs[TotalLEDs - 1]));
  HAlight2.setState(LEDs[TotalLEDs - 1] == CRGB(0, 0, 0) ? false : true, Force);
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
    Mode = WIFI;
  } else {
    LED_Fill(0, TotalLEDs, CRGB(0, 0, 0));                      //Change the whole LED strip
    if (OLDstate != state)
      Mode = RestoreToMode;
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
void onModeCommand(int8_t index, HASelect* sender) {
  if (index < 0 or index >= Modes_Amount)                       //Sanity check
    return;
  LastMode = -1;                                                //Make sure we init the new mode
  Mode = index;
  sender->setState(index);                                      //report the selected option back to the HA
}
void onBootModeCommand(int8_t index, HASelect* sender) {
  if (index < 0 or index >= Modes_Amount)                       //Sanity check
    return;
  LastMode = -1;                                                //Make sure we init the new mode
  Mode = index;
  sender->setState(index);                                      //report the selected option back to the HA
}
void onDoubplepressModeCommand(int8_t index, HASelect* sender) {
  if (index < 0 or index >= Modes_Amount)                       //Sanity check
    return;
  LastMode = -1;                                                //Make sure we init the new mode
  Mode = index;
  sender->setState(index);                                      //report the selected option back to the HA
}
void onAutoBrightnessCommand(bool state, HASwitch* sender){
    AutoBrightness = state;
    if (AutoBrightness) AudioLink = false;
    UpdateBrightness(true);
    sender->setState(state);
}
void onAnalogHoursCommand(bool state, HASwitch* sender){
    ClockHourAnalog = state;
    sender->setState(state);
}
void onClockAnalogCommand(bool state, HASwitch* sender){
    ClockAnalog = state;
    sender->setState(state);
}
void onHourlyAnimationSCommand(HANumeric number, HANumber* sender){
  if (!number.isSet()) {
      // the reset command was send by Home Assistant
  } else {
    HourlyAnimationS = number.toInt8();
  }
  sender->setState(number); 
}
void onClockHourLinesCommand(HANumeric number, HANumber* sender){
  if (!number.isSet()) {
      // the reset command was send by Home Assistant
  } else {
    ClockHourLines = number.toInt8();
  }
  sender->setState(number); 
}
extern void HaSetup(bool LoopAfter = false);
void HaLoop() {
  mqtt.loop();
  static unsigned long LastTime;
  if (TickEveryXms(&LastTime, HAEveryXmsReconnect)) {
    if (WiFiManager.CheckAndReconnectIfNeeded(false)){          //Try to connect to WiFi, but dont start ApMode
       HaSetup();
       HAlight1.setState(LEDs[TotalLEDs - 1] == CRGB(0, 0, 0) ? false : true, true);
    }
  }
  static unsigned long LastTime2;
  if (TickEveryXms(&LastTime2, HA_EveryXmsUpdate)) {
    int16_t ReadHALDR = 4096 - (analogRead(PAI_LIGHT) * 4);     //Inverse so dark=0 and bright=4096
    HALDR.setValue(ReadHALDR);                
  }
  static int8_t HALastMode = -1;
  if (HALastMode != Mode) {                                     //If the HA mode is not the same as the current mode
    HALastMode = Mode;
    HAMode.setState(Mode);
  }
  static int8_t HALastBootMode = -1;
  if (HALastBootMode != BootMode) {                             //If the HA mode is not the same as the current mode
    HALastBootMode = BootMode;
    HABootMode.setState(BootMode);
  }
  static int8_t HALastDoublePressMode = -1;
  if (HALastDoublePressMode != DoublePressMode) {               //If the HA mode is not the same as the current mode
    HALastDoublePressMode = DoublePressMode;
    HADoublepressMode.setState(DoublePressMode);
  }
  static int8_t HALastAutoBrightness = !AutoBrightness;
  if (HALastAutoBrightness != AutoBrightness) {                 //If the HA mode is not the same as the current mode
    HALastAutoBrightness = AutoBrightness;
    HAAutoBrightness.setState(AutoBrightness);
  }
  static int8_t HALastClockHourAnalog = !ClockHourAnalog;
  if (HALastClockHourAnalog != ClockHourAnalog) {               //If the HA mode is not the same as the current mode
    HALastClockHourAnalog = ClockHourAnalog;
    HAAnalogHours.setState(ClockHourAnalog);
  }
  static int8_t HALastClockAnalog = !ClockAnalog;
  if (HALastClockAnalog != ClockAnalog) {                       //If the HA mode is not the same as the current mode
    HALastClockAnalog = ClockAnalog;
    HAClockAnalog.setState(ClockAnalog);
  }
  static int8_t HALastHourlyAnimationS = !HourlyAnimationS;
  if (HALastHourlyAnimationS != HourlyAnimationS) {             //If the HA mode is not the same as the current mode
    HALastHourlyAnimationS = HourlyAnimationS;
    HAHourlyAnimation.setState(HourlyAnimationS);
  }
  static int8_t HALastClockHourLines = !ClockHourLines;
  if (HALastClockHourLines != ClockHourLines) {             //If the HA mode is not the same as the current mode
    HALastClockHourLines = ClockHourLines;
    HAHourlyLines.setState(ClockHourLines);
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

  HAlight1.setName("All");
  HAlight1.onStateCommand(onStateCommand1);
  HAlight1.onBrightnessCommand(onBrightnessCommand);
  HAlight1.onRGBColorCommand(onRGBColorCommand1);

  HAlight2.setName("Outer");
  HAlight2.onStateCommand(onStateCommand2);
  HAlight2.onBrightnessCommand(onBrightnessCommand);
  HAlight2.onRGBColorCommand(onRGBColorCommand2);

  HALDR.setName("Light");
  HALDR.setUnitOfMeasurement("lx");
  HALDR.setIcon("mdi:brightness-5");

  String AvailableModes;
  for (size_t i = 0; i < Modes_Amount; i++) {
    if (i > 0) {
        AvailableModes += ';';
    }
    AvailableModes += ModesString[i];
  }
  HAMode.setName("Mode");
  HAMode.setOptions(AvailableModes.c_str());
  HAMode.onCommand(onModeCommand);

  HABootMode.setName("Boot mode");
  HABootMode.setOptions(AvailableModes.c_str());
  HABootMode.onCommand(onBootModeCommand);

  HADoublepressMode.setName("Doubplepress mode");
  HADoublepressMode.setOptions(AvailableModes.c_str());
  HADoublepressMode.onCommand(onDoubplepressModeCommand); 

  HAAutoBrightness.setName("Auto brightness");
  HAAutoBrightness.onCommand(onAutoBrightnessCommand);

  HAAnalogHours.setName("Analog hours");
  HAAnalogHours.onCommand(onAnalogHoursCommand);

  HAClockAnalog.setName("Analog clock");
  HAClockAnalog.onCommand(onClockAnalogCommand);

  HAHourlyAnimation.setName("Hourly animation");
  HAHourlyAnimation.onCommand(onHourlyAnimationSCommand);
  HAHourlyAnimation.setMin(0);
  HAHourlyAnimation.setMax(255);
  HAHourlyAnimation.setStep(1);

  HAHourlyLines.setName("Hour lines");
  HAHourlyLines.onCommand(onClockHourLinesCommand);
  HAHourlyLines.setMin(0);
  HAHourlyLines.setMax(255);
  HAHourlyLines.setStep(1);

  HAUpdateLED(true);

  mqtt.begin(HA_BROKER_ADDR, HA_BROKER_USERNAME.c_str(), HA_BROKER_PASSWORD.c_str());
  if (LoopAfter)
    HaLoop();
#ifdef HomeAssistant_SerialEnabled                              //Just a way to measure setup speed, so the performance can be checked
  Serial.println("HA: Informed HA about our pressence");
#endif //HomeAssistant_SerialEnabled
}
