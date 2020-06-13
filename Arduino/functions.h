/* Written by JelleWho https://github.com/jellewie */
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
