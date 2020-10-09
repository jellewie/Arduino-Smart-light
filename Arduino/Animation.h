/* Written by JelleWho https://github.com/jellewie

  =====How to add an animation=====
  Animation.ino:  Add the funtion 'void Animation###(){}' this will be called repearetly asap when the animation needs to be shown, this function should be not blocking.
  Animation.ino:  Add the caller to 'switch (CurrentAnimation)' as a new case 'case x+1: Animation#();  break;'. 'AnimationRGB' can be used for the user set RGB color
  Animation.ino:  Update 'TotalAnimations' to be x+1
  =====To add it as a new mode (and not just animation)=====
  functions.h:  Add to 'String ModesString[] = {', it needs a uniquee name! and needs to be in CAPS!
  functions.h:  Also add to 'enum Modes {'
  handler.ino:  Add to webinterface by 'let Dm=.....possibleValues:['   (This is a to long string to automate it, sorry)
  Arduino.ino:  Add the caller to 'switch (Mode) {'  as a new case 'case ###:    if (LastMode != Mode) StartAnimation(xxx, -2);    UpdateLEDs = true;    break;'. where ### is the enum name and xxx ths ID in the 'switch (CurrentAnimation)'
*/
byte CurrentAnimation;                                              //Which AnimationCounter Animation is selected
byte TotalAnimations = 11;
CRGB AnimationRGB = {0, 0, 0};

//==================================================
//Basic universal LED functions. These includes start postion, amount (inc overflow correction)
//==================================================
void Fill(int From, int Amount, CRGB Color) {
  if (From + Amount >= TotalLEDs) {                                 //Overflow protection
    byte calc1 = TotalLEDs - From;                                  //Calculates the amount of LEDs which need to on on the end of the strip
    fill_solid(&(LEDs[From]), calc1,  Color);
    fill_solid(&(LEDs[0]), Amount - calc1, Color);
  } else
    fill_solid(&(LEDs[From]), Amount, Color);
}
void AnimationMove(byte From, byte Amount, CRGB Color, byte Sets, int Length, byte *Counter, bool Reverse = false, bool Reset = true);
void AnimationMove(byte From, byte Amount, CRGB Color, byte Sets, int Length, byte *Counter, bool Reverse, bool Reset) {
  //From = The first LED to do the animation on
  //Amount = The amount of LEDS to do the animation on
  //Color = the RGB value to use
  //Sets = The amount of sets going around
  //Length = How long each set would be
  //Counter = a pointer to a counter where we keep track of how far we are
  //Reverse = Reverse the animation direction
  //Reset = reset the LED color with every call (do not use if using overlapping moves)
  //Example:
  //  static int counter;
  //  Move(5, 10, CRGB(0, 1, 0), 2, 2, &counter);

  int Count = *Counter;
  if (Reverse)
    Count = Amount - Count - 1;
  if (Reset)
    Fill(From, Amount, CRGB(0, 0, 0));
  int poss[Sets];                                           //Array for saving the positions of the sections
  for (int i = 0; i < Sets; i++) {                          //Beginning of the loop which will send each position and length
    poss[i] = Count + Amount * i / Sets;                    //This will calculate each position by adding the offset times the position number to the first position
    byte posX;                                              //This is the variable which will be used for sending position start
    if (poss[i] >= Amount) {                                //To see if the position is to bigger than the total amount
      posX = From + poss[i] - Amount;                       //Subtract the total amount of LEDs from the position number
    } else {                                                //Otherwise it will just use the position data without modifying it
      posX = From + poss[i];                                //Just use the position number
    }
    if (posX <= From + Amount - Length) {                   //If the whole section ends before the total amount is reached it will just us the normal way of setting the LEDs
      Fill(posX, Length, Color);                            //With the standard fill solid command from FastLED, LEDs[posX] PosX stands for beginning position, Amount will stand for the size of the sections and the last one is the color
    } else {
      byte calc1 = (From + Amount) - posX;                  //Calculates the amount of LEDs which need to be set from the beginning
      Fill(posX, calc1, Color);                             //Fills the LEDs at the end of the strip
      Fill(From, Length - calc1, Color);                    //Fills the LEDs at the beginning of the strip
    }
  }
  *Counter += 1;
  if (*Counter >= Amount) {
    *Counter = *Counter - Amount;
  }
}
bool AnimationFlash(byte From, byte Amount, CRGB Color, CRGB Color2 = CRGB(0, 0, 0));
bool AnimationFlash(byte From, byte Amount, CRGB Color, CRGB Color2) {
  if (LEDs[From] != Color) {
    Fill(From, Amount, Color);
    return true;
  }
  Fill(From, Amount, Color2);
  return false;
}
void AnimationRainbow(byte From, byte Amount, byte DeltaHue) {
  //byte DeltaHue = Diffrence between each LED in hue
  static byte gHue;
  gHue++;
  if (From + Amount > TotalLEDs) {                                  //Overflow protection
    byte calc1 = TotalLEDs - From;                                  //Calculates the amount of LEDs which need to on on the end of the strip
    fill_rainbow(&(LEDs[From]), calc1, gHue, DeltaHue);
    fill_rainbow(&(LEDs[0]), Amount - calc1, gHue, DeltaHue);
  } else
    fill_rainbow(&LEDs[From], Amount, gHue, DeltaHue);
}
//==================================================
//And the animation code itzelf
//==================================================
void AnimationBlink(CRGB rgb, byte Delay, bool Start, byte AlwaysOn, byte LengthBlink) {
  //byte rgb[3] = {255, 0, 0};                                      //The color value
  //byte Delay = 84;                                                //Delay in ms for an update in the animation (technically the minimum time)
  //byte AlwaysOn = 1;                                              //How much LEDs should always be on
  static byte CounterBlink = 0;                                     //Create a counter (storing the amounts of LEDs on)
  if (Start) CounterBlink = 0;
  EVERY_N_MILLISECONDS(Delay) {                                     //Do every 'DelayAnimationBlink' ms
    fill_solid(&(LEDs[0]), LengthBlink,  CRGB(0, 0, 0));            //Turn LEDs off
    fill_solid(&(LEDs[0]), AlwaysOn,     CRGB(rgb[0], rgb[1], rgb[2]));//Set some LEDs to be always on
    fill_solid(&(LEDs[0]), CounterBlink, CRGB(rgb[0], rgb[1], rgb[2]));//Set the counter amount of LEDs on (this will increase)
    CounterBlink++;                                                 //This will make the blink 1 longer each time
    if (CounterBlink > LengthBlink)                                 //If we are at max length
      CounterBlink = 0;                                             //Reset counter
  }
}
void Animationjuggle(byte Delay, byte Amount) {                     //Eight colored dots, weaving in and out of sync with each other
  //byte Delay = 20
  //byte Amount = 8
  fadeToBlackBy(LEDs, TotalLEDs, Delay);
  byte dothue = 0;
  for (int i = 0; i < Amount; i++) {
    LEDs[beatsin16( i + 7, 0, TotalLEDs - 1 )] |= CHSV(dothue, 200, 255);
    //CHSV = 'angle' (of color wheel), Saturation(rich/pale), 'bright'
    dothue += 32;
  }
}
bool AnimationSinelon(CRGB rgb, byte DimBy, bool Start, byte BPM) { // a colored dot sweeping back and forth, with fading trails
  //byte rgb[3] = {255, 0, 0};                                      //The color value
  //byte DimBy = 2;                                                 //Delay in ms to fade to black
  //Returns true if this is the start
  fadeToBlackBy(LEDs, TotalLEDs, DimBy);
  int pos = 0;
  if (Start) pos = 0;
  pos = beatsin8(BPM, 0, TotalLEDs - 1);
  LEDs[pos] += CRGB(rgb[0], rgb[1], rgb[2]);
  static int Lastpos;
  if (pos == 0 and Lastpos != pos) {  //If we just ended
    Lastpos = pos;
    return true;
  }
  Lastpos = pos;
  return false;
}
void AnimationConfetti(CRGB rgb, byte DimBy, byte Delay) { // random colored speckles that blink in and fade smoothly
  //byte rgb[3] = {255, 0, 0};                                      //The color value
  //byte Delay = 100;                                               //Delay in ms to fade to black
  fadeToBlackBy(LEDs, TotalLEDs, DimBy);
  EVERY_N_MILLISECONDS(Delay) {
    LEDs[random8(TotalLEDs)] += CRGB(rgb[0], rgb[1], rgb[2]);
  }
}
void AnimationaddGlitter(CRGB rgb, byte chanceOfGlitter, byte Delay) {
  //byte rgb[3] = {255, 0, 0};                                      //The color value
  //byte chanceOfGlitter = 40;                                      //Chance in x/255 of a led being glitter
  //byte Delay = 1;                                                 //Delay in ms for an update in the animation (technically the minimum time)
  fadeToBlackBy(LEDs, TotalLEDs, Delay);                            //Dim a color by (X/256ths)
  if (random8() < chanceOfGlitter)
    LEDs[random8(TotalLEDs)] += CRGB(rgb[0], rgb[1], rgb[2]);
}
void AnimationBPM(byte gHue, byte BeatsPerMinute) {                 ///Colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  //byte gHue
  //byte BeatsPerMinute = 60;
  CRGBPalette16 palette = PartyColors_p;
  //(const CRGBPalette16 &pal, uint8_t index, uint8_t brightness=255, TBlendType blendType=LINEARBLEND)
  byte beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < TotalLEDs; i++)
    LEDs[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
}
void AnimationSmiley(CRGB rgb, bool Start, byte Delay) {
  static byte SmileyAnimationType = 0;
  if (Start) {
    SmileyAnimationType = random8(0, 3);
    Fill(LEDtoPosition(5),   4, rgb); //Right eye
    Fill(LEDtoPosition(50),  4, rgb); //Left eye
    Fill(LEDtoPosition(19), 20, CRGB(255, 0, 0)); //Mouth
  }
  //I still want to make an animation, and let the mounth juggle, but there is a out of array error in the AnimationMove that I'm unable to fix
  //  switch (SmileyAnimationType) {
  //    case 0:
  //      SmileyAnimationType = 1;
  //      break;
  //  }
  //  EVERY_N_MILLISECONDS(Delay) {     //Move mouth
  //    static byte _Counter;
  //    static bool _Direction = false;
  //    //if      (_Counter == 0) _Direction = false;
  //    //else if (_Counter == 4) _Direction = true;
  //    AnimationMove(LEDtoPosition(19), 20, CRGB(255, 0, 0), 1, 4, &_Counter, _Direction);
  //  }
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
    case 0: {
        static byte _Sets, _Length, _Counter;
        static bool _Direction;
        if (Start) {
          _Sets = random8(2, 9);
          _Length = random8(2, TotalLEDs / (_Sets + 1));
          _Direction = random8(0, 2);
        }
#define _Delay (10000 + 500) / (TotalLEDs * 4)  //define the speed so it goes around 4 times in 10seconds
        EVERY_N_MILLISECONDS(_Delay) {
          AnimationMove(0, TotalLEDs, AnimationRGB, _Sets, _Length, &_Counter, _Direction);
          UpdateLEDs = true;
        }
      } break;
    case 1: {
        EVERY_N_MILLISECONDS(500) {
          AnimationFlash(0, TotalLEDs, AnimationRGB);
          UpdateLEDs = true;
        }
      } break;
    case 2: {
        AnimationBlink(AnimationRGB, (HourlyAnimationS * 1000 / TotalLEDs) - 1, Start, 1, TotalLEDs);
        UpdateLEDs = true;
      } break;
    case 3: {
        EVERY_N_MILLISECONDS(10) {
          AnimationRainbow(0, TotalLEDs, 255 / TotalLEDs);
        }
      } break;
    case 4: {
        Animationjuggle(20, 8);
        UpdateLEDs = true;
      } break;
    case 5: {
        AnimationSinelon(AnimationRGB, 10, Start, 13);
        UpdateLEDs = true;
      } break;
    case 6: {
        AnimationConfetti(AnimationRGB, 1, 50);
        SetNewColor();
        UpdateLEDs = true;
      } break;
    case 7: {
        AnimationaddGlitter(AnimationRGB, 40, 1);
      } break;
    case 8: {
        AnimationBPM(9, 20);
        UpdateLEDs = true;
      } break;
    case 9: {
        if (AnimationSinelon(AnimationRGB, 2, Start, 13))
          SetNewColor();
        UpdateLEDs = true;
      } break;
    case 10: {
        EVERY_N_MILLISECONDS(500) {
          static bool _NextNewColor;
          if (_NextNewColor) {
            SetNewColor();
            _NextNewColor = false;
          }
          if (!AnimationFlash(0, TotalLEDs, AnimationRGB))
            _NextNewColor = true;
          UpdateLEDs = true;
        }
      } break;
    case 11: {
        AnimationSmiley(AnimationRGB, Start, 1000);
        UpdateLEDs = true;
      } break;

    default:
      AnimationCounter = 0;                                                   //Stop animation
#ifdef SerialEnabled
      Serial.println("AN: Animation with ID " + String(CurrentAnimation) + " not found");
#endif //SerialEnabled
      break;
  }
  FastLED.show();                                                             //Update the LEDs
}
//==================================================
void StartAnimation(byte ID, int Time) {
  AnimationCounter = Time;
  if (AnimationCounter != -1)
    AnimationCounter += 1;    //Stop at the given time

  CurrentAnimation = ID;
  SetNewColor();
#ifdef SerialEnabled
  Serial.println("AN: Selected special mode " + String(CurrentAnimation));
#endif //SerialEnabled
  FastLED.clear();
  ShowAnimation(true);
}
