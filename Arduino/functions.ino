/* Written by JelleWho https://github.com/jellewie */
void UpdateColor(bool ForceUpdate) {
  POT R = RED.ReadStable(PotMinChange, PotStick, AverageAmount);
  POT G = GREEN.ReadStable(PotMinChange, PotStick, AverageAmount);
  POT B = BLUE.ReadStable(PotMinChange, PotStick, AverageAmount);
  //  if (R.Value == 0 and G.Value == 0 and B.Value == 0) { //If all LEDs are invisible
  //    R.Value = 1 + constrain(BN, 0, 254);
  //  }     //Tried to turn on LED strip soft red when all colors are invisble.
  if (R.Changed or G.Changed or B.Changed or ForceUpdate) {
    if (Mode == ON or (R.Changed > PotMin or G.Changed > PotMin or B.Changed > PotMin)) {
#ifdef SerialEnabled
      Serial.println("Manual color changed to " +
                     String(R.Value) + "(" + String(R.Changed) + ")," +
                     String(G.Value) + "(" + String(G.Changed) + ")," +
                     String(B.Value) + "(" + String(B.Changed) + ")");
      if (Mode != ON)
        Serial.println(" Mode changed from " + String(Mode) + "=" + ConvertModeToString(Mode) + " to ON/manual");
#endif //SerialEnabled
      Mode = ON;
      fill_solid(&(LEDs[0]), TotalLEDs, CRGB(R.Value, G.Value, B.Value));
      UpdateLEDs = true;
    }
  }
}
void UpdateBrightness(bool ForceUpdate) {
  POT Brightness = BRIGH.ReadStable(PotMinChange, PotStick, 0);
  if (Brightness.Changed or ForceUpdate) {
    if (Brightness.Value < 1) Brightness.Value = 1;
    FastLED.setBrightness(Brightness.Value);
    UpdateLEDs = true;
#ifdef SerialEnabled
    Serial.println("Manual brigh changed to " + String(Brightness.Value) + "(" + String(Brightness.Changed) + ")");
#endif //SerialEnabled
  }
}
bool StartWIFIstuff() {
  //Returns true if WIFI was already started, returns FALSE if it wasn't
  if (WIFI_Started) return true;     //If WIFI already on, stop and return true

  fill_solid(&(LEDs[0]),             TotalLEDs,     CRGB(255, 0, 255)); //turn all LEDs blue  0202
  fill_solid(&(LEDs[0]),             TotalLEDs / 4, CRGB(255, 0, 0  )); //turn 1th quater red 1202
  fill_solid(&(LEDs[TotalLEDs / 2]), TotalLEDs / 4, CRGB(255, 0, 0  )); //turn 2rd quater red 1212
  FastLED.show();                                                       //Update leds to show wifi is starting
  WIFI_Started = WiFiManager_Start();                                   //run the wifi startup (and save results)
  if (WIFI_Started == 1) {
    WiFiManager_StartServer();              //Enable responce to web request
    WiFiManager_EnableSetup(true);          //Enable the setup page, disable for more security
    WIFIconnected = true;
    UpdateLEDs = false;
  }
  return false;
}
void ShowIP() {
  IPAddress MyIp = WiFi.localIP();
#ifdef SerialEnabled
  Serial.print("My IP =" + String(MyIp));
#endif //SerialEnabled
  /* To calculate IP: Amount of LEDs blink +
    if (RED)   then +0
    if (Green) then +60
    if (+Blue) then +120   (Note leds can either turn on 1 or 10 at a time)*/
  CRGB IPColor = CRGB(0, 0, 0);
  if (MyIp[2] < 60) {
    IPColor += CRGB(255, 0, 0);
  } else {
    if (MyIp[2] > 120) {
      MyIp[2] = MyIp[2] - 120;
      IPColor += CRGB(0, 0, 255);
    }
    if (MyIp[2] > 60) {
      MyIp[2] = MyIp[2] - 60;
      IPColor += CRGB(0, 255, 0);
    }
  }
  FastLED.clear();                        //Clear LED strip
  for (int i = 0; i < MyIp[3]; i++) {     //For each IP value left
    MyDelay(500);                         //Wait a bit so we can count it
    if (i + 9 < MyIp[3]) {                //If there is 10+ leds to do, do them in one go
      fill_solid(&(LEDs[i]), 10, IPColor); //Turn them on
      i = i + 9;                          //Adjust the counter (9 since every loop we do +1, meaning next loop is +10)
    } else {
      LEDs[i] = IPColor;                  //Turn it on
    }
    FastLED.show();                       //Update
  }
  MyDelay(500);
}
void MyDelay(int ms) {                    //Just a non-blocking delay
  unsigned long StopAtTime = millis() + ms;
  while (millis() < StopAtTime) {
    OTA_loop();                                         //Do OTA stuff if needed
    if (WIFIconnected) server.handleClient();           //Do WIFI server stuff if needed
    UpdateBrightness(false);
    if (UpdateLEDs) FastLED.show();
    FastLED.delay(1);
  }
}
//==============================
//Convert stuff
//==============================
String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3])  ;
}
String ConvertModeToString(byte IN) {
#ifdef Server_SerialEnabled
  Serial.println("ConvertModeToString '" + String(IN) + "'");
#endif //Server_SerialEnabled
  if (IN < Modes_Amount)
    return ModesString[IN];
  return "OFF";
}
int ConvertModeToInt(String IN) {
#ifdef Server_SerialEnabled
  Serial.println("ConvertModeToInt '" + IN + "'");
#endif //Server_SerialEnabled
  if (StringisDigit(IN)) {
    if (IN.toInt() < Modes_Amount)
      return IN.toInt();
    else
      return 0;
  }
  IN.toUpperCase();
  for (byte i = 0; i < Modes_Amount; i++) {
    if (IN == ModesString[i])
      return i;
  }
  return 0;
}
bool IsTrue(String input) {
  input.toLowerCase();
  if (input.toInt() == 1 or input == "true" or input == "yes" or input == "high")
    return true;
  return false;
}
String IsTrueToString(bool input) {
  if (input)
    return "TRUE";
  return "FALSE";
}
bool StringisDigit(String IN) {
  for (byte i = 0; i < IN.length(); i++) {
    if (not isDigit(IN.charAt(i)))
      return false;
  }
  return true;
}
