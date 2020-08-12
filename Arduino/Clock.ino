/* Written by JelleWho https://github.com/jellewie */
void UpdateAndShowClock(bool ShowClock) {
  //==============================
  //Update the internal time clock
  //==============================
  while (TimeCurrent.Ticks + 1000 <= millis()) {
#ifdef TimeExtra_SerialEnabled
    Serial.println("TM: Time = " + String(TimeCurrent.HH) + ":" + String(TimeCurrent.MM) + ":" + String(TimeCurrent.SS) + " " + String(TimeCurrent.Ticks) + " now=" + String(millis()));
#endif //TimeExtra_SerialEnabled
    TimeCurrent.Ticks += 1000;
    TimeCurrent.SS++;
    if (TimeCurrent.SS >= 60) {
      TimeCurrent.SS = 0;
      TimeCurrent.MM++;
    }
    if (TimeCurrent.MM >= 60) {
      TimeCurrent.MM = 0;
      TimeCurrent.HH++;
      if (ShowClock and DoHourlyAnimation) {
#ifdef Time_SerialEnabled
        Serial.println("TM: Start Hourly Animation");
#endif //Time_SerialEnabled
        StartAnimation(random(0, TotalAnimations), AnimationCounterTime); //Start a random Animation
        ShowClock = false;                       //Do not show the clock, an animation will be shown
      }
    }
    if (TimeCurrent.HH >= 24)
      TimeCurrent.HH = 0;
    static bool TimeFlag = false;
    if (TimeCurrent.HH == 4 and !TimeFlag) {
      TimeFlag = true;
      if (!UpdateTime())                    //Get a new sync timestamp from the server
        WiFiManager_connected = false;
    } else
      TimeFlag = false;
  }
  //==============================
  //Show the time on the LEDs if needed
  //==============================
  if (ShowClock) {
    static byte LastSec = TimeCurrent.SS;     //Store 'second' as an 'update already done' state. so if the seconds counter changes we update and else we skip updating
    if (LastSec != TimeCurrent.SS) {
      LastSec = TimeCurrent.SS;
      FastLED.clear();
      if (ClockHourLines) {
        for (int i = 0; i <= 55; i += 5)                         //Create the (12) hourly stripes
          LEDs[i] += CRGB(ClockHourLines, ClockHourLines, ClockHourLines);
      }
      LEDs[LEDtoPosition(TimeCurrent.SS)] += CRGB(0, 0, 255);
      LEDs[LEDtoPosition(TimeCurrent.MM)] += CRGB(0, 255, 0);
      byte ClockHH = TimeCurrent.HH * 5;
      if (ClockHourAnalog)
        ClockHH += (TimeCurrent.MM / 15);
      LEDs[LEDtoPosition(ClockHH)] += CRGB(255, 0, 0);
      UpdateLEDs = true;
    }
  }
}

byte LEDtoPosition(byte LEDID) {
  //Takes ClockOffset into account, so you can say turn LED 0 on (top of the CLOCK) and it will convert it to be the top LED
  //Basicly adding ClockOffset to the LED and wrapping LEDS around
  LEDID += ClockOffset;
  while (LEDID >= TotalLEDs)
    LEDID -= TotalLEDs;
  return LEDID;
}

bool UpdateTime() {
  if (!WiFiManager_connected) return false;   //If WIFI not connected, stop right away
  fill_solid(&(LEDs[0]),             TotalLEDs,     CRGB(255, 0, 255)); //turn all LEDs Purple  0202
  fill_solid(&(LEDs[0]),             TotalLEDs / 4, CRGB(0, 255, 0  )); //turn 1th quater green 1202
  fill_solid(&(LEDs[TotalLEDs / 2]), TotalLEDs / 4, CRGB(0, 255, 0  )); //turn 2rd quater green 1212
  FastLED.show();                                                       //Update leds to show wifi is starting
#ifdef Time_SerialEnabled
  Serial.println("TM: Get server time");
#endif //Time_SerialEnabled
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
#ifdef Time_SerialEnabled
    Serial.println("TM: Failed to obtain time");
#endif //Time_SerialEnabled
    FastLED.clear();
    FastLED.show();
    return false;
  } else {
#ifdef Time_SerialEnabled
    Serial.println(&timeinfo, "TM: %A, %B %d %Y %H:%M:%S");
#endif //Time_SerialEnabled
    TimeCurrent.Ticks = millis();
    TimeCurrent.HH = timeinfo.tm_hour;
    TimeCurrent.MM = timeinfo.tm_min;
    TimeCurrent.SS = timeinfo.tm_sec;
  }
  TimeSet = true;
  FastLED.clear();
  UpdateLEDs = true;
  return true;
}
