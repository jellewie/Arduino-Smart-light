//We also could check for 'Serial.availableForWrite()' but we choose to rely on the user for this information (SerialEnabled)
#ifdef SerialEnabled                                                //Serial is enabled
byte Retrieved[3];                                                  //The array where we put the com data in

void HandleSerial() {
  Retrieved[0] = Serial.parseInt();
  Serial.read();
  Retrieved[1] = Serial.parseInt();
  Serial.read();
  Retrieved[2] = Serial.parseInt();
  while (Serial.available() > 0)
    Serial.read();                                                  //Trash all other com commands
  if (Retrieved[0] >= 100) {
    byte b = Retrieved[0] - 100;
    StartAnimation(b);
  } else {
    if (Retrieved[0] > 0 and Retrieved[0] <= 24)
      Time[0] = Retrieved[0];
    else
      Serialprintln("Hours error, please use format 'HH-MM-SS'");
    if (Retrieved[1] > 0 and Retrieved[1] <= 60)
      Time[1] = Retrieved[1];
    else
      Serialprintln("Minutes error, please use format 'HH-MM-SS'");
    if (Retrieved[2] > 0 and Retrieved[2] <= 60)
      Time[2] = Retrieved[2];
    else
      Serialprintln("Seconds error, please use format 'HH-MM-SS'");
    UpdateLED = true;
  }
  ShowTime();
}
void Serialprint(String text) {
  Serial.print(text);
}
void Serialprintln(String text) {
  Serial.println(text);
}
#else //SerialEnabled
void Serialprint(String text) {}    //Make dummies to we won't get compiler errors
void Serialprintln(String text) {}  //Make dummies to we won't get compiler errors
#endif //SerialEnabled
