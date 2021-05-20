/* Written by JelleWho https://github.com/jellewie
   https://github.com/jellewie/Arduino-Stable-analog-read
*/
StableAnalog::StableAnalog(byte _pin) {                         //Must be <class name> this is kinda the 'root' function
  this->pin = _pin;                                             //Make a pointer
  pinMode(pin, INPUT);                                          //Set the pin as input
  analogReadResolution(StableAnalog_AnalogResolution);
}
byte StableAnalog::Read(byte Mode) {
  //Returns the average of the last StableAnalog_AverageAmount measurements (where a measurement is a call to this code)
  PointTotal -= Point[Counter];                                 //Remove the old value from the total value
  if (Mode == 0) {
    Point[Counter] = (analogRead(pin) / StableAnalog_AnalogScaler);
  } else if (Mode == 1) {
#if defined(ESP32)										        //Don't call touchRead if not on a ESP32, this would give compiler errors
	Point[Counter] = (touchRead(pin) / StableAnalog_AnalogScaler);
#else
    Point[Counter] = -1;                                        //When no ESP32 with touchRead support is used, just return invalid data in the hope the user catches it.
#endif
  }
  PointTotal += Point[Counter];                                 //Add the new value to the total value
  Counter++;
  if (Counter >= StableAnalog_AverageAmount)
    Counter = 0;
  byte ReturnValue = PointTotal / StableAnalog_AverageAmount;
  return ReturnValue;
}
POT StableAnalog::ReadStable(byte MinChange, byte Stick, byte SkipFirst, byte Mode) {
  //SkipFirst    How many measurements to block on start, so we can read stably, note this only does skip the first x after the first run. never again after that
  //PotMinChange How much the pot_value needs to change before we process it
  //PotStick     If this close to HIGH or LOW stick to it
  //SkipFirst    Skip the first x of measurements, this is so we have proper averages, else if will climb up from 0
  //Mode         0=analogRead(default), 1=touchRead

  //Calibration guide, to as sensitive as possible:
  //1. Crank StableAnalog_AverageAmount up to 32, or as high as your code looping /response allows you to
  //2. Run ReadStable(0,0, StableAnalog_AverageAmount)
  //3. Log the 'A.Changed' output, set PotMinChange as such
  //4. Increase PotMinChange by one if you have false positives (triggers that should not happen)
  
  byte New = Read(Mode);
  POT RV;
  RV.Raw=New;
  if (InitCount < SkipFirst) {                                  //If we have not yet a valid average (to few points)
    InitCount += 1;
    Old = New;                                                  //  Update value
    return RV;
  } else if (InitCount == SkipFirst) {                          //If we are done, and the first update needs to be send
    InitCount += 1;
    Old = New - 100;                                            //  Throw the value off, so we force an update
  }
  if (abs(New - Old) > MinChange) {                             //If we have a big enough change
    RV.Changed = abs(New - Old);
    Old = New;                                                  //  Update value
  } else
    RV.Changed = 0;
  if (Old < Stick)                                              //If we are close to min/0
    Old = 0;                                                    //  Stick to zero
  if (Old > 255 - Stick)                                        //If we are close to max/255
    Old = 255;                                                  //  Stick to max
  RV.Value = Old;
  return RV;
}