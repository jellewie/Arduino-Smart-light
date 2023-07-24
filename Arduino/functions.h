/* Written by JelleWho https://github.com/jellewie
  This file declairs the function types but does require some other file-functions and variables to work
*/
int LEDtoPosition(signed int LEDID) {
  //Takes LEDOffset into account, so you can say turn LED 0 on (top of the CLOCK) and it will convert it to be the top LED
  //Basicly adding LEDOffset to the LED and wrapping LEDS around

  //x-30                  = Rotates 180 degrees
  //(TotalLEDs-Amount)-x  = Flip in Y-Y axes

  LEDID += LEDOffset;
  while (LEDID < 0)                                             //When a negative overflow
    LEDID += TotalLEDsClock;
  while (LEDID >= TotalLEDsClock)                               //When a positive overflow
    LEDID -= TotalLEDsClock;
  return LEDID;
}
//==================================================
//Basic universal LED functions. These includes start postion, amount (inc overflow correction and such)
//==================================================
void LED_Fill(int From, int Amount, CRGB Color, int MaxBound = TotalLEDs);
void LED_Fill(int From, int Amount, CRGB Color, int MaxBound) {
  while (From >= MaxBound) From -= MaxBound;                    //(Protection out of array bounds) Loop the LEDs around (TotalLEDs+1 is the same as LED 1)
  if (Amount > MaxBound) Amount = MaxBound;                     //(Protection out of array bounds) if more LEDs are given than there are in the array, set the amount to all LEDs
  if (From + Amount > MaxBound) {                               //Overflow protection
    byte calc1 = MaxBound - From;                               //Calculates the amount of LEDs which need to on on the end of the strip
    fill_solid(&(LEDs[From]), calc1, Color);
    fill_solid(&(LEDs[0]), Amount - calc1, Color);
  } else
    fill_solid(&(LEDs[From]), Amount, Color);
}
void LED_Add(int From, int Amount, CRGB Color, int MaxBound = TotalLEDs);
void LED_Add(int From, int Amount, CRGB Color, int MaxBound) {
  while (From >= MaxBound) From -= MaxBound;                    //(Protection out of array bounds) Loop the LEDs around (TotalLEDs+1 is the same as LED 1)
  if (Amount > MaxBound) Amount = MaxBound;                     //(Protection out of array bounds) if more LEDs are given than there are in the array, set the amount to all LEDs
  if (From + Amount > MaxBound) {                               //Overflow protection
    byte calc1 = MaxBound - From;                               //Calculates the amount of LEDs which need to on on the end of the strip
    for (int i = From; i < From + calc1; i++)
      LEDs[i] += Color;
    for (int i = 0; i < Amount - calc1; i++)
      LEDs[i] += Color;
  } else
    for (int i = From; i < From + Amount; i++)
      LEDs[i] += Color;
}
void LED_Move(int From, int Amount, CRGB Color, byte Sets, byte Length, byte *Counter, bool Reverse = false, bool Reset = true, int MaxBound = TotalLEDs);
void LED_Move(int From, int Amount, CRGB Color, byte Sets, byte Length, byte *Counter, bool Reverse, bool Reset, int MaxBound) {
  //From = The first LED to do the animation on
  //Amount = The amount of LEDS to do the animation on
  //Color = the RGB value to use
  //Sets = The amount of sets going around
  //Length = How long each set would be
  //Counter = a pointer to a counter where we keep track of how far we are
  //Reverse = Reverse the animation direction
  //Reset = reset the LED color with every call (do not use if using overlapping moves)
  //Example:
  //  static byte counter;
  //  Move(5, 10, CRGB(0, 1, 0), 2, 2, &counter);

  byte Count = *Counter;
  if (Reverse)
    Count = Amount - Count - 1;
  if (Reset)
    LED_Fill(From, Amount, CRGB(0, 0, 0), MaxBound);
  byte poss[Sets];                                              //Array for saving the positions of the sections
  for (byte i = 0; i < Sets; i++) {                             //Beginning of the loop which will send each position and length
    poss[i] = Count + Amount * i / Sets;                        //This will calculate each position by adding the offset times the position number to the first position
    byte posX;                                                  //This is the variable which will be used for sending position start. (this can overflow above TotalLEDs, but this will be fixed by the Fill command)
    if (poss[i] >= Amount) {                                    //To see if the position is to bigger than the total amount
      posX = From + poss[i] - Amount;                           //Subtract the total amount of LEDs from the position number
    } else {                                                    //Otherwise it will just use the position data without modifying it
      posX = From + poss[i];                                    //Just use the position number
    }
    if (posX <= From + Amount - Length) {                       //If the whole section ends before the total amount is reached it will just us the normal way of setting the LEDs
      LED_Fill(posX, Length, Color, MaxBound);                  //With the standard fill solid command from FastLED, LEDs[posX] PosX stands for beginning position, Amount will stand for the size of the sections and the last one is the color
    } else {
      byte calc1 = (From + Amount) - posX;                      //Calculates the amount of LEDs which need to be set from the beginning
      LED_Fill(posX, calc1, Color, MaxBound);                   //Fills the LEDs at the end of the strip
      LED_Fill(From, Length - calc1, Color, MaxBound);          //Fills the LEDs at the beginning of the strip
    }
  }
  *Counter += 1;
  if (*Counter >= Amount)
    *Counter = *Counter - Amount;
}
bool LED_Flash(int From, int Amount, CRGB Color, CRGB Color2 = CRGB(0, 0, 0), int MaxBound = TotalLEDs);
bool LED_Flash(int From, int Amount, CRGB Color, CRGB Color2, int MaxBound) {
  if (LEDs[From] != Color) {
    LED_Fill(From, Amount, Color, MaxBound);
    return true;
  }
  LED_Fill(From, Amount, Color2, MaxBound);
  return false;
}
void LED_Rainbow(int From, int Amount, byte DeltaHue, int MaxBound = TotalLEDs);
void LED_Rainbow(int From, int Amount, byte DeltaHue, int MaxBound) {
  //byte DeltaHue = Diffrence between each LED in hue
  static byte gHue;
  gHue++;
  while (From >= MaxBound) From -= MaxBound;                    //(Protection out of array bounds) Loop the LEDs around (TotalLEDs+1 is the same as LED 1)
  if (Amount > MaxBound) Amount = MaxBound;                     //(Protection out of array bounds) if more LEDs are given than there are in the array, set the amount to all LEDs
  if (From + Amount > MaxBound) {                               //Overflow protection
    byte calc1 = MaxBound - From;                               //Calculates the amount of LEDs which need to on on the end of the strip
    fill_rainbow(&(LEDs[From]), calc1, gHue, DeltaHue);
    fill_rainbow(&(LEDs[0]), Amount - calc1, gHue, DeltaHue);
  } else
    fill_rainbow(&LEDs[From], Amount, gHue, DeltaHue);
}
void LED_Wobble(int From, int Amount, CRGB Color, byte Sets, byte Length, int MaxBound = TotalLEDs);
void LED_Wobble(int From, int Amount, CRGB Color, byte Sets, byte Length, int MaxBound) {
  //Sort of a move, but just back and forth between the start en end
  static byte Counter;                                          //this function can only be called once, this 'Counter' is a 1 time counter (could not get the pointer working to attach it to the caller)
  static bool Reverse = false;
  LED_Move(From, Amount, Color, Sets, Length, &Counter, Reverse, MaxBound);
  if (Reverse) {
    if (Counter == Amount - 1) {
      Reverse = false;
      Counter = 0;
    }
  } else {
    if (Counter == Amount - Length) {
      Reverse = true;
      Counter = Length - 1;
    }
  }
}
void LED_Blink(int From, int Amount, CRGB rgb, byte AlwaysOn, byte * Counter, bool Reverse = false, bool Reset = true, int MaxBound = TotalLEDs);
void LED_Blink(int From, int Amount, CRGB rgb, byte AlwaysOn, byte * Counter, bool Reverse, bool Reset, int MaxBound) {
  if (Reset)
    LED_Fill(From, Amount, CRGB(0, 0, 0), MaxBound);            //Turn LEDs off
  if (Reverse) {
    LED_Fill(From + Amount - AlwaysOn, AlwaysOn, rgb, MaxBound); //Set some LEDs to be always on
    LED_Fill(From + Amount - *Counter, *Counter, rgb, MaxBound); //Set the counter amount of LEDs on (this will increase)
  } else {
    LED_Fill(From, AlwaysOn, rgb, MaxBound);                    //Set some LEDs to be always on
    LED_Fill(From, *Counter, rgb, MaxBound);                    //Set the counter amount of LEDs on (this will increase)
  }
  *Counter += 1;                                                //This will make the blink 1 longer each time
  if (*Counter > Amount)                                        //If we are at max length
    *Counter = 0;                                               //Reset counter
}
void LED_BackAndForth(int From, int Amount, CRGB Color, byte *Counter, bool *Direcion, bool Reverse = false, bool Reset = true, int MaxBound = TotalLEDs);
void LED_BackAndForth(int From, int Amount, CRGB Color, byte *Counter, bool *Direcion, bool Reverse, bool Reset, int MaxBound) {
  //Fills then emties the range of leds one by one
  if (Reset)
    LED_Fill(From, Amount, CRGB(0, 0, 0), MaxBound);

  if (*Direcion)
    *Counter -= 1;                                              //This will make the animation_on 1 shorter each time
  else
    *Counter += 1;                                              //This will make the animation_on 1 longer each time
  if (*Counter >= Amount or * Counter == 0)                     //If we are at max length or at the start
    *Direcion = !*Direcion;                                     //Flip direction

  if (Reverse)
    LED_Fill(From + Amount - *Counter, *Counter, Color, MaxBound); //Set the counter amount of LEDs on
  else
    LED_Fill(From, *Counter, Color, MaxBound);                  //Set the counter amount of LEDs on
}
//==================================================
void ClockClear() {
  fill_solid(&(LEDs[0]), TotalLEDsClock, CRGB(0, 0, 0));        //Clear all the Clock LEDs
}
void UpdateColor(bool ForceUpdate) {
  if (digitalRead(PAI_DisablePOTs) == LOW) return;              //If the POTs are disabled with hardware
  POT R = RED.ReadStable(PotMinChange, PotStick, StableAnalog_AverageAmount);
  POT G = GREEN.ReadStable(PotMinChange, PotStick, StableAnalog_AverageAmount);
  POT B = BLUE.ReadStable(PotMinChange, PotStick, StableAnalog_AverageAmount);
  //  if (R.Value == 0 and G.Value == 0 and B.Value == 0) {     //If all LEDs are invisible
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
  Answer = round(Answer);                                       //Round to nearest instead of down
  return 255 - constrain(Answer, 0, 254);
}
void UpdateAudio(bool ForceUpdate) {
  if (digitalRead(PAI_DisablePOTs) == LOW) return;              //If the POTs are disabled with hardware
  if (!AudioLink) return;                                       //If AudioLink is disabled, dont continue
  POT D = AUDIO.ReadStable(PotMinChange, PotStick, StableAnalog_AverageAmount);
  if (D.Changed or ForceUpdate) {
    FastLED.setBrightness(GetAutoBrightness(D.Value));
    UpdateLEDs = true;
  }
}
void UpdateBrightness(bool ForceUpdate) {
  POT L = LIGHT.ReadStable(PotMinChange, PotStick, StableAnalog_AverageAmount);
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
  if (digitalRead(PAI_DisablePOTs) == LOW) return;              //If the POTs are disabled with hardware
  POT Brightness = BRIGH.ReadStable(PotMinChange, PotStick, StableAnalog_AverageAmount);
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
void ShowIPnumber(byte Number) {
  //192 == ABC == A=1,C=9,C=2* (the real value of A is the start position of that section, the section is going to represent the digit)
#ifdef SerialEnabled
  Serial.println("ShowIPnumber " + String(Number));
#endif //SerialEnabled
  FastLED.clear();
  const static byte SectionLength = TotalLEDsClock / 10;
  for (int i = 0; i < TotalLEDsClock; i += SectionLength)
    LED_Fill(LEDtoPosition(i), LEDSections, CRGB(128, 128, 128), TotalLEDsClock); //Add section spacers
  LED_Fill (LEDtoPosition(0 - LEDSections), LEDSections + 2, CRGB(255, 255, 255), TotalLEDsClock); //Mark the start by painting in 3 LEDs around it

  byte A = (Number / 100) * SectionLength + LEDSections;        //Set start position of the first digit
  Number = Number % 100;                                        //Modulo (so what is over when we keep deviding by whole 100)
  byte B = (Number / 10) * SectionLength + LEDSections;
  byte C = (Number % 10) * SectionLength + LEDSections;

  LED_Add(LEDtoPosition(A), SectionLength - LEDSections, CRGB(255, 0, 0), TotalLEDsClock); //Make section A Red
  LED_Add(LEDtoPosition(B), SectionLength - LEDSections, CRGB(0, 255, 0), TotalLEDsClock); //Make section B Green
  LED_Add(LEDtoPosition(C), SectionLength - LEDSections, CRGB(0, 0, 255), TotalLEDsClock); //Make section C Blue

  UpdateLEDs = true;
}
void UpdateLED() {
  if (UpdateLEDs) {
#ifdef UpdateLEDs_SerialEnabled
    Serial.println("UL: Update LEDs");
#endif //UpdateLEDs_SerialEnabled
    UpdateLEDs = false;
    FastLED.show();                                             //Update
  }
}
void MyDelay(int DelayMS, int MinDelayMS, bool ReturnOnButtonPress) { //Just a non-blocking delay
  //DelayMS, delay in ms like in the Arduino Delay() function
  //(ReturnOnButtonPress=true) MinDelayMS, min delay in ms before returning on buttonpress
  unsigned long _StartTime = millis();
  while (millis() < _StartTime + DelayMS) {
    WiFiManager.RunServer();                                    //Do WIFI server stuff if needed
    UpdateBrightness(false);                                    //Check if manual input potmeters has changed, if so flag the update
    UpdateColor(false);                                         //Check if manual input potmeters has changed, if so flag the update
    Button_Time Value = ButtonsA.CheckButton();                 //Read and save buttonstate (only used if needed, else there just trashed)
    if (ReturnOnButtonPress) {
      if (millis() > _StartTime + MinDelayMS) {                 //*When MinDelayMS=0 then the current time is always later than the StartTime, no need for another check for that
        if (Value.StartPress)                                   //Read buttonstate and return early when the button is pressed
          return;
      }
    }
    UpdateLED();
    yield();
    FastLED.delay(1);
  }
}
void ShowIP() {
  //The LEDs are divided into 10 sections, and each char in the IP range will be shown one at a time. Left down will be 0 and the next clockwise will be 1.
  //Where RGB will be the order of the numbers, so red will have a *100 multiplication and green will have *10
  //so when the IP is 198.168.1.150 and it shows the last section (150) then section 1 will be Red, 5 will be green and 0 will be blue
  IPAddress MyIp = WiFi.localIP();
#ifdef SerialEnabled
  Serial.print("My ip = ");
  Serial.println(WiFi.localIP());                               //Just send it's IP on boot to let you know
#endif //SerialEnabled

  ShowIPnumber(MyIp[0]);
  MyDelay(30000, 500, true);
  ShowIPnumber(MyIp[1]);
  MyDelay(30000, 500, true);
  ShowIPnumber(MyIp[2]);
  MyDelay(30000, 500, true);
  ShowIPnumber(MyIp[3]);
}
