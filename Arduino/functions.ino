/* Written by JelleWho https://github.com/jellewie
  This file declairs the function types but does require some other file-functions and variables to work
*/
void UpdateColor(bool ForceUpdate) {
  if (digitalRead(PAI_DisablePOTs) == LOW) return;    //If the POTs are disabled with hardware
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
#ifdef RGBL_SerialEnabled
      String MSG = "Manual";
      if (ForceUpdate) MSG = "ForceUpdate";
      Serial.println("RG: " + MSG + " color changed to " +
                     String(R.Value) + "(" + String(R.Changed) + ")," +
                     String(G.Value) + "(" + String(G.Changed) + ")," +
                     String(B.Value) + "(" + String(B.Changed) + ")");
      if (!ForceUpdate) Serial.println("RG: " + MSG + " mode changed from " + ConvertModeToString(Mode) + " to ON/manual");
#endif //RGBL_SerialEnabled
    }
  }
}
byte GetAutoBrightness(byte Value) {
  float Answer = AutoBrightnessP * (Value - AutoBrightnessN) - AutoBrightnessO;
  return 255 - constrain(Answer, 0, 254);
}
void UpdateBrightness(bool ForceUpdate) {
  POT L = LIGHT.ReadStable(PotMinChange, PotStick, AverageAmount);
  if (AutoBrightness) {
    if (L.Changed or ForceUpdate) {
      FastLED.setBrightness(GetAutoBrightness(L.Value));
      UpdateLEDs = true;
#ifdef RGBL_SerialEnabled
      String MSG = "Automaticly";
      if (ForceUpdate) MSG = "ForceUpdate";
      Serial.println("RG: " + MSG + " Auto brightness changed to " + String(FastLED.getBrightness()) + " raw=" + String(L.Value) + "");
#endif //RGBL_SerialEnabled
    }
    ForceUpdate = false;
  }
  if (digitalRead(PAI_DisablePOTs) == LOW) return; //If the POTs are disabled with hardware
  POT Brightness = BRIGH.ReadStable(PotMinChange, PotStick, AverageAmount);
  if (Brightness.Changed or ForceUpdate) {
    if (Brightness.Value == 0) Brightness.Value = 1;
    FastLED.setBrightness(Brightness.Value);
    AutoBrightness = false;
    UpdateLEDs = true;
#ifdef SerialEnabled
    String MSG = "Automaticly";
    if (ForceUpdate) MSG = "ForceUpdate";
    Serial.println("RG: " + MSG + " Manual brightness changed to " + String(FastLED.getBrightness()) + "(" + String(Brightness.Changed) + ")");
#endif //SerialEnabled
  }
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
  MyDelay(30000, 500, true);
  ShowIPnumber(MyIp[1]);
  MyDelay(30000, 500, true);
  ShowIPnumber(MyIp[2]);
  MyDelay(30000, 500, true);
  ShowIPnumber(MyIp[3]);
}
void ShowIPnumber(byte Number) {
  //192 == ABC == A=1,C=9,C=2
#ifdef SerialEnabled
  Serial.println("ShowIPnumber " + String(Number));
#endif //SerialEnabled
  FastLED.clear();
  const static byte SectionLength = TotalLEDs / 10;
  for (int i = 0; i < TotalLEDs; i += SectionLength) LEDs[LEDtoPosition(i)] = CRGB(128, 128, 128);  //Add section spacers

  LED_Fill(LEDtoPosition(-1), 3, CRGB(255, 255, 255));  //Mark the start by painting in 3 LEDs around it
  
  byte A = (Number / 100) * SectionLength + 1;
  Number = Number % 100;                              //Modulo (so what is over when we keep deviding by whole 100)
  byte B = (Number / 10) * SectionLength + 1;
  byte C = (Number % 10) * SectionLength + 1;

  for (byte i = 0; i < SectionLength - 1; i++) LEDs[LEDtoPosition(A + i)] += CRGB(255, 0, 0); //Make section A Red
  for (byte i = 0; i < SectionLength - 1; i++) LEDs[LEDtoPosition(B + i)] += CRGB(0, 255, 0); //Make section B Green
  for (byte i = 0; i < SectionLength - 1; i++) LEDs[LEDtoPosition(C + i)] += CRGB(0, 0, 255); //Make section C Blue
  UpdateLEDs = true;
}
void MyDelay(int DelayMS, int MinDelayMS, bool ReturnOnButtonPress) { //Just a non-blocking delay
  //DelayMS, delay in ms like in the Arduino Delay() function
  //(ReturnOnButtonPress=true) MinDelayMS, min delay in ms before returning on buttonpress
  unsigned long _StartTime = millis();
  while (millis() < _StartTime + DelayMS) {
    WiFiManager.RunServer();                          //Do WIFI server stuff if needed
    UpdateBrightness(false);                          //Check if manual input potmeters has changed, if so flag the update
    UpdateColor(false);                               //Check if manual input potmeters has changed, if so flag the update
    Button_Time Value = ButtonsA.CheckButton();       //Read and save buttonstate (only used if needed, else there just trashed)
    if (ReturnOnButtonPress) {
      if (millis() > _StartTime + MinDelayMS) {       //*When MinDelayMS=0 then the current time is always later than the StartTime, no need for another check for that
        if (Value.StartPress)                         //Read buttonstate and return early when the button is pressed
          return;
      }
    }
    UpdateLED();
    yield();
    FastLED.delay(1);
  }
}
