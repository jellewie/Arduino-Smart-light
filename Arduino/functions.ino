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
  if (WiFiManager_connected) return true;     //If WIFI already on, stop and return true

  fill_solid(&(LEDs[0]),             TotalLEDs,     CRGB(255, 0, 255)); //turn all LEDs blue  0202
  fill_solid(&(LEDs[0]),             TotalLEDs / 4, CRGB(255, 0, 0  )); //turn 1th quater red 1202
  fill_solid(&(LEDs[TotalLEDs / 2]), TotalLEDs / 4, CRGB(255, 0, 0  )); //turn 2rd quater red 1212
  FastLED.show();                                                       //Update leds to show wifi is starting
  if (WiFiManager_Start() == 1) {                                       //run the wifi startup (and save results)
    WiFiManager_StartServer();                //Enable responce to web request
    WiFiManager_EnableSetup(true);            //Enable the setup page, disable for more security
    UpdateLEDs = false;
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
  MyDelay(2500);
  ShowIPnumber(MyIp[1]);
  MyDelay(2500);
  ShowIPnumber(MyIp[2]);
  MyDelay(2500);
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
void MyDelay(int ms) {                    //Just a non-blocking delay
  unsigned long StopAtTime = millis() + ms;
  while (millis() < StopAtTime) {
    OTA_loop();                                         //Do OTA stuff if needed
    WiFiManager_RunServer();                            //Do WIFI server stuff if needed
    UpdateBrightness(false);      //Check if manual input potmeters has changed, if so flag the update
    UpdateColor(false);           //Check if manual input potmeters has changed, if so flag the update
    ButtonsA.CheckButton();       //Read buttonstate  (Just trash all inputs)
    WiFiManager_RunServer();                  //Do WIFI server stuff if needed
    UpdateBrightness(false);                  //Check if manual input potmeters has changed, if so flag the update
    UpdateColor(false);                       //Check if manual input potmeters has changed, if so flag the update
    if (TrashButtons) ButtonsA.CheckButton(); //Read buttonstate  (Just trash all inputs)
    UpdateLEDs();
    yield();
    FastLED.delay(1);
  }
}
