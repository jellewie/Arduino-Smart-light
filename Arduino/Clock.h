/* Written by JelleWho https://github.com/jellewie */
#define Clock_ConnectionTimeOutMS 10000

void ClearAndSetupClock() {
  ClockClear();
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
        StartAnimation(random8(0, TotalAnimations), HourlyAnimationS); //Start a random Animation
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

      float LED_SS = TimeCurrent.SS + (millis() - TimeCurrent.Ticks) / 1000.0;
      float LED_MM = TimeCurrent.MM + TimeCurrent.SS / 60.0;
      float LED_HH = TimeCurrent.HH * 5 + TimeCurrent.MM / 12.0;
      if (LED_HH >= 60) LED_HH -= 60;                           //If its above 12 hours, remove 12 since we can only show 0-12 hours

      //When there are more sections, raise the resolution (but sadly I could not figure out a formula so this is only limited to a few hand checked samples)
#if (LEDSections >= 4 and LEDSections % 4 == 0)                 //If its 4 or a multiplication of it (% =modulo)
      const float Multiplier = -0.16875;
      LED_SS = LED_SS * 4;
      LED_MM = LED_MM * 4;
      LED_HH = LED_HH * 4;
#elif (LEDSections >= 3 and LEDSections % 3 == 0)               //If its 3 or a multiplication of it (% =modulo)
      const float Multiplier = -0.3;
      LED_SS = LED_SS * 3;
      LED_MM = LED_MM * 3;
      LED_HH = LED_HH * 3;
#elif (LEDSections == 2)
      const float Multiplier = -0.675;
      LED_SS = LED_SS * LEDSections;
      LED_MM = LED_MM * LEDSections;
      LED_HH = LED_HH * LEDSections;
#else
      const float Multiplier = -2.7;
#endif //LEDSections

      byte Offset_SS = 0, Offset_MM = 0, Offset_HH = 0;       //If time is over a half circle, lower it, and set the offset. This make it so the EXP forulla loops around
      if (LED_SS > TotalLEDsClock / 2) LED_SS -= Offset_SS = TotalLEDsClock / 2;          //^
      if (LED_MM > TotalLEDsClock / 2) LED_MM -= Offset_MM = TotalLEDsClock / 2;          //^
      if (LED_HH > TotalLEDsClock / 2) LED_HH -= Offset_HH = TotalLEDsClock / 2;          //^
      for (int i = 0; i < TotalLEDsClock; i++) {
        byte Bri_SS = exp(Multiplier * sq(i - LED_SS)) * 255;         //https://www.desmos.com/calculator/zkl6idhjvx
        byte Bri_MM = exp(Multiplier * sq(i - LED_MM)) * 255;         //^
        byte Bri_HH = exp(Multiplier * sq(i - LED_HH)) * 255;         //^
        LED_Add(LEDtoPosition(i + Offset_SS), 1, CRGB(0, 0, Bri_SS), TotalLEDsClock);
        LED_Add(LEDtoPosition(i + Offset_MM), 1, CRGB(0, Bri_MM, 0), TotalLEDsClock);
        LED_Add(LEDtoPosition(i + Offset_HH), 1, CRGB(Bri_HH, 0, 0), TotalLEDsClock);
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
  if (!WiFiManager.CheckAndReconnectIfNeeded(false)) return false; //If WIFI is not connected, stop right away
  if (Mode != OFF) {
#ifdef LEDstatus_SerialEnabled
    Serial.println("LS: Setting LEDs to 'updating time'");
#endif //LEDstatus_SerialEnabled
    LED_Fill(LEDtoPosition(0),                  TotalLEDsClock,     CRGB(255, 0, 255), TotalLEDsClock); //Turn all LEDs purple  2222
    LED_Fill(LEDtoPosition(0),                  TotalLEDsClock / 4, CRGB(0  , 255, 0), TotalLEDsClock); //Turn 1th quater green 1222
    LED_Fill(LEDtoPosition(TotalLEDsClock / 2), TotalLEDsClock / 4, CRGB(0  , 255, 0), TotalLEDsClock); //Turn 2rd quater green 1212
    FastLED.show();                                             //Update leds to show updating time
    ClockClear();                                               //Clear and mark that when done update the LEDs again
    UpdateLEDs = true;
  }
#ifdef Time_SerialEnabled
  Serial.println("TM: Get server time");
#endif //Time_SerialEnabled
  configTime(0, 0, ntpServer);
  setenv("TZ", timeZone.c_str(), 1);   //https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
  tzset();

  if (!getLocalTime(&timeinfo, Clock_ConnectionTimeOutMS)) {
#ifdef Time_SerialEnabled
    Serial.println("TM: Failed to obtain time");
#endif //Time_SerialEnabled
    LastMode = -1;                                              //Re-init the mode
    return false;
  }

#ifdef Time_SerialEnabled
  Serial.println(&timeinfo, "TM: UTC: %A, %B %d %Y %H:%M:%S");
#endif //Time_SerialEnabled
  TimeCurrent.Ticks = millis();
  TimeCurrent.HH = timeinfo.tm_hour;
  TimeCurrent.MM = timeinfo.tm_min;
  TimeCurrent.SS = timeinfo.tm_sec;
  TimeSet = true;
  if (Mode == CLOCK) UpdateAndShowClock(true, true);            //If we are curently in CLOCK mode, make sure to update the shown time
  LastMode = -1;                                                //Re-init the mode
  return true;
}
