/*
 * This code is disabled for uploading by 'DCF77'
 * Originaly I though that syncing the clock would be a good idea, but I never inplemented this
 * 
 * Sync clock with radio  https://playground.arduino.cc/Code/DCF77/
 */

#ifdef DCF77

#include "DCF77.h"
#include "Time.h"

const byte DCF_PIN 2                // Connection pin to DCF 77 device
const byte DCF_INTERRUPT 0          // Interrupt number associated with pin

time_t time;
// Non-inverted input on pin DCF_PIN
DCF77 DCF = DCF77(DCF_PIN,DCF_INTERRUPT, true);

void setup() {
  Serial.begin(9600);
  DCF.Start();
  Serialprintln("Waiting for DCF77 time ... ");
  Serialprintln("It will take at least 2 minutes before a first time update.");
}

void loop() {
  delay(1000);
  time_t DCFtime = DCF.getTime(); // Check if new DCF77 time is available
  if (DCFtime!=0)
  {
    Serialprintln("Time is updated");
    setTime(DCFtime);
  }    
  digitalClockDisplay();  
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serialprintln();
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

#endif //DCF77
