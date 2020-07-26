//TODO FIXME We are not yet processig this time skipped, This means that every 50 days we could drift by each offset (<1s)

void OverFlowError() {
  unsigned long MAX;
  MAX = MAX - 1;

  Serialprintln(String(Time[0]) + ":" + String(Time[1]) + ":" + String(Time[2]));
  Serialprintln("OverFlowError! Now=" + String(millis()) + " NextTime=" + String(NextTime));
  Serialprintln("Time skipped" + String(MAX - NextTime) + "Plus millis()");

  if (MAX - NextTime < Now)                                         //if we can remove the skipped time from the nexttime
    Now = Now - (MAX - NextTime);                                   //Remove it
  else
    Now = 0;                                                        //Set it the the lowest value posible (this is as much as we are able to go right now
  NextTime = Now;
}
