//const int TotalLEDs = 60;                                         //The total amounts of LEDs in the strip

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
      fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 0, 0));
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
      fill_solid(&(LEDs[0]), TotalLEDs, CRGB(0, 0, 0));             //Set the whole LED strip to be off (Black)
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
void Animationsinelon(byte rgb[3], byte DimBy, bool Start, byte BPM) { // a colored dot sweeping back and forth, with fading trails
  //byte rgb[3] = {255, 0, 0};                                      //The color value
  //byte DimBy = 2;                                                 //Delay in ms to fade to black

  fadeToBlackBy(LEDs, TotalLEDs, DimBy);

  int pos = 0;
  if (Start)
    pos = 0;
  pos = beatsin8(BPM, 0, TotalLEDs - 1);
  LEDs[pos] += CRGB(rgb[0], rgb[1], rgb[2]);
}
void Animationconfetti(byte rgb[3], byte DimBy, byte Delay) { // random colored speckles that blink in and fade smoothly
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

void Animationbpm(byte gHue, byte BeatsPerMinute) { // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  //byte gHue
  //byte BeatsPerMinute = 60;

  CRGBPalette16 palette = PartyColors_p;
  //(const CRGBPalette16 &pal, uint8_t index, uint8_t brightness=255, TBlendType blendType=LINEARBLEND)

  byte beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < TotalLEDs; i++) //9948
    LEDs[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
}
