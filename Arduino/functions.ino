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
  //LEDs are devided in 10 sections and then show each char in the arra in RGB 9start counting at 0!)
  //so when the ip is .150 section 1 will be Red, 5=green, and 0=blue
  IPAddress MyIp = WiFi.localIP();
#ifdef SerialEnabled
  Serial.print("My ip = ");
  Serial.println(WiFi.localIP()); //Just send it's IP on boot to let you know
#endif //SerialEnabled

  ShowIPnumber(MyIp[0]);
  UpdateLEDs = true;
  MyDelay(2500);
  ShowIPnumber(MyIp[1]);
  UpdateLEDs = true;
  MyDelay(2500);
  ShowIPnumber(MyIp[2]);
  UpdateLEDs = true;
  MyDelay(2500);
  ShowIPnumber(MyIp[3]);
  UpdateLEDs = true;
}
void ShowIPnumber(byte Number) {
#ifdef SerialEnabled
  Serial.println("ShowIPnumber " + String(Number));
#endif //SerialEnabled
  FastLED.clear();
  const static byte SectionLength = TotalLEDs / 10;
  for (int i = 0; i < TotalLEDs; i += SectionLength) LEDs[i] = CRGB(128, 128, 128);

  byte A = (Number / 100) * SectionLength + 1;
  Number = Number % 100;                              //Modulo (so what is over when we keep deviding by whole 100)
  byte B = (Number / 10) * SectionLength + 1;
  byte C = (Number % 10) * SectionLength + 1;

  for (byte i = 0; i < SectionLength - 1; i++) LEDs[A + i] += CRGB(255, 0, 0);
  for (byte i = 0; i < SectionLength - 1; i++) LEDs[B + i] += CRGB(0, 255, 0);
  for (byte i = 0; i < SectionLength - 1; i++) LEDs[C + i] += CRGB(0, 0, 255);
}
void MyDelay(int ms) {                    //Just a non-blocking delay
  unsigned long StopAtTime = millis() + ms;
  while (millis() < StopAtTime) {
    OTA_loop();                                         //Do OTA stuff if needed
    WiFiManager_RunServer();                            //Do WIFI server stuff if needed
    UpdateBrightness(false);      //Check if manual input potmeters has changed, if so flag the update
    UpdateColor(false);           //Check if manual input potmeters has changed, if so flag the update
    ButtonsA.CheckButton();       //Read buttonstate  (Just trash all inputs)
    if (UpdateLEDs) {
#ifdef UpdateLEDs_SerialEnabled
      Serial.println("Update LEDs");
#endif //UpdateLEDs_SerialEnabled
      UpdateLEDs = false;
      FastLED.show();                         //Update
    }
    yield();
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
