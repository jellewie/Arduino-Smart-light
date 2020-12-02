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
byte ToByte(String IN, byte MAX = 255);
byte ToByte(String IN, byte MAX) {
  //Converts a String to a byte, taking into account the upper limit.
  //Used for WiFiManagerUser_Set_Value
  return IN.toInt() > MAX ? MAX : IN.toInt();
}
bool WiFiManagerUser_Set_Value(byte ValueID, String Value) {
  switch (ValueID) {                                            //Note the numbers are shifted from what is in memory, 0 is the first user value
    case 0:   BootMode           = ConvertModeToInt(Value);   return true;  break;
    case 1:   HourlyAnimationS   = ToByte(Value);             return true;  break;
    case 2:   DoublePressMode    = ConvertModeToInt(Value);   return true;  break;
    case 3:   AutoBrightness     = IsTrue(Value);             return true;  break;
    case 4:   AutoBrightnessN    = ToByte(Value);             return true;  break;
    case 5:   AutoBrightnessP    = Value.toFloat();           return true;  break;
    case 6:   AutoBrightnessO    = ToByte(Value);             return true;  break;
    case 7:   ClockHourLines     = ToByte(Value);             return true;  break;
    case 8:   ClockHourAnalog    = IsTrue(Value);             return true;  break;
    case 9:   ClockOffset        = ToByte(Value, TotalLEDs);  return true;  break;
    case 10:  ClockAnalog        = IsTrue(Value);             return true;  break;
    case 11:  gmtOffset_sec      = Value.toInt();             return true;  break;
    case 12:  daylightOffset_sec = Value.toInt();             return true;  break;
    case 13:  PotMinChange       = ToByte(Value);             return true;  break;
    case 14:  PotStick           = ToByte(Value);             return true;  break;
    case 15:  PotMin             = ToByte(Value);             return true;  break;
    case 16:  Value.toCharArray(Name, 16);                    return true;  break;
    //==============================
    //Tasks
    //==============================
    default:
      if (ValueID < 17 + 8) {
        byte i = ValueID - 17;
        String _Vars[5];                                                    //Create a space to but the cut string in
        CutVariable(Value, &_Vars[0], 5);                                   //Deconstruct the string, and put it into parts
        TASK TempTask;                                                      //Create a space to put a new Task in
        TempTask.Type                 = constrain(_Vars[0].toInt(), 0, 255);  //Set the ID of the task
        if (TempTask.Type != 0 and TempTask.Type != SAVEEEPROM) {               //If a task ID is given, and it was not SAVEEEPROM
          RemoveTask(i);            //Clear the spot this should go into, NOTE: this might not be an outdated value, but should be fine since the call function loops thought the list
          TempTask.ExectuteAt.HH    = constrain(_Vars[1].toInt(), 0, 23);
          TempTask.ExectuteAt.MM    = constrain(_Vars[2].toInt(), 0, 59);
          TempTask.ExectuteAt.SS    = constrain(_Vars[3].toInt(), 0, 59);
          TempTask.Var              = _Vars[4];
          AddTask(TempTask);                                                //Add the command to the task list
        }
        return true;
      }
  }
  return false;                                                             //Report back that the ValueID is unknown, and we could not set it
}
String WiFiManagerUser_Get_Value(byte ValueID, bool Safe, bool Convert) {
  //if its 'Safe' to return the real value (for example the password will return '****' or '1234')
  //'Convert' the value to a readable string for the user (bool '0/1' to 'FALSE/TRUE')
  switch (ValueID) {                                                        //Note the numbers are shifted from what is in memory, 0 is the first user value
    case 0:   return Convert ? ConvertModeToString(BootMode)        : String(BootMode);         break;
    case 1:   return String(HourlyAnimationS);                                                  break;
    case 2:   return Convert ? ConvertModeToString(DoublePressMode) : String(DoublePressMode);  break;
    case 3:   return Convert ? IsTrueToString(AutoBrightness)       : String(AutoBrightness);   break;
    case 4:   return String(AutoBrightnessN);                                                   break;
    case 5:   return String(AutoBrightnessP);                                                   break;
    case 6:   return String(AutoBrightnessO);                                                   break;
    case 7:   return String(ClockHourLines);                                                    break;
    case 8:   return Convert ? IsTrueToString(ClockHourAnalog)      : String(ClockHourAnalog);  break;
    case 9:   return String(ClockOffset);                                                       break;
    case 10:  return Convert ? IsTrueToString(ClockAnalog)          : String(ClockAnalog);      break;
    case 11:  return String(gmtOffset_sec);                                                     break;
    case 12:  return String(daylightOffset_sec);                                                break;
    case 13:  return String(PotMinChange);                                                      break;
    case 14:  return String(PotStick);                                                          break;
    case 15:  return String(PotMin);                                                            break;
    case 16:  return String(Name);                                                              break;
    //==============================
    //Tasks
    //==============================
    default:
      if (ValueID < 17 + 8) {
        byte i = ValueID - 17;
        if (TaskList[i].Type != 0 and TaskList[i].ExectuteAt.Ticks == 0)
          return String(TaskList[i].Type) + "," + String(TaskList[i].ExectuteAt.HH) + "," + String(TaskList[i].ExectuteAt.MM) + "," + String(TaskList[i].ExectuteAt.SS) + "," + TaskList[i].Var;
        return "0";
      }
  }
  return "";
}
void WiFiManagerUser_Status_Start() {                                   //Called before start of WiFi
  fill_solid(&(LEDs[0]),             TotalLEDs,     CRGB(255, 0, 255)); //Turn all LEDs purple 2222
  fill_solid(&(LEDs[0]),             TotalLEDs / 4, CRGB(0,   0, 255)); //Turn 1th quater blue 1222
  fill_solid(&(LEDs[TotalLEDs / 2]), TotalLEDs / 4, CRGB(0,   0, 255)); //Turn 2rd quater blue 1212
  FastLED.show();                                                       //Update leds to show wifi is starting
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}
void WiFiManagerUser_Status_Done() {                                    //Called after succesfull connection to WiFi
  WiFiManager.StartServer();                                            //Enable responce to web request
  WiFiManager.EnableSetup(true);                                        //Enable the setup page, disable for more security
  digitalWrite(LED_BUILTIN, LOW);
  if (BootMode != OFF) {
    FastLED.clear();
    Mode = BootMode;                                                    //Go into the right mode
  } else {
    fill_solid(&(LEDs[0]),           TotalLEDs,     CRGB(0, 255, 0));   //Turn all LEDs green
    FastLED.show();                                                     //Update leds to show wifi is done
  }
  if (!TimeSet) UpdateTime();                                           //If no time set, try to get an time from the internet we have just connected to
}
void WiFiManagerUser_Status_Blink() {                                   //Used when trying to connect/not connected
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}
void WiFiManagerUser_Status_StartAP() {                                 //Called before start of APmode
  fill_solid(&(LEDs[0]),             TotalLEDs,     CRGB(255, 0, 255)); //Turn all LEDs purple 2222
  fill_solid(&(LEDs[0]),             TotalLEDs / 4, CRGB(255, 0, 0  )); //Turn 1th quater red  1222
  fill_solid(&(LEDs[TotalLEDs / 2]), TotalLEDs / 4, CRGB(255, 0, 0  )); //Turn 2rd quater red  1212
  FastLED.show();                                                       //Update leds to show we are entering APmode
  FastLED.delay(1);
#ifdef LEDstatus_SerialEnabled
  Serial.println("LS: Setting LEDs to 'entering APmode'");
#endif //LEDstatus_SerialEnabled
}
bool WiFiManagerUser_HandleAP() {                                       //Called when in the While loop in APMode, this so you can exit it
  //Return true to leave APmode
#define TimeOutApMode 15 * 60 * 1000;                                   //Example for a timeout, (time in ms)
  unsigned long StopApAt = millis() + TimeOutApMode;
  if (millis() > StopApAt)    return true;                              //If we are running for to long, then flag we need to exit APMode
  return false;

  Button_Time Value = ButtonsA.CheckButton();                           //Read buttonstate
  if (Value.StartLongPress) {
#ifdef SerialEnabled
    Serial.println("WM: StartLongPress; reset BootMode and restart");
#endif //SerialEnabled
    BootMode = OFF;                                                     //Change bootmode (so we wont enable WIFI on startup)
    WiFiManager.WriteEEPROM();
    ESP.restart();                                                      //Restart the ESP
  }
  return false;
}
