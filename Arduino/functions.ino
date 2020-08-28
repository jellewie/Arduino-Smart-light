/* Written by JelleWho https://github.com/jellewie */
void UpdateColor(bool ForceUpdate) {
  POT R = RED.ReadStable(PotMinChange, PotStick, AverageAmount);
  POT G = GREEN.ReadStable(PotMinChange, PotStick, AverageAmount);
  POT B = BLUE.ReadStable(PotMinChange, PotStick, AverageAmount);
  //  if (R.Value == 0 and G.Value == 0 and B.Value == 0) { //If all LEDs are invisible
  //    R.Value = 1 + constrain(BN, 0, 254);
  //  }     //Tried to turn on LED strip soft red when all colors are invisble.
  if (R.Changed or G.Changed or B.Changed or ForceUpdate) {
    if (Mode == ON or (R.Changed > PotMin or G.Changed > PotMin or B.Changed > PotMin) or ForceUpdate) {
      if (!ForceUpdate and Mode != ON)
        Mode = ON;
      fill_solid(&(LEDs[0]), TotalLEDs, CRGB(R.Value, G.Value, B.Value));
      UpdateLEDs = true;
#ifdef SerialEnabled
      String MSG = "Manual";
      if (ForceUpdate) MSG = "ForceUpdate";
      Serial.println(MSG + " color changed to " +
                     String(R.Value) + "(" + String(R.Changed) + ")," +
                     String(G.Value) + "(" + String(G.Changed) + ")," +
                     String(B.Value) + "(" + String(B.Changed) + ")");
      if (!ForceUpdate) Serial.println(MSG + " mode changed from " + ConvertModeToString(Mode) + " to ON/manual");
#endif //SerialEnabled
    }
  }
}
byte GetAutoBrightness(byte Value) {
  if (Value < 1) Value = 1;
  float Answer = (Value - AutoBrightnessN) * AutoBrightnessP - AutoBrightnessO;
  return 255 - constrain(Answer, 0, 254);
}
void UpdateBrightness(bool ForceUpdate) {
  POT L = LIGHT.ReadStable(PotMinChange, PotStick, AverageAmount);
  if (AutoBrightness) {
    if (L.Changed or ForceUpdate) {
      FastLED.setBrightness(GetAutoBrightness(L.Value));
      UpdateLEDs = true;
#ifdef SerialEnabled
      String MSG = "Automaticly";
      if (ForceUpdate) MSG = "ForceUpdate";
      Serial.println(MSG + " Auto brightness changed to " + String(FastLED.getBrightness()) + " raw=" + String(L.Value) + "");
#endif //SerialEnabled
    }
    ForceUpdate = false;
  }
  POT Brightness = BRIGH.ReadStable(PotMinChange, PotStick, AverageAmount);
  if (Brightness.Changed or ForceUpdate) {
    if (Brightness.Value == 0) Brightness.Value = 1;
    FastLED.setBrightness(Brightness.Value);
    AutoBrightness = false;
    UpdateLEDs = true;
#ifdef SerialEnabled
    String MSG = "Automaticly";
    if (ForceUpdate) MSG = "ForceUpdate";
    Serial.println(MSG + " Manual brightness changed to " + String(FastLED.getBrightness()) + "(" + String(Brightness.Changed) + ")");
#endif //SerialEnabled
  }
}
bool StartWIFIstuff() {
  //Returns TRUE if WIFI was already started, returns FALSE if it wasn't
  if (WiFiManager_connected) return true;                               //If WIFI already on, stop and return true
  fill_solid(&(LEDs[0]),             TotalLEDs,     CRGB(255, 0, 255)); //Turn all LEDs purple 2222
  fill_solid(&(LEDs[0]),             TotalLEDs / 4, CRGB(0,   0, 255)); //Turn 1th quater blue 1222
  fill_solid(&(LEDs[TotalLEDs / 2]), TotalLEDs / 4, CRGB(0,   0, 255)); //Turn 2rd quater blue 1212
  FastLED.show();                                                       //Update leds to show wifi is starting
#ifdef LEDstatus_SerialEnabled
  Serial.println("LS: Setting LEDs to 'wifi is starting'");
#endif //LEDstatus_SerialEnabled
  if (WiFiManager_Start() == 1) {                                       //Run the wifi startup
    WiFiManager_StartServer();                                          //Enable responce to web request
    WiFiManager_EnableSetup(true);                                      //Enable the setup page, disable for more security
    fill_solid(&(LEDs[0]),           TotalLEDs,     CRGB(0, 255, 0));   //Turn all LEDs green
    FastLED.show();                                                     //Update leds to show wifi is done
#ifdef LEDstatus_SerialEnabled
    Serial.println("LS: Setting LEDs to 'wifi is done'");
#endif //LEDstatus_SerialEnabled
  }
  return false;
}
void ShowIP() {
  //The LEDs are divided into 10 sections, and each char in the IP range will be shown one at a time. Left down will be 0 and the next clockwise will be 1.
  //Where RGB will be the order of the numbers, so red will have a *100 multiplication and green will have *10
  //so when the IP is 198.168.1.150 and it shows the last section (150) then section 1 will be Red, 5 will be green and 0 will be blue
  IPAddress MyIp = WiFi.localIP();
#ifdef SerialEnabled
  Serial.print("My ip = ");
  Serial.println(WiFi.localIP()); //Just send it's IP on boot to let you know
#endif //SerialEnabled

  ShowIPnumber(MyIp[0]);
  MyDelay(30000, true);
  ShowIPnumber(MyIp[1]);
  MyDelay(30000, true);
  ShowIPnumber(MyIp[2]);
  MyDelay(30000, true);
  ShowIPnumber(MyIp[3]);
}
void ShowIPnumber(byte Number) {
#ifdef SerialEnabled
  Serial.println("ShowIPnumber " + String(Number));
#endif //SerialEnabled
  FastLED.clear();
  const static byte SectionLength = TotalLEDs / 10;
  for (int i = 0; i < TotalLEDs; i += SectionLength) LEDs[LEDtoPosition(i)] = CRGB(128, 128, 128);

  byte A = (Number / 100) * SectionLength + 1;
  Number = Number % 100;                              //Modulo (so what is over when we keep deviding by whole 100)
  byte B = (Number / 10) * SectionLength + 1;
  byte C = (Number % 10) * SectionLength + 1;

  for (byte i = 0; i < SectionLength - 1; i++) LEDs[LEDtoPosition(A + i)] += CRGB(255, 0, 0);
  for (byte i = 0; i < SectionLength - 1; i++) LEDs[LEDtoPosition(B + i)] += CRGB(0, 255, 0);
  for (byte i = 0; i < SectionLength - 1; i++) LEDs[LEDtoPosition(C + i)] += CRGB(0, 0, 255);
  UpdateLEDs = true;
}
void MyDelay(int ms, bool ReturnOnButtonPress) {                    //Just a non-blocking delay
  unsigned long StopAtTime = millis() + ms;
  while (millis() < StopAtTime) {
    WiFiManager_RunServer();                  //Do WIFI server stuff if needed
    UpdateBrightness(false);                  //Check if manual input potmeters has changed, if so flag the update
    UpdateColor(false);                       //Check if manual input potmeters has changed, if so flag the update
    if (ReturnOnButtonPress) {
      if (ButtonsA.CheckButton().StartPress)  //Read buttonstate and return early when the button is pressed
        return;
    } else
      ButtonsA.CheckButton();                 //Read buttonstate  (Just trash all inputs)
    UpdateLED();
    yield();
    FastLED.delay(1);
  }
}
