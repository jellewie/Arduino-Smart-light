/* Written by JelleWho https://github.com/jellewie
   https://github.com/jellewie/Arduino-Button
*/
#ifndef Button_h                                                //This prevents including this file more than once
#define Button_h

//#define Button_SerialEnabled                                  //Disable to not send Serial debug feedback

#ifndef Time_StartLongPressMS
#define Time_StartLongPressMS 3000                              //After howmuch Ms we should consider a press a long press
#endif //Time_StartLongPressMS

#ifndef Time_ESPrestartMS
#define Time_ESPrestartMS 15000                                 //After howmuch Ms we should restart the ESP, note this is only triggered on released, or on a CheckButtons() call
#endif //Time_ESPrestartMS

#ifndef Time_StartDoublePress
#define Time_StartDoublePress 200                               //Withing howmuch Ms of the last release should the button be pressed for it to be a double press?
#endif //Time_StartDoublePress

#ifndef Time_RejectStarts
#define Time_RejectStarts 80                                    //Just some rejection
#endif //Time_RejectStarts

struct Button_Time {
  bool StartPress;                                              //Triggered once on start press
  bool StartLongPress;                                          //Triggered once if timePressed > LongPress
  bool StartDoublePress;                                        //Triggered once on start press if the last button was less then DoublePress time ago
  bool StartRelease;                                            //Triggered once on stop press
  bool Pressed;                                                 //If button is pressed
  bool PressedLong;                                             //If timePressed > LongPress
  bool DoublePress;                                             //If this press is a double press
  bool PressEnded;                                              //If button was pressed but isn't
  int PressedTime;                                              //How long the button is pressed (in MS)
};
class Button {
  private:                                                      //Private variables/functions
    bool HighState = HIGH;
    bool StartLongFlagged;
    bool StartReleaseFlagged = true;
    unsigned long ButtonStartTime;                              //the button pressed time (to calculate long press)
    unsigned long LastButtonEndTime;                            //To detect double presses
    Button_Time State;
  public:                                                       //public variables/functions (these can be acces from the normal sketch)
    byte PIN_Button;                                            //To store the pointer to pins of this instance of button
    byte PIN_LED;                           //^ but optional (0=unused)
    Button(const byte _PIN_Button, const byte ButtonPinMode = INPUT, const byte _PIN_LED = 0); //Called to initialize
    Button_Time CheckButton();                                  //Call to recieve button state
    void Pinchange();                                           //Called with interupt on pin change
};
#include "Button.cpp"
#endif
