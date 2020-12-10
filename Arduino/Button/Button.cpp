/* Written by JelleWho https://github.com/jellewie */
Button::Button(const buttons Input) {                   //Called to initialize
  this->Data = Input;                                   //Set the pointer, so we point to the pins
  pinMode(Data.PIN_Button, INPUT);                      //Set the button pin as INPUT
  if (Data.PIN_LED != 0)                                //If a LED pin is given
    pinMode(Data.PIN_LED, OUTPUT);                      //Set the LED pin as output
  Pinchange();                                          //Init the pin, this will make sure it starts in the right HIGH or LOW state
};
Button_Time Button::CheckButton() {
  if (!State.PressEnded)
    State.PressedTime = millis() - ButtonStartTime;     //If still pushing; give back pushed time so far
  if (State.PressedTime > Time_StartLongPressMS) {      //if it was/is a long press
    if (State.PressedTime > Time_ESPrestartMS)          //if it was/is a way to long press
      ESP.restart();                                    //Restart the ESP
    State.PressedLong = true;                           //Flag it's a long pres
    if (!StartLongFlagged) {                            //If it's started to be a long press
      State.StartLongPress = true;                      //Flag that this was a long press
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
#ifndef Button_SerialEnabled
  Serial.println("BU:CheckButton="
                 "S" + String(State.StartPress) + "." + String(State.Pressed) + "_"
                 "L" + String(State.StartLongPress) + "." + String(State.PressedLong) + "_"
                 "D" + String(State.StartDoublePress) + "." + String(State.DoublePress) + "_"
                 "R" + String(State.StartRelease) + "." + String(State.PressEnded) + "_"
                 "T" + String(State.PressedTime));
#endif //Button_SerialEnabled
  return ReturnValue;
}
void Button::Pinchange() {
  //We do not need special overflow code here. Here I will show you with 4 bits as example
  //ButtonStartTime = 12(1100)    millis = 3(0011)    PressedTime should be = 7 (13,14,15,0,1,2,3 = 7 ticks)
  //PressedTime = millis() - ButtonStartTime[i] = 3-12=-9(1111 0111) overflow! = 7(0111)  Thus there is nothing to fix, it just works
  if (digitalRead(Data.PIN_Button)) {                   //If button is pressed
    State.PressedTime = 0;
    State.Pressed = true;
    State.PressEnded = false;
    StartLongFlagged = false;
    StartReleaseFlagged = false;
    unsigned long ElapsedTimeSinceLast = millis() - LastButtonEndTime;
#ifndef Button_SerialEnabled
    Serial.println("BU:Up TsinceLast=" + String(ElapsedTimeSinceLast));
#endif //Button_SerialEnabled
    if (ElapsedTimeSinceLast > Time_RejectStarts) {
      ButtonStartTime = millis();                       //Save the start time
      State.StartPress = true;
      if (ElapsedTimeSinceLast < Time_StartDoublePress) {
        State.StartDoublePress = true;
        State.DoublePress = true;
      }
    }
  } else if (millis() - ButtonStartTime > Time_ESPrestartMS) {//If the button was pressed longer than 10 seconds
    ESP.restart();                                      //Restart the ESP
  } else {
    State.PressEnded = true;
    State.PressedTime = millis() - ButtonStartTime;
    LastButtonEndTime = millis();
#ifndef Button_SerialEnabled
    Serial.println("BU:Down PressT=" + String(State.PressedTime));
#endif //Button_SerialEnabled
  }
}

//resetFunc(); is a reset function for Arduino itself I believe