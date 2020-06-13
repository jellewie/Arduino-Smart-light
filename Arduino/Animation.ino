/* Written by JelleWho https://github.com/jellewie */
const byte AnimationCounterTime = 10;                               //Howlong the AnimationCounter hourly Animation should happen (in Seconds)
byte CurrentAnimation;                                              //Which AnimationCounter Animation is selected
byte TotalAnimations = 9;
byte RGBColor[3] = {0, 0, 0};

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

void ShowAnimation(bool Start) {       //This would be called to show an Animation every hour
  EVERY_N_SECONDS(1) {
    if (AnimationCounter != -1)
      AnimationCounter = AnimationCounter - 1;
#ifdef SerialEnabled
    Serial.println("AN: AnimationCounter=" + String(AnimationCounter));
#endif //SerialEnabled
  }
  switch (CurrentAnimation) {
    case 0:
#define AnimationMoveValue (AnimationCounterTime * 1000 + 500) / TotalLEDs * 2
      AnimationMove(RGBColor, AnimationMoveValue, Start, 2, 3); //void AnimationMove(byte rgb[3], byte Delay, byte MoveAmount, byte MoveLength)
      break;
    case 1:
      AnimationFlash(RGBColor, 500, Start, 4);                                   //void AnimationFlash(byte rgb[3], int Delay, bool Start, byte BrightScaler)
      break;
    case 2:
      AnimationBlink(RGBColor, (AnimationCounterTime * 1000 / TotalLEDs) - 1, Start, 1, TotalLEDs);     //void AnimationBlink(byte rgb[3], byte Delay, bool Start, byte AlwaysOn, byte LengthBlink)
      break;
    case 3:
      AnimationRainbow(10, 255 / TotalLEDs);                                  //void AnimationRainbow(byte Delay, byte DeltaHue)
      break;
    case 4:
      Animationjuggle(20, 8);                                                 //void Animationjuggle(byte Delay, byte Amount)
      break;
    case 5:
      AnimationSinelon(RGBColor, 2, Start, 13);                               //void AnimationSinelon(byte rgb[3], byte DimBy, bool Return)
      break;
    case 6:
      SetNewColor();
      AnimationConfetti(RGBColor, 1, 50);                                     //void AnimationConfetti(byte rgb[3],byte DimBy, byte Delay)
      break;
    case 7:
      AnimationaddGlitter(RGBColor, 40, 1);                                   //void AnimationaddGlitter(byte rgb[3], byte chanceOfGlitter, byte Delay)
      break;
    case 8:
      AnimationBPM(9, 20);
      break;
    case 9:
      if (AnimationSinelon(RGBColor, 2, Start, 13)) {
        SetNewColor();
      }
      break;
    default:
      AnimationCounter = 0;                                                   //Stop animation
#ifdef SerialEnabled
      Serial.println("AN: Animation with ID " + String(CurrentAnimation) + " not found");
#endif //SerialEnabled
      break;
  }
  FastLED.show();                                                             //Update the LEDs
}
void SetNewColor() {                                                          //Sets 'RGBColor[]' to a new color, so the Animation would change color
  RGBColor[0] = 0;
  RGBColor[1] = 0;
  RGBColor[2] = 0;
  byte Number = random8(0, 5);
  switch (Number) {
    case 0:
      RGBColor[0] = 255;   //R
      break;
    case 1:
      RGBColor[1] = 255;   // G
      break;
    case 2:
      RGBColor[0] = 255;   //R
      RGBColor[1] = 255;   // G
      break;
    case 3:
      RGBColor[2] = 255;   //  B
      break;
    case 4:
      RGBColor[0] = 255;   //R
      RGBColor[2] = 255;   //  B
      break;
    case 5:
      RGBColor[1] = 255;   // G
      RGBColor[2] = 255;   //  B
      break;
  }
}
//==================================================
//And the animation code itzelf
//==================================================
void AnimationMove(byte rgb[3], byte Delay, bool Start, byte MoveAmount, byte MoveLength) {
  //byte rgb[3] = {255, 0, 0};                                      //The color value
  //byte Delay = 84;                                                //Delay in ms for an update in the animation (technically the minimum time)
  //byte MoveAmount = 2;                                            //Quantity of the sections in the move animation
  //byte MoveLength = 2;                                            //Length of the sections in the move animation

  static int CounterMove = 0;                                       //Create a counter (To store the position in)
  if (Start)
    CounterMove = 0;
  int OffsetMove = TotalLEDs / MoveAmount;                          //Calculation for calculating offset from first Position
  int poss[MoveAmount];                                             //Array for saving the positions of the sections
  if (CounterMove >= TotalLEDs) {                                   //Will check if the main position is bigger than the total
    CounterMove = 0;                                                //If that is the case than it will reset it to 0
  } else {
    EVERY_N_MILLISECONDS(Delay) {                                   //Do every 'Delay' ms
      CounterMove++;                                                //It will just set it to 0
      FastLED.clear();
    }
  }
  for (int i = 0; i < MoveAmount; i++) {                            //Beginning of the loop which will send each position and length
    poss[i] = CounterMove + (OffsetMove * i);                       //This will calculate each position by adding the offset times the position number to the first position
    int posX;                                                       //This is the variable which will be used for sending position start
    if (poss[i] >= TotalLEDs) {                                     //To see if the position is to bigger than the total amount
      posX = poss[i] - TotalLEDs;                                   //Subtract the total amount of LEDs from the position number
    } else                                                          //Otherwise it will just use the position data without modifying it
      posX = poss[i];                                               //Just use the position number
    if (posX <= (TotalLEDs - MoveLength)) {                         //If the whole section ends before the total amount is reached it will just us the normal way of setting the LEDs
      fill_solid(&(LEDs[posX]), MoveLength, CRGB(rgb[0], rgb[1], rgb[2])); //With the standard fill solid command from FastLED, LEDs[posX] PosX stands for beginning position, MoveLength will stand for the size of the sections and the last one is the color
    } else if ((posX >= (TotalLEDs - MoveLength)) && (posX <= TotalLEDs)) {//This will check if the thing is beyond the total amount of LEDs
      int calc1 = (TotalLEDs - (posX + MoveLength)) * -1;           //Calculates the amount of LEDs which need to be set from the beginning
      int calc2 = MoveLength - calc1;                               //Calculates the amount of LEDs which need to be set at the last so the total will be reached but won’t be bigger than the total
      fill_solid(&(LEDs[posX]), calc2, CRGB(rgb[0], rgb[1], rgb[2])); //Fills the LEDs at the end of the strip
      fill_solid(&(LEDs[0])   , calc1, CRGB(rgb[0], rgb[1], rgb[2])); //Fills the LEDs at the beginning of the strip
    }
  }
}
void AnimationFlash(byte rgb[3], int Delay, bool Start, byte BrightScaler) {
  //byte rgb[3] = {255, 0, 0};                                      //The color value
  //byte Delay = 500;                                               //Delay in ms for an update in the animation (technically the minimum time)
  //byte BrightScaler = 4;                                          //Scale brightness down
  static bool FlashStateOn = false;                                 //Create a bool to store the flash state in
  if (Start)
    FlashStateOn = false;
  EVERY_N_MILLISECONDS(Delay) {                                     //Do every 'Delay' ms
    FlashStateOn = !FlashStateOn;                                   //Toggle the state
    if (FlashStateOn)                                               //If the flash needs to be on
      fill_solid(&(LEDs[0]), TotalLEDs, CRGB(rgb[0] / BrightScaler, rgb[1] / BrightScaler, rgb[2] / BrightScaler)); //Set the whole LED strip to be the right color
    else
      FastLED.clear();             //Set the whole LED strip to be off (Black)
  }
}
void AnimationBlink(byte rgb[3], byte Delay, bool Start, byte AlwaysOn, byte LengthBlink) {
  //byte rgb[3] = {255, 0, 0};                                      //The color value
  //byte Delay = 84;                                                //Delay in ms for an update in the animation (technically the minimum time)
  //byte AlwaysOn = 1;                                              //How much LEDs should always be on

  static byte CounterBlink = 0;                                     //Create a counter (storing the amounts of LEDs on)
  if (Start)
    CounterBlink = 0;
  EVERY_N_MILLISECONDS(Delay) {                                     //Do every 'DelayAnimationBlink' ms
    fill_solid(&(LEDs[0]), LengthBlink,  CRGB(0, 0, 0));            //Turn LEDs off
    fill_solid(&(LEDs[0]), AlwaysOn,     CRGB(rgb[0], rgb[1], rgb[2]));//Set some LEDs to be always on
    fill_solid(&(LEDs[0]), CounterBlink, CRGB(rgb[0], rgb[1], rgb[2]));//Set the counter amount of LEDs on (this will increase)
    CounterBlink++;                                                 //This will make the blink 1 longer each time
    if (CounterBlink > LengthBlink)                                 //If we are at max length
      CounterBlink = 0;                                             //Reset counter
  }
}
void AnimationRainbow(byte Delay, byte DeltaHue) {
  //byte Delay = 10;                                                //Delay in ms for an update in the animation (technically the minimum time)
  //byte DeltaHue = 255 / TotalLEDs;                                //Diffrence between each LED in hue

  static byte gHue;
  EVERY_N_MILLISECONDS(Delay) {                                     //Do every 'Delay' ms
    gHue++;
    fill_rainbow(LEDs, TotalLEDs, gHue, DeltaHue);
  }
}
void Animationjuggle(byte Delay, byte Amount) { // eight colored dots, weaving in and out of sync with each other
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
bool AnimationSinelon(byte rgb[3], byte DimBy, bool Start, byte BPM) { // a colored dot sweeping back and forth, with fading trails
  //byte rgb[3] = {255, 0, 0};                                      //The color value
  //byte DimBy = 2;                                                 //Delay in ms to fade to black
//Returns true if this is the start
  fadeToBlackBy(LEDs, TotalLEDs, DimBy);

  int pos = 0;
  if (Start)
    pos = 0;
  pos = beatsin8(BPM, 0, TotalLEDs - 1);
  LEDs[pos] += CRGB(rgb[0], rgb[1], rgb[2]);

  static int Lastpot;
  if (pos == 0 and Lastpot != pos)
    return true;     //If we are at the start
  Lastpot = pos;
  return false;
}
void AnimationConfetti(byte rgb[3], byte DimBy, byte Delay) { // random colored speckles that blink in and fade smoothly
  //byte rgb[3] = {255, 0, 0};                                      //The color value
  //byte Delay = 100;                                                //Delay in ms to fade to black

  fadeToBlackBy(LEDs, TotalLEDs, DimBy);
  EVERY_N_MILLISECONDS(Delay) {
    LEDs[random8(TotalLEDs)] += CRGB(rgb[0], rgb[1], rgb[2]);
  }
}
void AnimationaddGlitter(byte rgb[3], byte chanceOfGlitter, byte Delay) {
  //byte rgb[3] = {255, 0, 0};                                      //The color value
  //byte chanceOfGlitter = 40;                                      //Chance in x/255 of a led being glitter
  //byte Delay = 1;                                                 //Delay in ms for an update in the animation (technically the minimum time)

  fadeToBlackBy(LEDs, TotalLEDs, Delay);                            //Dim a color by (X/256ths)
  if (random8() < chanceOfGlitter)
    LEDs[random8(TotalLEDs)] += CRGB(rgb[0], rgb[1], rgb[2]);
}
void AnimationBPM(byte gHue, byte BeatsPerMinute) { // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  //byte gHue
  //byte BeatsPerMinute = 60;

  CRGBPalette16 palette = PartyColors_p;
  //(const CRGBPalette16 &pal, uint8_t index, uint8_t brightness=255, TBlendType blendType=LINEARBLEND)

  byte beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < TotalLEDs; i++)
    LEDs[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
}
