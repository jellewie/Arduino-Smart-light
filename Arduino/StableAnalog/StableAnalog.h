/* Written by JelleWho https://github.com/jellewie
  https://github.com/jellewie/Arduino-Stable-analog-read
*/
#ifndef StableAnalog_h                                  //This prevents including this file more than once
#define StableAnalog_h

#ifndef StableAnalog_AverageAmount
#define StableAnalog_AverageAmount 16                         //On how much points to take the average from
#endif //StableAnalog_AverageAmount

#ifndef StableAnalog_AnalogResolution
#define StableAnalog_AnalogResolution 8                       //Howmany bits an analog read is
#endif //StableAnalog_AnalogResolution

#ifndef StableAnalog_AnalogScaler
#define StableAnalog_AnalogScaler pow(2,(StableAnalog_AnalogResolution - 8))
#endif //StableAnalog_AnalogScaler

struct POT {
  byte Value;
  byte Changed;
};
class StableAnalog {
  private:                                              //Private variables/functions
    byte pin;
    byte Counter;
    int PointTotal;
    byte Point[StableAnalog_AverageAmount];
    byte Old;
    byte InitCount;                                     //just the counter to block first measurements

  public:                                               //public variables/functions (these can be acces from the normal sketch)
    StableAnalog(byte _pin);
	byte Read();
	POT ReadStable(byte MinChange, byte Stick, byte SkipFirst);
};
#include "StableAnalog.cpp"
#endif