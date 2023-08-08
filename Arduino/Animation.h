/* Written by JelleWho https://github.com/jellewie

  =====How to add an animation=====
  Animation.ino:  Add the funtion to 'ShowAnimation::switch (CurrentAnimation)' as a new case 'case x+1: {the code}  break;'.
                  'AnimationRGB' can be used for the user set RGB color
                  'Start' Can be used to detect the start of the animation
                  Note that 'UpdateLEDs = true;' must be set to update the leds
  Animation.ino:  Update 'TotalAnimations' to be x+1 (Note the SwitchCase counts from 0, not 1)
  =====To add it as a new mode (and not just animation)=====
  functions.h:  Add to 'String ModesString[] = {', it needs a uniquee name! and needs to be in CAPS!
                Also add to 'enum Modes {'
  handler.ino:  Add to webinterface by 'let Dm=.....possibleValues:['   (This is a to long string to automate it, sorry)
  Arduino.ino:  Add the caller to 'switch (Mode) {'  as a new case 'case ###: if (LastMode != Mode) StartAnimation(xxx, -2); break;'. where ### is the enum name and xxx ths ID in the 'switch (CurrentAnimation)'
*/
byte CurrentAnimation;                                          //Which AnimationCounter Animation is selected
byte TotalAnimations = 14;
CRGB AnimationRGB = {0, 0, 0};

//==================================================
//Basic universal LED functions can be found in Functions.h
//
//And some not universal LED functions:
//==================================================
bool AnimationSinelon(CRGB rgb, byte DimBy, byte BPM) {         // a colored dot sweeping back and forth, with fading trails
  //byte rgb[3] = {255, 0, 0};                                  //The color value
  //byte DimBy = 2;                                             //Delay in ms to fade to black
  //Returns true if this is the start
  fadeToBlackBy(LEDs, TotalLEDsClock, DimBy);
  byte pos = beatsin8(BPM, 0, TotalLEDsClock - 1);
  LEDs[LEDtoPosition(pos)] += rgb;
  static byte Lastpos;
  if ((pos == 0 or pos == TotalLEDsClock - 1) and Lastpos != pos) { //If we just ended
    Lastpos = pos;
    return true;
  }
  Lastpos = pos;
  return false;
}
//==================================================
//End of LED functions
//==================================================
void SetNewColor() {                                            //Sets 'AnimationRGB[]' to a new color, so the Animation would change color
  AnimationRGB.r = 0; AnimationRGB.g = 0; AnimationRGB.b = 0;   //Clear animation color, so we can set a new one
  switch (random8(0, 6)) {
    case 0: AnimationRGB.r = 255;                                               break;
    case 1:                       AnimationRGB.g = 255;                         break;
    case 2: AnimationRGB.r = 255; AnimationRGB.g = 255;                         break;
    case 3:                                               AnimationRGB.b = 255; break;
    case 4: AnimationRGB.r = 255;                         AnimationRGB.b = 255; break;
    case 5:                       AnimationRGB.g = 255;   AnimationRGB.b = 255; break;
  }
}
//==================================================
void ShowAnimation(bool Start) {                                //This would be called to show an Animation every hour
  EVERY_N_SECONDS(1) {
    if (AnimationCounter != -1)
      AnimationCounter = AnimationCounter - 1;
#ifdef Animation_SerialEnabled
    Serial.println("AN: AnimationCounter=" + String(AnimationCounter));
#endif //Animation_SerialEnabled
  }
  switch (CurrentAnimation) {
    case 0: {                                                   //BLINK
        static byte _Counter;
        static bool _Direction;
        static bool _GoingOn;
        if (Start) {
          _Counter = 0;
          _GoingOn = true;
          _Direction = random8(0, 2);
        }
#define ANIMATION_TIME_BLINK (10000 / TotalLEDsClock) - 1
        EVERY_N_MILLISECONDS(ANIMATION_TIME_BLINK) {            //Make it so it does a full round every 10s
          if (_GoingOn)
            LED_Blink(LEDtoPosition(0), TotalLEDsClock, AnimationRGB, 1, &_Counter, _Direction, true, TotalLEDsClock);
          else
            LED_Blink(LEDtoPosition(0), TotalLEDsClock, CRGB(0, 0, 0), 1, &_Counter, _Direction, false, TotalLEDsClock);
          if (_Counter == 0)
            _GoingOn = !_GoingOn;
          UpdateLEDs = true;
        }
      } break;
    case 1: {                                                   //BPM
        CRGBPalette16 palette = PartyColors_p;                  //(const CRGBPalette16 &pal, uint8_t index, uint8_t brightness=255, TBlendType blendType=LINEARBLEND)
        byte beat = beatsin8(20, 64, 255);
        const byte gHue = 9;
        for (int i = 0; i < TotalLEDsClock; i++)
          LEDs[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
        UpdateLEDs = true;
      } break;
    case 2: {                                                   //CONFETTI
        if (Start) ClockClear();
#define ANIMATION_TIME_CONFETTI 1000 / 60
        EVERY_N_MILLISECONDS(ANIMATION_TIME_CONFETTI) {         //Limit to x FPS
          fadeToBlackBy(LEDs, TotalLEDsClock, 1);               //Dim a color by (X/256ths)
          EVERY_N_MILLISECONDS(50) {
            LEDs[random(TotalLEDsClock)] += AnimationRGB;
          }
          SetNewColor();
          UpdateLEDs = true;
        }
      } break;
    case 3: {                                                   //FLASH
        if (Start) ClockClear();
        EVERY_N_MILLISECONDS(500) {
          LED_Flash(0, TotalLEDsClock, AnimationRGB);
          UpdateLEDs = true;
        }
      } break;
    case 4: {                                                   //GLITTER
        if (Start) ClockClear();
#define ANIMATION_TIME_GLITTER 1000 / 60
        EVERY_N_MILLISECONDS(ANIMATION_TIME_GLITTER) {          //Limit to x FPS
          fadeToBlackBy(LEDs, TotalLEDsClock, 1);               //Dim a color by (X/256ths)
          if (random8() < 40)                                   //x/255 change to exectue:
            LEDs[random(TotalLEDsClock)] += AnimationRGB;
          UpdateLEDs = true;
        }
      } break;
    case 5: {                                                   //JUGGLE
        if (Start) ClockClear();
#define ANIMATION_TIME_JUGGLE 1000 / 120
        EVERY_N_MILLISECONDS(ANIMATION_TIME_JUGGLE) {           //Limit to x FPS
          fadeToBlackBy(LEDs, TotalLEDsClock, 10);
          byte dothue = 0;
          for (byte i = 0; i < 8; i++) {
            LEDs[LEDtoPosition(beatsin8(i + 7, 0, TotalLEDsClock))] |= CHSV(dothue, 200, 255); //CHSV = 'angle' (of color wheel), Saturation(rich/pale), 'bright'
            dothue += 32;
          }
          UpdateLEDs = true;
        }
      } break;
    case 6: {                                                   //MOVE
        static byte _Sets, _Length, _Counter;
        static bool _Direction;
        if (Start) {
          _Sets = random8(2, 9);
          _Length = random(2, TotalLEDsClock / (_Sets + 1));
          _Direction = random8(0, 2);
        }
#define ANIMATION_TIME_MOVE (10000 + 500) / (TotalLEDsClock * 3)
        EVERY_N_MILLISECONDS(ANIMATION_TIME_MOVE) {             //Define the speed so it goes around 3 times in 10seconds
          LED_Move(0, TotalLEDsClock, AnimationRGB, _Sets, _Length, &_Counter, _Direction);
          UpdateLEDs = true;
        }
      } break;
    case 7: {                                                   //RAINBOW
        EVERY_N_MILLISECONDS(10) {
          LED_Rainbow(0, TotalLEDsClock, 255 / TotalLEDsClock);
          UpdateLEDs = true;
        }
      } break;
    case 8: {                                                   //SINELON
        if (Start) ClockClear();
#define ANIMATION_TIME_SINELON 1000 / 120
        EVERY_N_MILLISECONDS(ANIMATION_TIME_SINELON) {          //Limit to x FPS
          AnimationSinelon(AnimationRGB, 5, 13);
          UpdateLEDs = true;
        }
      } break;
    case 9: {                                                   //SINELON2
        if (Start) ClockClear();
#define ANIMATION_TIME_SINELON2 1000 / 120
        EVERY_N_MILLISECONDS(ANIMATION_TIME_SINELON2) {         //Limit to x FPS
          if (AnimationSinelon(AnimationRGB, 1, 13))
            SetNewColor();
          UpdateLEDs = true;
        }
      } break;
    case 10: {                                                  //SMILEY
        static bool BlinkLeft;
        static byte BlinkCounter;
        static byte BlinkEachxLoops = 20;
        if (Start) {
          ClockClear();
          BlinkLeft = random8(0, 2);
          BlinkCounter = 0;
          BlinkEachxLoops = random8(20, 50);
          LED_Fill(LEDtoPosition(5 * LEDSections),   4 * LEDSections, AnimationRGB, TotalLEDsClock);  //Right eye
          LED_Fill(LEDtoPosition(50 * LEDSections),  4 * LEDSections, AnimationRGB, TotalLEDsClock);  //Left eye
          UpdateLEDs = true;
        }
        EVERY_N_MILLISECONDS(200) {
          LED_Wobble(LEDtoPosition(15 * LEDSections), 30 * LEDSections, CRGB(255, 0, 0), 1, 20 * LEDSections, TotalLEDsClock);
          UpdateLEDs = true;
        }
        EVERY_N_MILLISECONDS(100) {
          BlinkCounter++;
          if (BlinkCounter >= BlinkEachxLoops) {
            if (BlinkCounter > BlinkEachxLoops) BlinkCounter = 0;
            if (BlinkLeft)
              LED_Flash(LEDtoPosition(5 * LEDSections), 4 * LEDSections, AnimationRGB, CRGB(0, 0, 0), TotalLEDsClock); //Right eye
            else
              LED_Flash(LEDtoPosition(50 * LEDSections), 4 * LEDSections, AnimationRGB, CRGB(0, 0, 0), TotalLEDsClock); //Left eye
          }
          UpdateLEDs = true;
        }
      } break;
    case 11: {                                                  //FLASH2
        EVERY_N_MILLISECONDS(500) {
          static bool _NextNewColor;
          if (_NextNewColor) {
            SetNewColor();
            _NextNewColor = false;
          }
          if (!LED_Flash(0, TotalLEDsClock, AnimationRGB, CRGB(0, 0, 0), TotalLEDsClock))
            _NextNewColor = true;
          UpdateLEDs = true;
        }
      } break;
    case 12: {                                                  //PACMAN
#define PacmanMouthOpen 16 * LEDSections                        //1/4 of a circle         Amounth the mpouth opens (up+down)
#define PacmanMouthOpenhalf (PacmanMouthOpen / 2)               //                        Length the mouth opens (up or down)
#define PacmanMouthMiddle 15 * LEDSections                      //Right (90degrees)       Center of the mouth
#define PacmanEyeLength 6 * LEDSections
#define PacmanEyeOffset 0 * LEDSections                         //Howmuch to move the Eye to the right
#define PacmanBowTieLength 6 * LEDSections
#define PacmanBowTieOffset 4 * LEDSections                      //Howmuch to move the BowTie to the left
        static byte _Counter, _Counter2, _LEDPosU, _LEDPosL;
        static bool _Direcion, _Direcion2, _Left, _Miss;
        if (Start) {
          AnimationRGB = CRGB(255, 255, 0);                     //Set this color, to the web interface would show it
          _Left = random8(0, 2);
          if (random8(0, 4) == 0)                               //Make Miss apearing rarely
            _Miss = random8(0, 2);
          else
            _Miss = false;
          _Counter = 0;
          _Counter2 = 0;
          _Direcion = false;
          _Direcion2 = false;
          LED_Fill(0, TotalLEDsClock, AnimationRGB, TotalLEDsClock); //Fill the whole stip with yellow, we will but stuff out/overwrite it if we need so
          if (_Left) {
            _LEDPosU = (PacmanMouthMiddle - PacmanMouthOpenhalf) + TotalLEDsClock / 2;  //Upperlip position + half a clock
            _LEDPosL = PacmanMouthMiddle + TotalLEDsClock / 2;
            LED_Fill(LEDtoPosition(TotalLEDsClock - PacmanEyeOffset - PacmanEyeLength), PacmanEyeLength, CRGB(0, 0, 0), TotalLEDsClock);                        //Cut out the eye
            if (_Miss) LED_Fill(LEDtoPosition(PacmanBowTieOffset), PacmanBowTieLength, CRGB(255, 0, 0));                                                        //Set here bow tie
          } else {
            _LEDPosU = PacmanMouthMiddle - PacmanMouthOpenhalf;
            _LEDPosL = PacmanMouthMiddle;
            LED_Fill(LEDtoPosition(PacmanEyeOffset), PacmanEyeLength, CRGB(0, 0, 0), TotalLEDsClock);                                                           //Cut out the eye
            if (_Miss) LED_Fill(LEDtoPosition(TotalLEDsClock - PacmanBowTieOffset - PacmanBowTieLength), PacmanBowTieLength, CRGB(255, 0, 0), TotalLEDsClock);  //Set here bow tie
          }
          UpdateLEDs = true;
        }
#define ANIMATION_TIME_PACMAN 400 / PacmanMouthOpen
        EVERY_N_MILLISECONDS(ANIMATION_TIME_PACMAN) {           //Execute animation in such way every WAKA is 0.4seconds as original (~10bites in 101frames at 25FPS=0.4S per WAKA)
          LED_BackAndForth(LEDtoPosition(_LEDPosU), PacmanMouthOpenhalf, CRGB(255, 255, 0), &_Counter,  &_Direcion,  false, true, TotalLEDsClock);              //Upper lip (or lower if direction is reversed)
          LED_BackAndForth(LEDtoPosition(_LEDPosL), PacmanMouthOpenhalf, CRGB(255, 255, 0), &_Counter2, & _Direcion2, true, true, TotalLEDsClock);              //Lower lip
          if (_Miss) {
            LED_Fill(LEDtoPosition(_LEDPosU + _Counter)                                     , LEDSections , CRGB(255, 0, 0), TotalLEDsClock);                   //Color the lips
            LED_Fill(LEDtoPosition(_LEDPosL + PacmanMouthOpenhalf - _Counter2 - LEDSections), LEDSections , CRGB(255, 0, 0), TotalLEDsClock);
          }
          UpdateLEDs = true;
        }
      } break;
    case 13: {                                                  //PHYSICS
#define Speed -0.004   //Lower = slower
#define Drag 0.995      //Lower = more slowdown per step
#define FallToLED TotalLEDsClock / 2
        static CRGB Saved_Color[TotalLEDsClock];
        static float Position[TotalLEDsClock];
        static float Velocity[TotalLEDsClock];
        EVERY_N_SECONDS(30) {                                   //Just repeat is sometimes, to keep the standalone Mode intresting
          for (int i = 0; i < TotalLEDsClock; i++) {
            Position[i] = i;
            Velocity[i] = 0;
          }
        }
        if (Start) {
          for (int i = 0; i < TotalLEDsClock; i++) {
            Position[i] = i;
            Velocity[i] = 0;
            Saved_Color[i] = LEDs[LEDtoPosition(i)];            //Dont use memcpy since its about the same speed, but doesnt allow the offset
          }
        }
#define ANIMATION_TIME_PHYSICS 1000/60
        EVERY_N_MILLISECONDS(ANIMATION_TIME_PHYSICS) {
          ClockClear();                                         //Clear the LEDs so we start from a blank slate
          for (int i = 0; i < TotalLEDsClock; i++) {
            float Acceleration = Speed * (Position[i] - FallToLED);//Calculate howmuch we wish to move (just linear)  https://www.desmos.com/calculator/ljo4mllyzq   y=-\frac{1}{2}\left(x-b\right)
            Velocity[i] = Velocity[i] * Drag + Acceleration;    //Set the Velocity to be (the speed we shere add) * (Drag) + (howmuch we wish to move)
            Position[i] = Position[i] + Velocity[i];            //Calculate new position
            signed int _Pos = round(Position[i]);
            LED_Add(LEDtoPosition(_Pos), 1, Saved_Color[i], TotalLEDsClock); //Draw the LED
          }
          UpdateLEDs = true;
        }
      } break;

    default:
      AnimationCounter = 0;                                     //Stop animation
#ifdef Animation_SerialEnabled
      Serial.println("AN: Animation with ID " + String(CurrentAnimation) + " not found");
#endif //Animation_SerialEnabled
      break;
  }
}
//==================================================
void StartAnimation(byte ID, int Time) {
  AnimationCounter = Time;
  if (AnimationCounter != -1)
    AnimationCounter += 1;                                      //Stop at the given time
  CurrentAnimation = ID;
  SetNewColor();
#ifdef Animation_SerialEnabled
  Serial.println("AN: Selected special mode " + String(CurrentAnimation));
#endif //Animation_SerialEnabled
  ShowAnimation(true);
}
