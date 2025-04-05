/* Written by JelleWho https://github.com/jellewie
   https://github.com/jellewie/Arduino-WiFiManager

  These are some examples how to 'hook' functions with code into the WiFiManager.
  This file and all functions are not required, but when defined before '#include "WiFiManagerUser.h"' they will be hooked in to WiFiManager
  if you want to declair functions after including WiFiManager, uses a 'extern <function name>' to declair a dummy here, so the hook will be made, but will be hooked to a later (extern if I may say) declaration

  NOTES
   DO NOT USE char(") in any of input stings on the webpage, use char(') if you need it. char(") will be replaced

  HOW TO ADD CUSTOM VALUES
   -"WiFiManagerUser_VariableNames_Defined" define this, and ass custom names for the values
   -"WiFiManager_VariableNames"  Add the description name of the value to get/set to this list
   -"EEPROM_size"     [optional] Make sure it is big enough for your needs, SIZE_SSID+SIZE_PASS+YourValues (1 byte = 1 character)
   -"Set_Value"       Set the action on what to do on startup with this value
   -"Get_Value"       [optional] Set the action on what to fill in in the boxes in the 'AP settings portal'
*/
int ToByte(String IN, int MAX = 255);
int ToByte(String IN, int MAX) {
  //Converts a String to a byte, taking into account the upper limit.
  //Used for WiFiManagerUser_Set_Value
  return IN.toInt() > MAX ? MAX : IN.toInt();
}

bool WiFiManagerUser_Set_Value(byte ValueID, String Value) {
  switch (ValueID) {                                            //Note the numbers are shifted from what is in memory, 0 is the first user value
    case 0:   BootMode           = ConvertModeToInt(Value);   return true;  break;
    case 1: {
        if (not StringIsDigit(Value))                         return false;  //No number given
        HourlyAnimationS         = ToByte(Value);             return true;
      } break;
    case 2:   DoublePressMode    = ConvertModeToInt(Value);   return true;  break;
    case 3:   AutoBrightness     = IsTrue(Value);             return true;  break;
    case 4: {
        if (not StringIsDigit(Value))                         return false;  //No number given
        AutoBrightnessN          = ToByte(Value);             return true;
      } break;
    case 5: {
        Value.replace(",", ".");                                //Replace EN to UN decimal seperator (thats what Arduino uses)
        if (not StringIsDigit(Value, '.', '-'))               return false;  //No float number given
        AutoBrightnessP          = Value.toFloat();           return true;
      } break;
    case 6: {
        if (not StringIsDigit(Value))                         return false;  //No number given
        AutoBrightnessO          = ToByte(Value);             return true;
      } break;
    case 7: {
        if (not StringIsDigit(Value))                         return false;  //No number given
        ClockHourLines           = ToByte(Value);             return true;
      } break;
    case 8:   ClockHourAnalog    = IsTrue(Value);             return true;  break;
    case 9: {
        if (not StringIsDigit(Value))                         return false;  //No number given
        LEDOffset                = ToByte(Value, TotalLEDs);  return true;
      } break;
    case 10:  ClockAnalog        = IsTrue(Value);             return true;  break;
    case 11: {
        if (Value.length() < 4)                               return false;  //Number instead of string given
        if (StringIsDigit(Value))                             return false;  //Emthy string given (to short to contain the proper data)
        timeZone = Value;
        return true;
      } break;
    case 12:   AudioLink         = IsTrue(Value);             return true;  break;
    case 13: {
        if (not StringIsDigit(Value))                         return false;  //No number given
        PotMinChange             = ToByte(Value);             return true;  break;
      }
    case 14: {
        if (not StringIsDigit(Value))                         return false;  //No number given
        PotStick                 = ToByte(Value);             return true;
      } break;
    case 15: {
        if (not StringIsDigit(Value))                         return false;  //No number given
        PotMin                   = ToByte(Value);             return true;
      } break;
    case 16: {
        if (Value.length() > sizeof(Name))                    return false; //Length is to long, it would not fit so stop here
        Value.toCharArray(Name, 16);                          return true;
      } break;
    case 33: {
        Value.replace(",", ".");                                //Replace EN to UN decimal seperator (thats what Arduino uses)
        if (not StringIsDigit(Value, '.', '-'))               return false;  //No float number given
        AudioMultiplier          = Value.toFloat();           return true;
      } break;
    case 34: {
        if (not StringIsDigit(Value, '-'))                    return false;  //No number given
        AudioAddition            = Value.toInt();             return true;
      } break;
    case 35: {
        if (not StringIsDigit(Value))                         return false;  //No number given
        MinAudioBrightness       = ToByte(Value);             return true;
      } break;
    case 36: {
        if (not StringIsDigit(Value))                         return false;  //No number given
        MaxAudioBrightness       = ToByte(Value);             return true;
      } break;
    case 37: {
        if (not StringIsDigit(Value))                         return false;  //No number given
        AmountAudioAverageEnd    = ToByte(Value);             return true;
      } break;
    case 38: {
        if (HA_MQTT_Enabled)                                  return false;  //Can only be changed before MQTT is enabled/settup
        IPAddress result = String2IpAddress(Value);
        if (result == IPAddress(0, 0, 0, 0))                  return false; //No valid IP
        HA_BROKER_ADDR           = result;                    return true;
      } break;
    case 39: {
        if (HA_MQTT_Enabled)                                  return false;  //Can only be changed before MQTT is enabled/settup
        HA_BROKER_USERNAME       = Value;                     return true;
      } break;
    case 40: {
        if (HA_MQTT_Enabled)                                  return false;  //Can only be changed before MQTT is enabled/settup
        HA_BROKER_PASSWORD       = Value;                     return true;
      } break;
    case 41: HA_MQTT_Enabled_On_Boot  = IsTrue(Value);        return true;  break;
    case 42: {
        int Val = Value.toInt();
        if (Val < 0 or Val > 86400000)                        return false;
        HAEveryXmsReconnect = Val;                            return true;
      } break;
    case 43: {
        if (not StringIsDigit(Value))                         return false;  //No number given
        EveryXmsMic              = Value.toInt();             return true;
      } break;
    //==============================
    //Tasks
    //==============================
    default:
      if (ValueID < 17 + TaskLimit) {
        byte i = ValueID - 17;
        String _Vars[5];                                        //Create a space to but the cut string in
        CutVariable(Value, &_Vars[0], 5);                       //Deconstruct the string, and put it into parts
        TASK TempTask;                                          //Create a space to put a new Task in
        TempTask.Type               = constrain(_Vars[0].toInt(), 0, 255);  //Set the ID of the task
        if (TempTask.Type != 0 and TempTask.Type != SAVEEEPROM) { //If a task ID is given, and it was not SAVEEEPROM
          RemoveTask(i);                                        //Clear the spot this should go into, NOTE: this might not be an outdated value, but should be fine since the call function loops thought the list
          TempTask.ExectuteAt.HH    = constrain(_Vars[1].toInt(), 0, 23);
          TempTask.ExectuteAt.MM    = constrain(_Vars[2].toInt(), 0, 59);
          TempTask.ExectuteAt.SS    = constrain(_Vars[3].toInt(), 0, 59);
          TempTask.Var              = _Vars[4];
          AddTask(TempTask);                                    //Add the command to the task list
        }
        return true;
      }
  }
  return false;                                                 //Report back that the ValueID is unknown, and we could not set it
}
String WiFiManagerUser_Get_Value(byte ValueID, bool Safe, bool Convert) {
  //if its 'Safe' to return the real value (for example the password will return '****' or '1234')
  //'Convert' the value to a readable string for the user (bool '0/1' to 'FALSE/TRUE')
  switch (ValueID) {                                            //Note the numbers are shifted from what is in memory, 0 is the first user value
    case 0:   return Convert ? ConvertModeToString(BootMode)        : String(BootMode);         break;
    case 1:   return String(HourlyAnimationS);                                                  break;
    case 2:   return Convert ? ConvertModeToString(DoublePressMode) : String(DoublePressMode);  break;
    case 3:   return Convert ? IsTrueToString(AutoBrightness)       : String(AutoBrightness);   break;
    case 4:   return String(AutoBrightnessN);                                                   break;
    case 5:   return String(AutoBrightnessP);                                                   break;
    case 6:   return String(AutoBrightnessO);                                                   break;
    case 7:   return String(ClockHourLines);                                                    break;
    case 8:   return Convert ? IsTrueToString(ClockHourAnalog)      : String(ClockHourAnalog);  break;
    case 9:   return String(LEDOffset);                                                         break;
    case 10:  return Convert ? IsTrueToString(ClockAnalog)          : String(ClockAnalog);      break;
    case 11:  return timeZone;                                                                  break;
    case 12:  return Convert ? IsTrueToString(AudioLink)            : String(AudioLink);        break;
    case 13:  return String(PotMinChange);                                                      break;
    case 14:  return String(PotStick);                                                          break;
    case 15:  return String(PotMin);                                                            break;
    case 16:  return String(Name);                                                              break;
    case 33:  return String(AudioMultiplier);                                                   break;
    case 34:  return String(AudioAddition);                                                     break;
    case 35:  return String(MinAudioBrightness);                                                break;
    case 36:  return String(MaxAudioBrightness);                                                break;
    case 37:  return String(AmountAudioAverageEnd);                                             break;
    case 38:  return IpAddress2String(HA_BROKER_ADDR);                                          break;
    case 39:  return HA_BROKER_USERNAME;                                                        break;
    case 40: {
        String Return_Value = "";
        if (Safe)                                                 //If's it's safe to return password.
          Return_Value += String(HA_BROKER_PASSWORD);
        else {
          for (byte i = 0; i < String(HA_BROKER_PASSWORD).length(); i++)
            Return_Value += "*";
        }
        return Return_Value;
      }
    case 41:  return Convert ? IsTrueToString(HA_MQTT_Enabled_On_Boot) : String(HA_MQTT_Enabled_On_Boot); break;
    case 42:  return String(HAEveryXmsReconnect);                                              break;
    case 43:  return String(EveryXmsMic);                                                    break;
    //==============================
    //Tasks
    //==============================
    default:
      if (ValueID < 17 + TaskLimit) {
        byte i = ValueID - 17;
        if (TaskList[i].Type != 0 and TaskList[i].ExectuteAt.Ticks == 0)
          return String(TaskList[i].Type) + "," + String(TaskList[i].ExectuteAt.HH) + "," + String(TaskList[i].ExectuteAt.MM) + "," + String(TaskList[i].ExectuteAt.SS) + "," + TaskList[i].Var;
        return "0";
      }
  }
  return "";
}
void WiFiManagerUser_Status_Start() {                                   //Called before start of WiFi
  LED_Fill(LEDtoPosition(0),                  TotalLEDsClock,     CRGB(255, 0, 255), TotalLEDsClock); //Turn all LEDs purple 2222
  LED_Fill(LEDtoPosition(0),                  TotalLEDsClock / 4, CRGB(0  , 0, 255), TotalLEDsClock); //Turn 1th quater blue 1222
  LED_Fill(LEDtoPosition(TotalLEDsClock / 2), TotalLEDsClock / 4, CRGB(0  , 0, 255), TotalLEDsClock); //Turn 2rd quater blue 1212
  FastLED.show();                                                       //Update leds to show wifi is starting
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}
void WiFiManagerUser_Status_Done() {                            //Called after succesfull connection to WiFi
  WiFiManager.StartServer();                                    //Enable responce to web request
  WiFiManager.EnableSetup(true);                                //Enable the setup page, disable for more security
  digitalWrite(LED_BUILTIN, LOW);
  if (BootMode != OFF) {
    ClockClear();
    Mode = BootMode;                                            //Go into the right mode
  } else {
    LED_Fill(LEDtoPosition(0), TotalLEDsClock, CRGB(0, 255, 0), TotalLEDsClock); //Turn all Clock LEDs green
    FastLED.show();                                             //Update leds to show wifi is done
  }
  if (!TimeSet) UpdateTime();                                   //If no time set, try to get an time from the internet we have just connected to
}
void WiFiManagerUser_Status_Blink() {                           //Used when trying to connect/not connected
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}
void WiFiManagerUser_Status_StartAP() {                                 //Called before start of APmode
  LED_Fill(LEDtoPosition(0),                  TotalLEDsClock,     CRGB(255, 0, 255), TotalLEDsClock); //Turn all LEDs purple 2222
  LED_Fill(LEDtoPosition(0),                  TotalLEDsClock / 4, CRGB(255, 0, 0  ), TotalLEDsClock); //Turn 1th quater red  1222
  LED_Fill(LEDtoPosition(TotalLEDsClock / 2), TotalLEDsClock / 4, CRGB(255, 0, 0  ), TotalLEDsClock); //Turn 2rd quater red  1212
  FastLED.show();                                                       //Update leds to show we are entering APmode
  FastLED.delay(1);
#ifdef LEDstatus_SerialEnabled
  Serial.println("LS: Setting LEDs to 'entering APmode'");
#endif //LEDstatus_SerialEnabled
}
bool WiFiManagerUser_HandleAP() {                               //Called when in the While loop in APMode, this so you can exit it
  //Return true to leave APmode
#define TimeOutApMode 15 * 60 * 1000;                           //Example for a timeout, (time in ms)
  unsigned long StopApAt = millis() + TimeOutApMode;
  if (millis() > StopApAt)    return true;                      //If we are running for to long, then flag we need to exit APMode
  return false;

  Button_Time Value = ButtonsA.CheckButton();                   //Read buttonstate
  if (Value.StartLongPress) {
#ifdef SerialEnabled
    Serial.println("WM: StartLongPress; reset BootMode and restart");
#endif //SerialEnabled
    BootMode = OFF;                                             //Change bootmode (so we wont enable WIFI on startup)
    WiFiManager.WriteEEPROM();
    ESP.restart();                                              //Restart the ESP
  }
  return false;
}
byte StandAloneAPMode() {
  static bool Started = false;
  if (!Started) {
    WiFi.mode(WIFI_AP_STA);                               //https://github.com/espressif/arduino-esp32/blob/1287c529330d0c11853b9f23ddf254e4a0bc9aaf/libraries/WiFi/src/WiFiType.h#L33
    if (!WiFi.softAP(Name))                                       //config doesn't seem to work, so do not use it: 'WiFi.softAPConfig(ap_local_IP, ap_gateway, ap_subnet)'
      return 2;
  }
  WiFiManager.handleClient();
  return 1;
}
