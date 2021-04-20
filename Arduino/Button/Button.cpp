/* Written by JelleWho https://github.com/jellewie */

Button::Button(const byte _PIN_Button, const byte ButtonPinMode, const byte _PIN_LED) {
  this->PIN_Button = _PIN_Button;                       //Set the pointer, so we point to the pins
  this->PIN_LED = _PIN_LED;                        		//Set the pointer, so we point to the pins
  pinMode(PIN_Button, ButtonPinMode);                 	//Set the button pin as INPUT
  if (ButtonPinMode == INPUT_PULLUP) HighState = LOW;	//If we have an inverse button (Pushed is 0V/GNS, and released/default is HIGH)
  if (PIN_LED != 0)                                		//If a LED pin is given
    pinMode(PIN_LED, OUTPUT);                      		//Set the LED pin as output
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
  
  if (State.PressEnded) {
    State.Pressed = false;
    State.DoublePress = false;
  }
  
  Button_Time ReturnValue = State;
  State.StartPress = false;
  State.StartLongPress = false;
  State.StartRelease = false;
  State.StartDoublePress = false;
  if (State.PressEnded) {
    State.PressedTime = 0;
    if (!StartReleaseFlagged) {
      StartReleaseFlagged = true;
      State.StartRelease = true;
    }
  }
#ifdef Button_SerialEnabled
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
  if (digitalRead(PIN_Button) == HighState) {                   		//If button is pressed
    State.PressedTime = 0;
    State.Pressed = true;
    State.PressEnded = false;
    StartLongFlagged = false;
    StartReleaseFlagged = false;
    unsigned long ElapsedTimeSinceLast = millis() - LastButtonEndTime;
#ifdef Button_SerialEnabled
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
#ifdef Button_SerialEnabled
    Serial.println("BU:Down PressT=" + String(State.PressedTime));
#endif //Button_SerialEnabled
  }
}

//resetFunc(); is a reset function for Arduino itself I believe