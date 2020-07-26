const byte AnimationCounterTime = 10;                               //Howlong the AnimationCounter hourly Animationjuggle should happen (in Seconds)
byte CurrentAnimation;                                              //Which AnimationCounter Animationjuggle is selected
byte RandomValColor;                                                //random value for the Animationjuggle, used for color
byte RandomVal;
byte Color[3] = {0, 0, 0};

void StartAnimation(byte Animationjuggle) {
  AnimationCounter = AnimationCounterTime + 1;
  CurrentAnimation = Animationjuggle;
  SetNewColor();
  RandomVal = random8();
  Serialprintln("Selected special mode " + String(CurrentAnimation));
  ShowAnimation(true);
}

/* all always white
   1 doesnt wok
   2 does not finisch
   5 sinelon does not start at 0
*/
void ShowAnimation(bool Start) {       //This would be called to show an Animationjuggle every hour
  EVERY_N_SECONDS(1) {
    AnimationCounter = AnimationCounter - 1;
    Serialprintln(String(AnimationCounter));
    UpdateLED = false;
  }
  switch (CurrentAnimation) {
    case 0:
      AnimationMove(Color, ((AnimationCounterTime * 1000 + 500) / TotalLEDs * 2), Start, 2, 3); //void AnimationjuggleMove(byte rgb[3], byte Delay, byte MoveAmount, byte MoveLength)
      break;
    case 1:
      AnimationFlash(Color, 500, Start, 4);                                   //void AnimationFlash(byte rgb[3], int Delay, bool Start, byte BrightScaler)
      break;
    case 2:
      AnimationBlink(Color, (AnimationCounterTime * 1000 / TotalLEDs) - 1, Start, 1, TotalLEDs);     //void AnimationBlink(byte rgb[3], byte Delay, bool Start, byte AlwaysOn, byte LengthBlink)
      break;
    case 3:
      AnimationRainbow(10, 255 / TotalLEDs);                                  //void AnimationRainbow(byte Delay, byte DeltaHue)
      break;
    case 4:
      Animationjuggle(20, 8);                                                 //void Animationjuggle(byte Delay, byte Amount)
      break;
    case 5:
      Animationsinelon(Color, 2, Start, 13);                                  //void Animationsinelon(byte rgb[3], byte DimBy, bool Return)
      break;
    case 6:
      SetNewColor();
      Animationconfetti(Color, 1, 50);                                       //void Animationconfetti(byte rgb[3],byte DimBy, byte Delay)
      break;
    case 7:
      AnimationaddGlitter(Color, 40, 1);                                      //void AnimationaddGlitter(byte rgb[3], byte chanceOfGlitter, byte Delay)
      break;
    default:
      Serialprintln("Animationjuggle with ID " + String(CurrentAnimation) + " not found");
      AnimationCounter = 0;                                                   //Stop animation
      break;
  }
  //Animationbpm(9, 20);   //Disabled since I could not supply enough power for it, and it was ugly anyway
  FastLED.show();                                                             //Update the LEDs
}
void SetNewColor() {                                                          //Sets 'Color[]' to a new color, so the Animationjuggle would change color
  Color[0] = 0;
  Color[1] = 0;
  Color[2] = 0;
  byte Number = random8(0, 5);
  switch (Number) {
    case 0:
      Color[0] = 255;   //R
      break;
    case 1:
      Color[1] = 255;   // G
      break;
    case 2:
      Color[0] = 255;   //R
      Color[1] = 255;   // G
      break;
    case 3:
      Color[2] = 255;   //  B
      break;
    case 4:
      Color[0] = 255;   //R
      Color[2] = 255;   //  B
      break;
    case 5:
      Color[1] = 255;   // G
      Color[2] = 255;   //  B
      break;
  }
  //Serialprintln("Color set to " + String(Color[0]) + "," + String(Color[1]) + "," + String(Color[2]) + " with Number" + String(Number));
}
