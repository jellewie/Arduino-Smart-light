/* Written by JelleWho https://github.com/jellewie */
#define Time_StartLongPressMS 3000    //After how much Ms we should consider a press a long press
#define Time_ESPrestartMS 15000       //After how much Ms we should restart the ESP, note this is only triggered on released, or on a CheckButtons() call
#define Time_StartDoublePress 200     //Withing how much Ms of the last release should the button be pressed for it to be a double press?
#define Time_RejectStarts 80          //Just some rejection
struct buttons {
  byte PIN_Button;
  byte PIN_LED;
};

struct Button_Time {
  bool StartPress;      //Triggered once on start press
  bool StartLongPress;  //Triggered once if timePressed > LongPress
  bool StartDoublePress;//Triggered once on start press if the last button was les then DoublePress time ago
  bool StartRelease;    //Triggered once on stop press
  bool Pressed;         //If button is pressed
  bool PressedLong;     //If timePressed > LongPress
  bool DoublePress;     //If this press is a double press
  bool PressEnded;      //If button was pressed but isn't
  int PressedTime;      //How long the button is pressed (in MS)
};

class Button {
  private:                                //Private variables/functions
    bool StartLongFlagged;
    bool StartReleaseFlagged = true;
    unsigned long ButtonStartTime;        //the button pressed time (to calculate long press)
    unsigned long LastButtonEndTime;      //To detect double presses
    Button_Time State;
  public:                                 //public variables/functions (these can be acces from the normal sketch)
    buttons Data;                                 //To store the pointer to the (group) of pins of this instance of buttons
    Button(const buttons Input) {                 //Called to initialize
      this->Data = Input;                         //Set the pointer, so we point to the pins
      pinMode(Data.PIN_Button, INPUT);            //Set the button pin as INPUT
      if (Data.PIN_LED != 0)                      //If a LED pin is given
        pinMode(Data.PIN_LED, OUTPUT);            //Set the LED pin as output
      State.PressEnded = true;
    }
    Button_Time CheckButton() {
      if (!State.PressEnded)
        State.PressedTime = millis() - ButtonStartTime; //If still pushing; give back pushed time so far
      if (State.PressedTime > Time_StartLongPressMS) {  //if it was/is a long press
        if (State.PressedTime > Time_ESPrestartMS)      //if it was/is a way to long press
          ESP.restart();                                //Restart the ESP
        State.PressedLong = true;                       //Flag it's a long pres
        if (!StartLongFlagged) {                        //If it's started to be a long press
          State.StartLongPress = true;                  //Flag that this was a long press
          StartLongFlagged = true;
        }
      } else {
        State.PressedLong = false;
      }
      Button_Time ReturnValue = State;
      State.StartPress = false;
      State.StartLongPress = false;
      State.StartRelease = false;
      State.StartDoublePress = false;
      if (State.PressEnded) {
        State.DoublePress = false;
        State.Pressed = false;
        State.PressedTime = 0;
        if (!StartReleaseFlagged) {
          StartReleaseFlagged = true;
          State.StartRelease = true;
        }
      }
      return ReturnValue;
    }
    void Pinchange() {
      //We do not need special overflow code here. Here I will show you with 4 bits as example
      //ButtonStartTime = 12(1100)    millis = 3(0011)    PressedTime should be = 7 (13,14,15,0,1,2,3 = 7 ticks)
      //PressedTime = millis() - ButtonStartTime[i] = 3-12=-9(1111 0111) overflow! = 7(0111)  Thus there is nothing to fix, it just works
      if (digitalRead(Data.PIN_Button)) {                         //If button is pressed
        State.PressedTime = 0;
        State.Pressed = true;
        State.PressEnded = false;
        StartLongFlagged = false;
        StartReleaseFlagged = false;
        ButtonStartTime = millis();                               //Save the start time
        unsigned long ElapsedTimeSinceLast = ButtonStartTime - LastButtonEndTime;
        if (ElapsedTimeSinceLast > Time_RejectStarts) {
          State.StartPress = true;
          if (ElapsedTimeSinceLast < Time_StartDoublePress) {
            State.StartDoublePress = true;
            State.DoublePress = true;
          }
        }
      } else if (millis() - ButtonStartTime > Time_ESPrestartMS) {//If the button was pressed longer than 10 seconds
        ESP.restart();                                            //Restart the ESP
      } else {
        State.PressEnded = true;
        State.PressedTime = millis() - ButtonStartTime;
        LastButtonEndTime = millis();
      }
    }
};
//resetFunc(); is a reset function for Arduino itself I believe
