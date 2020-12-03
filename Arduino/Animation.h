/* Written by JelleWho https://github.com/jellewie

  =====How to add an animation=====
  Animation.ino:  Add the funtion to 'ShowAnimation::switch (CurrentAnimation)' as a new case 'case x+1: {the code}  break;'.
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
//Basic universal LED functions can be found in Functions.h
//
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
#define PacmanMouthOpen 16
#define PacmanMouthOpenhalf PacmanMouthOpen / 2
#define PacmanMouthMiddle 15
#define PacmanStartU PacmanMouthMiddle - PacmanMouthOpenhalf
#define PacmanEyeLength 4
#define PacmanEyeOffset 2
#define PacmanBowTieLength 6
#define PacmanBowTieOffset 8
        static byte _Counter, _Counter2, _LEDPosU, _LEDPosL;
        static bool _Direcion, _Direcion2, _Left, _Miss;
        if (Start) {
          _Left = random8(0, 2);
          if (random8(0, 4) == 0)                                             //Make miss apearing more rare
            _Miss = random8(0, 2);
          else
            _Miss = false;
          _Counter = 0;
          _Counter2 = 0;
          _Direcion = false;
          _Direcion2 = false;
          LED_Fill(0, TotalLEDs, CRGB(255, 255, 0));                         //Fill the whol stip with yellow, we will but stuff out/overwrite it if we need so
          if (_Left) {
            _LEDPosU = PacmanStartU + TotalLEDs / 2;
            _LEDPosL = PacmanMouthMiddle + TotalLEDs / 2;
            LED_Fill(           LEDtoPosition(TotalLEDs - PacmanEyeLength    - (PacmanStartU - PacmanEyeLength    - PacmanEyeOffset)   ), PacmanEyeLength   , CRGB(0, 0, 0));                          //Cut out the eye
            if (_Miss) LED_Fill(LEDtoPosition(TotalLEDs - PacmanBowTieLength - (PacmanStartU - PacmanBowTieLength - PacmanBowTieOffset)), PacmanBowTieLength, CRGB(255, 0, 0)); //Set here bow tie
          } else {
            _LEDPosU = PacmanStartU;
            _LEDPosL = PacmanMouthMiddle;
            LED_Fill(           LEDtoPosition(PacmanStartU - PacmanEyeLength    - PacmanEyeOffset   ), PacmanEyeLength   , CRGB(0, 0, 0));    //Cut out the eye
            if (_Miss) LED_Fill(LEDtoPosition(PacmanStartU - PacmanBowTieLength - PacmanBowTieOffset), PacmanBowTieLength, CRGB(255, 0, 0));  //Set here bow tie
          }
          UpdateLEDs = true;
        }
        EVERY_N_MILLISECONDS(25) {
          LED_BackAndForth(LEDtoPosition(_LEDPosU), PacmanMouthOpenhalf, CRGB(255, 255, 0), &_Counter,  & _Direcion, false);  //Upper lip (or lower if direction is reversed)
          LED_BackAndForth(LEDtoPosition(_LEDPosL), PacmanMouthOpenhalf, CRGB(255, 255, 0), &_Counter2, & _Direcion2, true);  //Lower lip
          if (_Miss) {
            LED_Fill(LEDtoPosition(_LEDPosU + _Counter)                           , 1, CRGB(255, 0, 0));  //Color the lips
            LED_Fill(LEDtoPosition(_LEDPosL + PacmanMouthOpenhalf - _Counter2 - 1), 1, CRGB(255, 0, 0));
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
