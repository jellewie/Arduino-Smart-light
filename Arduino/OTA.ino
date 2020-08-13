/* Written by JelleWho https://github.com/jellewie

   DO NOT USE:
   Prefix 'OTA' for anything
*/
#include <ArduinoOTA.h>

#define OTA_Name Name

bool OTA_Enabled;

void OTA_setup() {
  OTA_Enabled = true;
  ArduinoOTA.setHostname(OTA_Name);
  OTA_loop();
}
bool OTA_loop() {
  if (!OTA_Enabled)                 //If OTA is disabed
    return false;                   //Stop and do nothing with OTA
  static bool OTA_Started = false;
  if (!OTA_Started) {               //If OTA hasn't started yet
    ArduinoOTA.begin();             //Start OTA
    OTA_Started = true;
  }
  ArduinoOTA.handle();
  if (TickEveryMS(1000)) digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); //Blink every 1 second to show OTA is enabled
  return true;
}
