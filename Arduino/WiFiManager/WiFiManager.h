/* Written by JelleWho https://github.com/jellewie
  https://github.com/jellewie/Arduino-WiFiManager

  Basic function:
  1. Load EEPROM data (if not yet done)
  2. while(no data) Set up AP mode and wait for user data
  3. try connecting, if (not) {GOTO 2}
*/
#ifndef WiFiManager_h                                           //This prevents including this file more than once
#define WiFiManager_h
#include <WebServer.h>                                          //includes <WiFi.h>
#include "Arduino.h"
#include <EEPROM.h>
#ifdef WiFiManager_DNS
#include <DNSServer.h>
DNSServer dnsServer;
#endif //WiFiManager_DNS
WebServer server(80);
#ifdef WiFiManager_OTA
#include <Update.h>
#endif //WiFiManager_OTA
#ifdef WiFiManager_mDNS
#include <ESPmDNS.h>
#endif //WiFiManager_mDNS
#ifndef WiFiManagerUser_VariableNames_Defined
const String WiFiManager_VariableNames[] = {"SSID", "Password"};
#endif //WiFiManagerUser_VariableNames_Defined
const byte WiFiManager_Settings = sizeof(WiFiManager_VariableNames) / sizeof(WiFiManager_VariableNames[0]); //Why filling this in if we can automate that? :)
#ifdef WiFiManager_DoRequest
enum {REQ_UNK, REQ_SUCCES, REQ_HUB_CONNECT_ERROR, REQ_TIMEOUT, REQ_PAGE_NOT_FOUND, REQ_SETUP_REQUIRED};
#endif //WiFiManager_DoRequest

class CWiFiManager {
  private:
    const char EEPROM_Seperator = char(9);                      //use 'TAB' as a seperator
    const int ConnectionTimeOutMS = 10000;
    bool SettingsEnabled = false;                               //This holds the flag to enable settings, else it would not responce to settings commands
    bool WaitOnAPMode = true;                                   //This holds the flag if we should wait in Apmode for data
    bool TickEveryMS(int);
    byte APMode();                                              //Start a WIFI APmode
    bool Connect(int);
    String LoadEEPROM();                                        //Get raw data from EEPROM
  public:
    int EEPROM_USED = 0;                                        //Howmany bytes we have used for data in the EEPROM
    //#define strip_ip, gateway_ip, subnet_mask to use static IP
#ifndef WiFiManagerUser_password_Defined
    char password[32] = "";                                     //Also defines howmany characters can be in the SSID
#endif //WiFiManagerUser_password_Defined
#ifndef WiFiManagerUser_ssid_Defined
    char ssid[32] = "";                                         //^
#endif //WiFiManagerUser_ssid_Defined
#ifndef WiFiManagerUser_Name_Defined
    char Name[32] = "ESP32";                                    //If you want to define the name somewhere else use 'char* Name = Name'
#endif //WiFiManagerUser_Name_Defined
#ifndef WiFiManagerUser_VariableNames_Defined
    const byte EEPROM_size = 65;                                //Set EEPROM size to default: Max Amount of chars of 'SSID + PASSWORD' (+1)
#endif //WiFiManagerUser_VariableNames_Defined
    void Status_Start();                                        //[UserHook]
    void Status_Done();                                         //[UserHook]
    void Status_Blink();                                        //[UserHook]
    void Status_StartAP();                                      //[UserHook]
    bool HandleAP();                                            //[UserHook]
    bool Set_Value(byte, String);                               //[UserHook]
    String Get_Value(byte, bool, bool);                         //[UserHook]
    void StartServer();                                         //Start the webserver
    void EnableSetup(bool);                                     //Enable/disable setup page
    void RunServer();                                           //Proces server client commands if needed
    void handle_Connect();
    void handle_Settings();
    bool WriteEEPROM();
    bool ClearEEPROM();
    bool CheckAndReconnectIfNeeded(bool);
    byte Start();                                               //Start all WIFI stuff
    byte LoadData();                                            //Only load data from EEPROM to memory
#ifdef WiFiManager_OTA
    bool OTA_Enabled = true;
# ifndef WiFiManagerUser_UpdateWebpage_Defined
    const String UpdateWebpage = "https://github.com/jellewie";
# endif //WiFiManagerUser_UpdateWebpage_Defined
    void handle_uploadPage();
    void handle_update();
    void handle_update2();
#endif //WiFiManager_OTA
#ifdef WiFiManager_DoRequest
    enum {REQ_UNK, REQ_SUCCES, REQ_HUB_CONNECT_ERROR, REQ_TIMEOUT, REQ_PAGE_NOT_FOUND, REQ_SETUP_REQUIRED};
    byte DoRequest(char _IP[16], int _Port = 80, String _Path = "", String _Json = "", byte TimeOut = 0, String Header = "");
#endif //WiFiManager_DoRequest
    void handleClient();
};

CWiFiManager WiFiManager;

#include "WiFiManager.cpp"
#endif
