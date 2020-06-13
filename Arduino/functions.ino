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

  fill_solid(&(LEDs[0]),             TotalLEDs,     CRGB(0,   0, 255)); //turn all LEDs blue  0202
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
  String Feedback = "OFF";
  switch (IN) {
    case ON:
      Feedback = "ON";
      break;
    case WIFI:
      Feedback = "WIFI";
      break;
    case CLOCK:
      Feedback = "CLOCK";
      break;

    case MOVE:
      Feedback = "MOVE";
      break;
    case FLASH:
      Feedback = "FLASH";
      break;
    case BLINK:
      Feedback = "BLINK";
      break;
    case RAINBOW:
      Feedback = "RAINBOW";
      break;
    case JUGGLE:
      Feedback = "JUGGLE";
      break;
    case SINELON:
      Feedback = "SINELON";
      break;
    case CONFETTI:
      Feedback = "CONFETTI";
      break;
    case GLITTER:
      Feedback = "GLITTER";
      break;
    case BPM:
      Feedback = "BPM";
      break;
  }
  return Feedback;
}
Modes ConvertModeToInt(String IN) {
  Modes Feedback = OFF;
  IN.toUpperCase();
  if (IN == "ON" or IN == String(ON))
    Feedback = ON;
  else if (IN == "RAINBOW" or IN == String(RAINBOW))
    Feedback = RAINBOW;
  else if (IN == "WIFI" or IN == String(WIFI))
    Feedback = WIFI;
  else if (IN == "RESET" or IN == String(RESET))
    Feedback = RESET;
  else if (IN == "CLOCK" or IN == String(CLOCK))
    Feedback = CLOCK;

  else if (IN == "MOVE" or IN == String(MOVE))
    Feedback = MOVE;
  else if (IN == "FLASH" or IN == String(FLASH))
    Feedback = FLASH;
  else if (IN == "BLINK" or IN == String(BLINK))
    Feedback = BLINK;
  else if (IN == "RAINBOW" or IN == String(RAINBOW))
    Feedback = RAINBOW;
  else if (IN == "JUGGLE" or IN == String(JUGGLE))
    Feedback = JUGGLE;
  else if (IN == "SINELON" or IN == String(SINELON))
    Feedback = SINELON;
  else if (IN == "CONFETTI" or IN == String(CONFETTI))
    Feedback = CONFETTI;
  else if (IN == "GLITTER" or IN == String(GLITTER))
    Feedback = GLITTER;
  else if (IN == "BPM" or IN == String(BPM))
    Feedback = BPM;
  return Feedback;
}
bool IsTrue(String input) {
  input.toLowerCase();
  if (input.toInt() == 1 or input == "true" or input == "yes")
    return true;
  return false;
}
String IsTrueToString(bool input) {
  if (input)
    return "TRUE";
  return "FALSE";
}
