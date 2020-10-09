/* Written by JelleWho https://github.com/jellewie
  This file declairs the function types and does not reply on other files
*/
struct TimeS {
  byte HH, MM, SS;
  unsigned long Ticks;
};
enum Modes {OFF, ON, WIFI, RESET, CLOCK,
            BLINK, BPM, CONFETTI, FLASH, GLITTER, JUGGLE, MOVE,  RAINBOW, SINELON, SINELON2
           };     //Just to make the code more clear to read, OFF=0 and ON=1 etc
String ModesString[] = {"OFF", "ON", "WIFI", "RESET", "CLOCK",
                        "BLINK", "BPM", "CONFETTI", "FLASH", "GLITTER", "JUGGLE", "MOVE",  "RAINBOW", "SINELON", "SINELON2"
                       }; //ALL CAPS!
const byte Modes_Amount = sizeof(ModesString) / sizeof(ModesString[0]); //Why filling this in if we can automate that? :)

//==============================
//Convert stuff
//==============================
String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3])  ;
}
bool StringisDigit(String IN) {
  for (byte i = 0; i < IN.length(); i++) {
    if (not isDigit(IN.charAt(i)))
      return false;
  }
  return true;
}
String ConvertModeToString(byte IN) {
#ifdef Convert_SerialEnabled
  Serial.println("CV: ConvertModeToString '" + String(IN) + "'");
#endif //Convert_SerialEnabled
  if (IN < Modes_Amount)
    return ModesString[IN];
  return "UNK";
}
int ConvertModeToInt(String IN) {
#ifdef Convert_SerialEnabled
  Serial.println("CV: ConvertModeToInt '" + IN + "'");
#endif //Convert_SerialEnabled
  if (StringisDigit(IN)) {
    if (IN.toInt() < Modes_Amount)
      return IN.toInt();
    else
      return 0;
  }
  IN.trim();
  IN.toUpperCase();
  for (byte i = 0; i < Modes_Amount; i++) {
    if (IN == ModesString[i])
      return i;
  }
  return 0;
}
bool IsTrue(String input) {
  input.toLowerCase();
  if (input.toInt() == 1 or input == "true" or input == "yes" or input == "high")
    return true;
  return false;
}
String IsTrueToString(bool input) {
  if (input)
    return "TRUE";
  return "FALSE";
}
