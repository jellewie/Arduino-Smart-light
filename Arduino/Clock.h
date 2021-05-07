/* Written by JelleWho https://github.com/jellewie */
#define Clock_ConnectionTimeOutMS 10000

void ClearAndSetupClock() {
  FastLED.clear();
  if (ClockHourLines) {
    for (int i = 0; i <= 55; i += 5)                            //Create the (12) hourly stripes
      LED_Add(LEDtoPosition(i * LEDSections), LEDSections, CRGB(ClockHourLines, ClockHourLines, ClockHourLines));
  }
}
void UpdateAndShowClock(bool ShowClock, bool ForceClock = false);
void UpdateAndShowClock(bool ShowClock, bool ForceClock) {
  //ShowClock true = show the clock if needed
  //& ForceClock true = force the clock to be shown
  //==============================
  //Update the internal time clock
  //==============================
  static bool FirstUpdate = true;                               //This is just to run the first time. Mostly needed if the boot time is lower than 1 second, since we would otherwise skip updating the time until 1s has passed
#ifdef Time_SerialEnabled
  if (FirstUpdate) Serial.println("TM: UpdateAndShowClock due to FirstUpdate");
#endif //Time_SerialEnabled
  while (TimeCurrent.Ticks + 1000 <= millis() or FirstUpdate) { //While more than 1 second pased, or its the first update
    if (FirstUpdate) {
      FirstUpdate = false;
      UpdateTime();                                             //Get a new sync timestamp from the server
    } else {
      TimeCurrent.Ticks += 1000;
    }
#ifdef TimeExtra_SerialEnabled
    Serial.println("TME: Time = " + String(TimeCurrent.HH) + ":" + String(TimeCurrent.MM) + ":" + String(TimeCurrent.SS) + " " + String(TimeCurrent.Ticks) + " now=" + String(millis()));
#endif //TimeExtra_SerialEnabled
    TimeCurrent.SS++;
    if (TimeCurrent.SS >= 60) {
      TimeCurrent.SS = 0;
      TimeCurrent.MM++;
    }
    if (TimeCurrent.MM >= 60) {
      TimeCurrent.MM = 0;
      TimeCurrent.HH++;
      if (ShowClock and HourlyAnimationS > 0) {
#ifdef Time_SerialEnabled
        Serial.println("TM: Start Hourly Animation");
#endif //Time_SerialEnabled
        StartAnimation(random(0, TotalAnimations), HourlyAnimationS);//Start a random Animation
        ShowClock = false;                                      //Do not show the clock, an animation will be shown
      }
    }
    if (TimeCurrent.HH >= 24)
      TimeCurrent.HH = 0;
  }
  if (WiFi.status() != WL_CONNECTED) {                          //If we are no longer connected to WIFI
    EVERY_N_MILLISECONDS(2000) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));     //Blink every 2 second to show we have lost WIFI and can not sync
    }
  }
  //==============================
  //Show the time on the LEDs if needed
  //==============================
  if (ShowClock) {
    if (ClockAnalog) {
      ClearAndSetupClock();

      float TimeSSfactor = (millis() - TimeCurrent.Ticks) / 1000.0 + TimeCurrent.SS;
      float TimeMMfactor = TimeCurrent.MM + TimeCurrent.SS / 60.0;
      float TimeHHfactor = TimeCurrent.HH * 5 + TimeCurrent.MM / 60.0;
      if (TimeHHfactor > 60) TimeHHfactor -= 60;                //If its above 12 hours, remove 12 since we can only show 0-12 hours

      byte TimeSSoffset = 0, TimeMMoffset = 0, TimeHHoffset = 0;//If time is over a half circle, lower it, and set the offset
      if (TimeSSfactor > 30) TimeSSfactor -= TimeSSoffset = 30; //^
      if (TimeMMfactor > 30) TimeMMfactor -= TimeMMoffset = 30; //^
      if (TimeHHfactor > 30) TimeHHfactor -= TimeHHoffset = 30; //^

      for (int i = 0; i < TotalLEDs; i++) {
        byte TimeSSbr = exp(-2.7 * sq(i - TimeSSfactor)) * 255; //https://www.desmos.com/calculator/zkl6idhjvx
        byte TimeMMbr = exp(-2.7 * sq(i - TimeMMfactor)) * 255; //^
        byte TimeHHbr = exp(-2.7 * sq(i - TimeHHfactor)) * 255; //^
        LED_Add(LEDtoPosition((i + TimeSSoffset) * LEDSections), LEDSections, CRGB(0, 0, TimeSSbr));
        LED_Add(LEDtoPosition((i + TimeMMoffset) * LEDSections), LEDSections, CRGB(0, TimeMMbr, 0));
        LED_Add(LEDtoPosition((i + TimeHHoffset) * LEDSections), LEDSections, CRGB(TimeHHbr, 0, 0));
        UpdateLEDs = true;
      }
    } else {
      static byte LastSec = -1;                                 //Store 'second' as an 'update already done' state. so if the seconds counter changes we update and else we skip updating
      if (LastSec != TimeCurrent.SS or ForceClock) {
        LastSec = TimeCurrent.SS;
        ClearAndSetupClock();
        LED_Add(LEDtoPosition(TimeCurrent.SS * LEDSections), LEDSections, CRGB(0, 0, 255));
        LED_Add(LEDtoPosition(TimeCurrent.MM * LEDSections), LEDSections, CRGB(0, 255, 0));
        byte ClockHH = TimeCurrent.HH * 5;
        if (ClockHourAnalog)
          ClockHH += (TimeCurrent.MM / 12);
        LED_Add(LEDtoPosition(ClockHH * LEDSections), LEDSections, CRGB(255, 0, 0));
        UpdateLEDs = true;
      }
    }
  }
}
bool UpdateTime() {
  if (!WiFiManager.CheckAndReconnectIfNeeded(false)) return false;      //If WIFI is not connected, stop right away
  fill_solid(&(LEDs[0]),             TotalLEDs,     CRGB(255, 0, 255)); //Turn all LEDs Purple  0202
  fill_solid(&(LEDs[0]),             TotalLEDs / 4, CRGB(0, 255, 0  )); //Turn 1th quater green 1202
  fill_solid(&(LEDs[TotalLEDs / 2]), TotalLEDs / 4, CRGB(0, 255, 0  )); //Turn 2rd quater green 1212
  FastLED.show();                                                       //Update leds to show updating time
  FastLED.clear();
  UpdateLEDs = true;
#ifdef LEDstatus_SerialEnabled
  Serial.println("LS: Setting LEDs to 'updating time'");
#endif //LEDstatus_SerialEnabled
#ifdef Time_SerialEnabled
  Serial.println("TM: Get server time");
#endif //Time_SerialEnabled
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;                                           //https://www.cplusplus.com/reference/ctime/tm/
  if (!getLocalTime(&timeinfo, Clock_ConnectionTimeOutMS)) {
#ifdef Time_SerialEnabled
    Serial.println("TM: Failed to obtain time");
#endif //Time_SerialEnabled
    LastMode = -1;                                              //Re-init the mode
    return false;
  }
  time_t Temptimeinfo = mktime(&timeinfo);
  struct tm *timeinfoLocal;
  timeinfoLocal = localtime(&Temptimeinfo);                     //Convert UTC time to local time to exclude include DST offset automatically
#ifdef Time_SerialEnabled
  Serial.println(&timeinfo, "TM: UTC: %A, %B %d %Y %H:%M:%S");
  Serial.println("TM: LOCAL: " + String(timeinfoLocal->tm_hour) + ":" + String(timeinfoLocal->tm_min) + ":" + String(timeinfoLocal->tm_sec));
#endif //Time_SerialEnabled
  TimeCurrent.Ticks = millis();
  TimeCurrent.HH = timeinfoLocal->tm_hour;
  TimeCurrent.MM = timeinfoLocal->tm_min;
  TimeCurrent.SS = timeinfoLocal->tm_sec;
  TimeSet = true;
  if (Mode == CLOCK) UpdateAndShowClock(true, true);            //If we are curently in CLOCK mode, make sure to update the shown time
  LastMode = -1;                                                //Re-init the mode
  return true;
}
