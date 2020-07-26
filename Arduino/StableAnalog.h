/* Written by JelleWho https://github.com/jellewie */
#define AverageAmount 16                  //On how much points to take the average from      
#define AnalogResolution 12               //howmany bits an analog read is
#define AnalogScaler pow(2,(AnalogResolution - 8))

struct POT {
  byte Value;
  byte Changed;
};
class StableAnalog {
  private:                              //Private variables/functions
    byte pin;
    byte Counter;
    int PointTotal;
    byte Point[AverageAmount];
    byte Old;
    byte InitCount;                       //just the counter to block first measurements

  public:                               //public variables/functions (these can be acces from the normal sketch)
    StableAnalog(byte _pin) {             //Must be <class name> this is kinda the 'root' function
      this->pin = _pin;                   //Make a pointer
      pinMode(pin, INPUT);                //Set the pin as input
    }
    byte Read() {
      PointTotal -= Point[Counter];       //Remove the old value from the total value
      Point[Counter] = (analogRead(pin) / AnalogScaler);
      PointTotal += Point[Counter];       //Add the new value to the total value
      Counter++;
      if (Counter >= AverageAmount)
        Counter = 0;
      byte ReturnValue = PointTotal / AverageAmount;
      return ReturnValue;
    }
    POT ReadStable(byte MinChange, byte Stick, byte SkipFirst) {
      //SkipFirst = Howmany measurements to block on start, so we can read stably, 
      //note this only does skip the first x after the first run. never again after that
      byte New = Read();
      POT RV;
      if (InitCount < SkipFirst) {        //If we havn't yet a valid average (to few points)
        InitCount += 1;
        Old = New;                        //  Update value
        return RV;
      }
      if (abs(New - Old) > MinChange) {   //If we have a big enough change
        RV.Changed = abs(New - Old);
        Old = New;                        //  Update value
      } else
        RV.Changed = 0;
      if (Old < Stick)                    //If we are close to min/0
        Old = 0;                          //  Stick to zero
      if (Old > 255 - Stick)              //If we are close to max/255
        Old = 255;                        //  Stick to max
      RV.Value = Old;
      return RV;
    }
};
