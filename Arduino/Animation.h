/* Written by JelleWho https://github.com/jellewie

  =====How to add an animation=====
  Animation.ino:  Add the funtion to 'switch (CurrentAnimation)' as a new case 'case x+1: {the code}  break;'.
                  'AnimationRGB' can be used for the user set RGB color
                  'Start' Can be used to detect the start of the animation
                  Note that 'UpdateLEDs = true;' must be set to update the leds
  Animation.ino:  Update 'TotalAnimations' to be x+1
  =====To add it as a new mode (and not just animation)=====
  functions.h:  Add to 'String ModesString[] = {', it needs a uniquee name! and needs to be in CAPS!
                Also add to 'enum Modes {'
  handler.ino:  Add to webinterface by 'let Dm=.....possibleValues:['   (This is a to long string to automate it, sorry)
  Arduino.ino:  Add the caller to 'switch (Mode) {'  as a new case 'case ###: if (LastMode != Mode) StartAnimation(xxx, -2); break;'. where ### is the enum name and xxx ths ID in the 'switch (CurrentAnimation)'
*/
byte CurrentAnimation;                                              //Which AnimationCounter Animation is selected
byte TotalAnimations = 12;
CRGB AnimationRGB = {0, 0, 0};

//==================================================
//Basic universal LED functions. These includes start postion, amount (inc overflow correction and such):
//==================================================
void LED_Fill(byte From, byte Amount, CRGB Color) {
  while (From >= TotalLEDs) From -= TotalLEDs;                      //(Protection out of array bounds) Loop the LEDs around (TotalLEDs+1 is the same as LED 1)
  if (Amount >= TotalLEDs) Amount = TotalLEDs;                      //(Protection out of array bounds) if more LEDs are given than there are in the array, set the amount to all LEDs
  if (From + Amount >= TotalLEDs) {                                 //Overflow protection
    byte calc1 = TotalLEDs - From;                                  //Calculates the amount of LEDs which need to on on the end of the strip
    fill_solid(&(LEDs[From]), calc1, Color);
    fill_solid(&(LEDs[0]), Amount - calc1, Color);
  } else
    fill_solid(&(LEDs[From]), Amount, Color);
}
void LED_Move(byte From, byte Amount, CRGB Color, byte Sets, byte Length, byte *Counter, bool Reverse = false, bool Reset = true);
void LED_Move(byte From, byte Amount, CRGB Color, byte Sets, byte Length, byte *Counter, bool Reverse, bool Reset) {
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
    LED_Fill(From, Amount, CRGB(0, 0, 0));
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
      LED_Fill(posX, Length, Color);                            //With the standard fill solid command from FastLED, LEDs[posX] PosX stands for beginning position, Amount will stand for the size of the sections and the last one is the color
    } else {
      byte calc1 = (From + Amount) - posX;                      //Calculates the amount of LEDs which need to be set from the beginning
      LED_Fill(posX, calc1, Color);                             //Fills the LEDs at the end of the strip
      LED_Fill(From, Length - calc1, Color);                    //Fills the LEDs at the beginning of the strip
    }
  }
  *Counter += 1;
  if (*Counter >= Amount)
    *Counter = *Counter - Amount;
}
bool LED_Flash(byte From, byte Amount, CRGB Color, CRGB Color2 = CRGB(0, 0, 0));
bool LED_Flash(byte From, byte Amount, CRGB Color, CRGB Color2) {
  if (LEDs[From] != Color) {
    LED_Fill(From, Amount, Color);
    return true;
  }
  LED_Fill(From, Amount, Color2);
  return false;
}
void LED_Rainbow(byte From, byte Amount, byte DeltaHue) {
  //byte DeltaHue = Diffrence between each LED in hue
  static byte gHue;
  gHue++;
  while (From >= TotalLEDs) From -= TotalLEDs;                      //(Protection out of array bounds) Loop the LEDs around (TotalLEDs+1 is the same as LED 1)
  if (Amount >= TotalLEDs) Amount = TotalLEDs;                      //(Protection out of array bounds) if more LEDs are given than there are in the array, set the amount to all LEDs
  if (From + Amount >= TotalLEDs) {                                 //Overflow protection
    byte calc1 = TotalLEDs - From;                                  //Calculates the amount of LEDs which need to on on the end of the strip
    fill_rainbow(&(LEDs[From]), calc1, gHue, DeltaHue);
    fill_rainbow(&(LEDs[0]), Amount - calc1, gHue, DeltaHue);
  } else
    fill_rainbow(&LEDs[From], Amount, gHue, DeltaHue);
}
void LED_Wobble(byte From, byte Amount, CRGB Color, byte Sets, byte Length) {
  //Sort of a move, but just back and forth between the start en end

  //this function can only be called once, this 'Counter' is a 1 time counter (could not get the pointer working to attach it to the caller)
  static byte Counter;

  static bool Reverse = false;
  LED_Move(From, Amount, Color, Sets, Length, &Counter, Reverse);
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
void LED_Blink(byte From, byte Amount, CRGB rgb, byte AlwaysOn, byte *Counter, bool Reverse = false);
void LED_Blink(byte From, byte Amount, CRGB rgb, byte AlwaysOn, byte *Counter, bool Reverse) {
  LED_Fill(From, Amount,       CRGB(0, 0, 0));                      //Turn LEDs off
  if (Reverse) {
    LED_Fill(From + Amount - AlwaysOn, AlwaysOn, rgb);              //Set some LEDs to be always on
    LED_Fill(From + Amount - *Counter, *Counter, rgb);              //Set the counter amount of LEDs on (this will increase)
  } else {
    LED_Fill(From, AlwaysOn,     rgb);                              //Set some LEDs to be always on
    LED_Fill(From, *Counter, rgb);                                  //Set the counter amount of LEDs on (this will increase)
  }
  *Counter += 1;                                                    //This will make the blink 1 longer each time
  if (*Counter > Amount)                                            //If we are at max length
    *Counter = 0;                                                   //Reset counter
}
void LED_BackAndForth(byte From, byte Amount, CRGB rgb, byte *Counter, bool *Direcion, bool Reverse = false, bool Reset = true);
void LED_BackAndForth(byte From, byte Amount, CRGB rgb, byte *Counter, bool *Direcion, bool Reverse, bool Reset) {
  //Fills then emties the range of leds one by one
  if (Reset)
    LED_Fill(From, Amount, CRGB(0, 0, 0));

  if (Reverse)
    LED_Fill(From + Amount - *Counter, *Counter, rgb);            //Set the counter amount of LEDs on
  else
    LED_Fill(From, *Counter, rgb);                                    //Set the counter amount of LEDs on

  if (*Direcion)
    *Counter -= 1;                                                  //This will make the animation_on 1 shorter each time
  else
    *Counter += 1;                                                  //This will make the animation_on 1 longer each time
  if (*Counter >= Amount or * Counter == 0)                          //If we are at max length or at the start
    *Direcion = !*Direcion;                                         //Flip direction
}
//==================================================
//And some not universal LED functions:
//==================================================
bool AnimationSinelon(CRGB rgb, byte DimBy, bool Start, byte BPM) { // a colored dot sweeping back and forth, with fading trails
  //byte rgb[3] = {255, 0, 0};                                      //The color value
  //byte DimBy = 2;                                                 //Delay in ms to fade to black
  //Returns true if this is the start
  fadeToBlackBy(LEDs, TotalLEDs, DimBy);
  byte pos = 0;
  if (Start) pos = 0;
  pos = beatsin8(BPM, 0, TotalLEDs - 1);
  LEDs[pos] += rgb;
  static byte Lastpos;
  if (pos == 0 and Lastpos != pos) {                                //If we just ended
    Lastpos = pos;
    return true;
  }
  Lastpos = pos;
  return false;
}
//==================================================
//End of LED functions
//==================================================
void SetNewColor() {                                                          //Sets 'AnimationRGB[]' to a new color, so the Animation would change color
  AnimationRGB.r = 0; AnimationRGB.g = 0; AnimationRGB.b = 0;                 //Clear animation color, so we can set a new one
  switch (random8(0, 5)) {
    case 0: AnimationRGB.r = 255;                                               break;
    case 1:                       AnimationRGB.g = 255;                         break;
    case 2: AnimationRGB.r = 255; AnimationRGB.g = 255;                         break;
    case 3:                                               AnimationRGB.b = 255; break;
    case 4: AnimationRGB.r = 255;                         AnimationRGB.b = 255; break;
    case 5:                       AnimationRGB.g = 255;   AnimationRGB.b = 255; break;
  }
}
//==================================================
void ShowAnimation(bool Start) {       //This would be called to show an Animation every hour
  EVERY_N_SECONDS(1) {
    if (AnimationCounter != -1)
      AnimationCounter = AnimationCounter - 1;
#ifdef SerialEnabled
    Serial.println("AN: AnimationCounter=" + String(AnimationCounter));
#endif //SerialEnabled
  }
  switch (CurrentAnimation) {
    case 0: {                                                                 //BLINK
        static byte _Counter;
        static bool _Direction;
        if (Start) {
          _Counter = 0;
          _Direction = random8(0, 2);
        }
        EVERY_N_MILLISECONDS((10000 / TotalLEDs) - 1) {
          LED_Blink(0, TotalLEDs, AnimationRGB, 1, &_Counter, _Direction);
          UpdateLEDs = true;
        }
      } break;
    case 1: {                                                                 //BPM
        CRGBPalette16 palette = PartyColors_p; //(const CRGBPalette16 &pal, uint8_t index, uint8_t brightness=255, TBlendType blendType=LINEARBLEND)
        byte beat = beatsin8(20, 64, 255);
#define gHue 9
        for (byte i = 0; i < TotalLEDs; i++)
          LEDs[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
        UpdateLEDs = true;
      } break;
    case 2: {                                                                 //CONFETTI
        fadeToBlackBy(LEDs, TotalLEDs, 1);                                    //Dim a color by (X/256ths)
        EVERY_N_MILLISECONDS(50) {
          LEDs[random8(TotalLEDs)] += AnimationRGB;
        }
        SetNewColor();
        UpdateLEDs = true;
      } break;
    case 3: {                                                                 //FLASH
        EVERY_N_MILLISECONDS(500) {
          LED_Flash(0, TotalLEDs, AnimationRGB);
          UpdateLEDs = true;
        }
      } break;
    case 4: {                                                                 //GLITTER
        fadeToBlackBy(LEDs, TotalLEDs, 1);                                    //Dim a color by (X/256ths)
        if (random8() < 40)                                                   //x/255 change to exectue:
          LEDs[random8(TotalLEDs)] += AnimationRGB;
        UpdateLEDs = true;
      } break;
    case 5: {                                                                 //JUGGLE
        fadeToBlackBy(LEDs, TotalLEDs, 20);
        byte dothue = 0;
        for (byte i = 0; i < 8; i++) {
          LEDs[beatsin8( i + 7, 0, TotalLEDs - 1 )] |= CHSV(dothue, 200, 255); //CHSV = 'angle' (of color wheel), Saturation(rich/pale), 'bright'
          dothue += 32;
        }
        UpdateLEDs = true;
      } break;
    case 6: {                                                                 //MOVE
        static byte _Sets, _Length, _Counter;
        static bool _Direction;
        if (Start) {
          _Sets = random8(2, 9);
          _Length = random8(2, TotalLEDs / (_Sets + 1));
          _Direction = random8(0, 2);
        }
#define _Delay (10000 + 500) / (TotalLEDs * 4)  //define the speed so it goes around 4 times in 10seconds
        EVERY_N_MILLISECONDS(_Delay) {
          LED_Move(0, TotalLEDs, AnimationRGB, _Sets, _Length, &_Counter, _Direction);
          UpdateLEDs = true;
        }
      } break;
    case 7: {                                                                 //RAINBOW
        EVERY_N_MILLISECONDS(10) {
          LED_Rainbow(0, TotalLEDs, 255 / TotalLEDs);
          UpdateLEDs = true;
        }
      } break;
    case 8: {                                                                 //SINELON
        AnimationSinelon(AnimationRGB, 10, Start, 13);
        UpdateLEDs = true;
      } break;
    case 9: {                                                                 //SINELON2
        if (AnimationSinelon(AnimationRGB, 2, Start, 13))
          SetNewColor();
        UpdateLEDs = true;
      } break;
    case 10: {                                                                //SMILEY
        static bool BlinkLeft;
        static byte BlinkCounter;
        static byte BlinkEachxLoops = 20;
        if (Start) {
          BlinkLeft = random8(0, 2);
          BlinkCounter = 0;
          BlinkEachxLoops = random8(20, 50);
          LED_Fill(LEDtoPosition(5),   4, AnimationRGB);                      //Right eye
          LED_Fill(LEDtoPosition(50),  4, AnimationRGB);                      //Left eye
          UpdateLEDs = true;
        }
        EVERY_N_MILLISECONDS(200) {
          LED_Wobble(LEDtoPosition(15), 30, CRGB(255, 0, 0), 1, 20);
          UpdateLEDs = true;
        }
        EVERY_N_MILLISECONDS(100) {
          BlinkCounter++;
          if (BlinkCounter >= BlinkEachxLoops) {
            if (BlinkCounter > BlinkEachxLoops) BlinkCounter = 0;
            if (BlinkLeft)
              LED_Flash(LEDtoPosition(5), 4, AnimationRGB);                   //Right eye
            else
              LED_Flash(LEDtoPosition(50), 4, AnimationRGB);                  //Left eye
          }
          UpdateLEDs = true;
        }
      } break;
    case 11: {                                                                //FLASH2
        EVERY_N_MILLISECONDS(500) {
          static bool _NextNewColor;
          if (_NextNewColor) {
            SetNewColor();
            _NextNewColor = false;
          }
          if (!LED_Flash(0, TotalLEDs, AnimationRGB))
            _NextNewColor = true;
          UpdateLEDs = true;
        }
      } break;
    case 12: {                                                                //PACMAN

        //TODO maybe add a rare female PACMAN

#define PacmanMouthOpen 16
#define PacmanMouthOpenhalf PacmanMouthOpen / 2
#define PacmanStartT 15 - PacmanMouthOpenhalf
#define PacmanStartB 15
        static byte _Counter, _Counter2;
        static bool _Direcion, _Direcion2, _LeftMouth;
        if (Start) {
          _LeftMouth = random8(0, 2);
          _Counter = 0;
          _Counter2 = 0;
          _Direcion = false;
          _Direcion2 = false;
          LED_Fill(0, TotalLEDs, CRGB(255, 255, 0));
          if (_LeftMouth)
            LED_Fill(60 - LEDtoPosition(PacmanStartT - 1), 2, CRGB(0, 0, 0)); //Cut out the eye
          else
            LED_Fill(LEDtoPosition(PacmanStartT - 4), 2, CRGB(0, 0, 0));      //Cut out the eye
          UpdateLEDs = true;
        }
        EVERY_N_MILLISECONDS(25) {
          if (_LeftMouth) {
            LED_BackAndForth(60 - LEDtoPosition(PacmanStartT), PacmanMouthOpenhalf, CRGB(255, 255, 0), &_Counter,  & _Direcion, false);  //Upper lip
            LED_BackAndForth(60 - LEDtoPosition(PacmanStartB), PacmanMouthOpenhalf, CRGB(255, 255, 0), &_Counter2, & _Direcion2, true);  //Lower lip
          } else {
            LED_BackAndForth(LEDtoPosition(PacmanStartT), PacmanMouthOpenhalf, CRGB(255, 255, 0), &_Counter,  & _Direcion, false);  //Upper lip
            LED_BackAndForth(LEDtoPosition(PacmanStartB), PacmanMouthOpenhalf, CRGB(255, 255, 0), &_Counter2, & _Direcion2, true);  //Lower lip
          }
          UpdateLEDs = true;
        }
      } break;

    default:
      AnimationCounter = 0;                                                   //Stop animation
#ifdef SerialEnabled
      Serial.println("AN: Animation with ID " + String(CurrentAnimation) + " not found");
#endif //SerialEnabled
      break;
  }
}
//==================================================
void StartAnimation(byte ID, int Time) {
  AnimationCounter = Time;
  if (AnimationCounter != -1)
    AnimationCounter += 1;                                                    //Stop at the given time
  CurrentAnimation = ID;
  SetNewColor();
#ifdef SerialEnabled
  Serial.println("AN: Selected special mode " + String(CurrentAnimation));
#endif //SerialEnabled
  FastLED.clear();
  ShowAnimation(true);
}
