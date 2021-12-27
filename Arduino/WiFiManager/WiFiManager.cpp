#ifdef WiFiManagerUser_Set_Value_Defined
extern bool WiFiManagerUser_Set_Value(byte ValueID, String Value);
#endif //WiFiManagerUser_Set_Value_Defined

#ifdef WiFiManagerUser_Get_Value_Defined
extern String WiFiManagerUser_Get_Value(byte ValueID, bool Safe, bool Convert);
#endif //WiFiManagerUser_Get_Value_Defined

#ifdef WiFiManagerUser_Status_Start_Defined
extern void WiFiManagerUser_Status_Start();
#endif //WiFiManagerUser_Status_Start_Defined

#ifdef WiFiManagerUser_Status_Done_Defined
extern void WiFiManagerUser_Status_Done();
#endif //WiFiManagerUser_Status_Done_Defined

#ifdef WiFiManagerUser_Status_Blink_Defined
extern void WiFiManagerUser_Status_Blink();
#endif //WiFiManagerUser_Status_Blink_Defined

#ifdef WiFiManagerUser_Status_StartAP_Defined
extern void WiFiManagerUser_Status_StartAP();
#endif //WiFiManagerUser_Status_StartAP_Defined

#ifdef WiFiManagerUser_HandleAP_Defined
extern bool WiFiManagerUser_HandleAP();
#endif //WiFiManagerUser_HandleAP_Defined
//===========================================================================
bool CWiFiManager::Set_Value(byte ValueID, String Value) {
  //From EEPROM to RAM
#ifdef WiFiManager_SerialEnabled
  Serial.println("WM: Set current value: " + String(ValueID) + " = " + Value);
#endif //WiFiManager_SerialEnabled
  switch (ValueID) {
    case 0:
      return false;                                             //Invalid ValueID
      break;
    case 1:
      Value.toCharArray(ssid, Value.length() + 1);
      break;
    case 2:
      for (byte i = 0; i < String(Value).length(); i++) {
        if (Value.charAt(i) != '*') {                           //if the password is set (and not just the '*****' we have given the client)
          Value.toCharArray(password, Value.length() + 1);
          return true;                                          //Stop for loop
        }
      }
      return false;                                             //Not set, the password was just '*****'
      break;
#ifdef WiFiManagerUser_Set_Value_Defined
    default:
      return WiFiManagerUser_Set_Value(ValueID - 3, Value);
      break;
#endif //WiFiManagerUser_Set_Value_Defined
  }
  return true;
}
String CWiFiManager::Get_Value(byte ValueID, bool Safe, bool Convert) {
  //Safe == true will return the real password,
  //From RAM to EEPROM
#ifdef WiFiManager_SerialEnabled
  Serial.print("WM: Get current value of: " + String(ValueID) + " safe=" + String(Safe) + " conv=" + String(Convert));
#endif //WiFiManager_SerialEnabled
  String Return_Value = "";                                     //Make sure to return something, if we return bad data of NULL, the HTML page will break
  switch (ValueID) {
    case 0:
      return "";                                                //Invalid ValueID
      break;
    case 1:
      Return_Value += String(ssid);
      break;
    case 2:
      if (Safe)                                                 //If's it's safe to return password.
        Return_Value += String(password);
      else {
        for (byte i = 0; i < String(password).length(); i++)
          Return_Value += "*";
      }
      break;
#ifdef WiFiManagerUser_Get_Value_Defined
    default:
      Return_Value = WiFiManagerUser_Get_Value(ValueID - 3, Safe, Convert);
      break;
#endif //WiFiManagerUser_Get_Value_Defined
  }
#ifdef WiFiManager_SerialEnabled
  Serial.println(" = " + Return_Value);
#endif //WiFiManager_SerialEnabled
  if (!Safe){
    Return_Value.replace("\"", "&quot;");                         //Make sure to change char("), since we can't use that, change to char(')
    Return_Value.replace(String(EEPROM_Seperator), " ");          //Make sure to change the EEPROM seperator, since we can't use that
  }
  return String(Return_Value);
}
void CWiFiManager::Status_Start() {
#ifdef WiFiManagerUser_Status_Start_Defined
  WiFiManagerUser_Status_Start();
#endif //WiFiManagerUser_Status_Start_Defined
}
void CWiFiManager::Status_Done() {
#ifdef WiFiManagerUser_Status_Done_Defined
  WiFiManagerUser_Status_Done();
#endif //WiFiManagerUser_Status_Done_Defined
}
void CWiFiManager::Status_Blink() {
#ifdef WiFiManagerUser_Status_Blink_Defined
  WiFiManagerUser_Status_Blink();
#endif //WiFiManagerUser_Status_Blink_Defined
}
void CWiFiManager::Status_StartAP() {
#ifdef WiFiManagerUser_Status_StartAP_Defined
  WiFiManagerUser_Status_StartAP();
#endif //WiFiManagerUser_Status_StartAP_Defined
}
bool CWiFiManager::HandleAP() {
#ifdef WiFiManagerUser_HandleAP_Defined
  return WiFiManagerUser_HandleAP();
#else
  return false;
#endif //WiFiManagerUser_HandleAP_Defined
}
//===========================================================================
//End of user variables calls
//===========================================================================
//Just for debugging
#ifdef WiFiManager_SerialEnabled
String ConvertWifistatus(byte IN) {
  switch (IN) {
    case WL_CONNECTED:        return "WL_CONNECTED";          break;
    case WL_NO_SHIELD:        return "WL_NO_SHIELD";          break;
    case WL_IDLE_STATUS:      return "WL_IDLE_STATUS";        break;
    case WL_NO_SSID_AVAIL:    return "WL_NO_SSID_AVAILABLE";  break;
    case WL_SCAN_COMPLETED:   return "WL_SCAN_COMPLETED";     break;
    case WL_CONNECT_FAILED:   return "WL_CONNECT_FAILED";     break;
    case WL_CONNECTION_LOST:  return "WL_CONNECTION_LOST";    break;
    case WL_DISCONNECTED:     return "WL_DISCONNECTED";       break;
  }
  return "UNKNOWN";
}
#endif //WiFiManager_SerialEnabled
//===========================================================================
//ISR must return nothing and take no arguments, so we need this sh*t
void WiFiManager_handle_Connect() {
  WiFiManager.handle_Connect();
}
void WiFiManager_handle_Settings() {
  WiFiManager.handle_Settings();
}
#ifdef WiFiManager_OTA
void WiFiManager_OTA_handle_uploadPage() {
  WiFiManager.handle_uploadPage();
}
void WiFiManager_OTA_handle_update() {
  WiFiManager.handle_update();
}
void WiFiManager_OTA_handle_update2() {
  WiFiManager.handle_update2();
}
#endif //WiFiManager_OTA
//===========================================================================
#ifdef WiFiManager_Restart
void handle_Restart() {
#ifdef WiFiManager_SerialEnabled
  Serial.println("WM: handle_Restart");
#endif //WiFiManager_SerialEnabled
  server.send(200, "text/html", "OK, restarting...");
  for (byte i = 50; i > 0; i--) {                               //Add some delay here to send feedback to the client, i is delay in MS
    server.handleClient();
    delay(1);
  }
  ESP.restart();
}
#endif //WiFiManager_Restart
void CWiFiManager::StartServer() {
  //Should not be called when it isn't connected or hosting
  static bool ServerStarted = false;
  if (ServerStarted) return;                                    //If the server is already started, stop here
  ServerStarted = true;
  server.on("/",          WiFiManager_handle_Connect);
  server.on("/ip",        WiFiManager_handle_Connect);          //Just as backup, so the "/" can be overwritten by user
  server.on("/setup",     WiFiManager_handle_Settings);
#ifdef WiFiManager_Restart
  server.on("/restart",   handle_Restart);
#endif //WiFiManager_Restart
#ifdef WiFiManager_OTA
  server.on("/ota",       WiFiManager_OTA_handle_uploadPage);
  server.on("/update",    HTTP_POST, WiFiManager_OTA_handle_update, WiFiManager_OTA_handle_update2);
#endif //WiFiManager_OTA
  server.begin();                                               //Begin server
}
void CWiFiManager::EnableSetup(bool State) {
#ifdef WiFiManager_SerialEnabled
  if (State) {
	Serial.print("WM: Settings page online, ip=");
    if (WiFi.status() == WL_CONNECTED)
      Serial.println(WiFi.localIP());
    else
      Serial.println(WiFi.softAPIP());
  } else
    Serial.println("WM: Settings page offline");
#endif //WiFiManager_SerialEnabled
  SettingsEnabled = State;
}
bool CWiFiManager::TickEveryMS(int _Delay) {
  static unsigned long _LastTime = 0;                           //Make it so it returns 1 if called for the FIST time
  if (millis() > _LastTime + _Delay) {
    _LastTime = millis();
    return true;
  }
  return false;
}
byte CWiFiManager::APMode() {
  //IP of AP = 192.168.4.1
  /* Returns:
    1= Done, Client submitted new data though AP portal
    2= Soft-AP setup Failed
    3= Manual leaving AP mode (User code has requested it)
  */
  WiFi.mode(WIFI_AP_STA);                               //https://github.com/espressif/arduino-esp32/blob/1287c529330d0c11853b9f23ddf254e4a0bc9aaf/libraries/WiFi/src/WiFiType.h#L33
  if (!WiFi.softAP(Name))                                       //config doesn't seem to work, so do not use it: 'WiFi.softAPConfig(ap_local_IP, ap_gateway, ap_subnet)'
    return 2;
  Status_StartAP();
  EnableSetup(true);                                            //Flag we need to responce to settings commands
  StartServer();                                                //Start server (if we havn't already)
#ifdef WiFiManager_DNS
# ifndef WiFiManager_SerialEnabled
  dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));          //Start a DNS server at the default DNS port, and send ALL trafic to it OWN IP (DNS_port, DNS_domainName, DNS_resolvedIP)
# else
  if (dnsServer.start(53, "*", IPAddress(192, 168, 4, 1)))
    Serial.println("WM: DNS server started");
  else
    Serial.println("WM: DNS server failed to start");
# endif //WiFiManager_SerialEnabled
#endif //WiFiManager_DNS
#ifdef WiFiManager_SerialEnabled
  Serial.print("WM: APMode on; SSID=" + String(Name) + " ip=");
  Serial.println(WiFi.softAPIP());
#endif //WiFiManager_SerialEnabled
  while (WaitOnAPMode) {
    if (TickEveryMS(100)) Status_Blink();                       //Let the LED blink to show we are not connected
    server.handleClient();
#ifdef WiFiManager_DNS
    dnsServer.processNextRequest();
#endif //WiFiManager_DNS
    if (HandleAP()) {
#ifdef SerialEnabled
      Serial.println("WM: Manual leaving APMode");
#endif //SerialEnabled
      EnableSetup(false);                                       //Flag to stop responce to settings commands
#ifdef WiFiManager_DNS
      dnsServer.stop();
#endif //WiFiManager_DNS
      return 3;
    }
  }
#ifdef WiFiManager_DNS
  dnsServer.stop();
#endif //WiFiManager_DNS
#ifdef WiFiManager_SerialEnabled
  Serial.println("WM: Leaving APmode");
#endif //WiFiManager_SerialEnabled
  WaitOnAPMode = true;                                          //reset flag for next time
  EnableSetup(false);                                           //Flag to stop responce to settings commands
  return 1;
}
String CWiFiManager::LoadEEPROM() {
  String Value;
#ifdef WiFiManager_SerialEnabled
  Serial.print("WM: EEPROM LOAD");
#endif //WiFiManager_SerialEnabled
  for (int i = 0; i < EEPROM_size; i++) {
    byte Input = EEPROM.read(i);
    if (Input == 255) {                                         //If at the end of data
#ifdef WiFiManager_SerialEnabled
      Serial.println();
#endif //WiFiManager_SerialEnabled
      EEPROM_USED = Value.length();
      return Value;                                             //Stop and return all data stored
    }
    if (Input == 0) {                                           //If no data found (NULL)
      EEPROM_USED = Value.length();
      return String(EEPROM_Seperator);
    }
    Value += char(Input);
#ifdef WiFiManager_SerialEnabled
    Serial.print("_" + String(char(Input)) + "_");
#endif //WiFiManager_SerialEnabled
  }
#ifdef WiFiManager_SerialEnabled
  Serial.println();
#endif //WiFiManager_SerialEnabled
  EEPROM_USED = Value.length();
  return String(EEPROM_Seperator);                              //ERROR; [maybe] not enough space
}
byte CWiFiManager::LoadData() {
  if (!EEPROM.begin(EEPROM_size))
    return 2;
  String Value = LoadEEPROM();
#ifdef WiFiManager_SerialEnabled
  Serial.println("WM: EEPROM data=" + Value);
#endif //WiFiManager_SerialEnabled
  if (Value != String(EEPROM_Seperator)) {                      //If there is data in EEPROM
    for (byte i = 1; i < WiFiManager_Settings + 1; i++) {
      byte j = Value.indexOf(char(EEPROM_Seperator));
      if (j == 255)
        j = Value.length();
      String _Value = Value.substring(0, j);
      if (_Value != "")                                         //If there is a value
        Set_Value(i, _Value);                                   //set the value in memory (and thus overwrite the Hardcoded stuff)
      Value = Value.substring(j + 1);
    }
  }
  return 1;
}
bool CWiFiManager::Connect(int TimeOutMS) {
  if ((strlen(ssid) == 0 or strlen(password) == 0))             //If no SSID or password given
    return false;
#ifdef WiFiManager_SerialEnabled
  Serial.println("WM: Connecting to ssid='" + String(ssid) + "' password='" + String(password) + "'");
#endif //WiFiManager_SerialEnabled
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
#if defined(strip_ip) && defined(gateway_ip) && defined(subnet_mask)
  WiFi.config(strip_ip, gateway_ip, subnet_mask);
#endif
  unsigned long StopTime = millis() + TimeOutMS;
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() > StopTime) {                                  //If we are in overtime
#ifdef WiFiManager_SerialEnabled
      Serial.println("WM: Could not connect within " + String(TimeOutMS) + "ms to given SSID, aborting with code " + ConvertWifistatus(WiFi.status()));
#endif //WiFiManager_SerialEnabled
      return false;
    }
    if (TickEveryMS(500)) Status_Blink();                       //Let the LED blink to show we are trying to connect
  }
  return true;
}
byte CWiFiManager::Start() {
  //starts wifi stuff, only returns 1/TRUE when Connected. Will start AP setup if needed
  /* Returns:
     1= Done or already connected
     2= EEPROM error (Can't begin EEPROM problably)
     3= Apmode was unsuccessful
  */
  if (WiFi.status() == WL_CONNECTED) return 1;                  //If WIFI already on, stop and return true
  Status_Start();
  if (ssid[0] == 0 and password[0] == 0)                        //If the ssid and password are not yet in memory
    if (LoadData() != 1) return 2;                              //Load the EEPROM to get the ssid and password. Exit with code if failed
  while (WiFi.status() != WL_CONNECTED) {
    if (!Connect(ConnectionTimeOutMS))                          //Try to connected to ssid+password
      if (APMode() != 1) return 3;                              //(If we could not connector for whatever reason), Entering APmode and if that fails return the error
  }
#ifdef WiFiManager_mDNS                                         //https://github.com/espressif/arduino-esp32/blob/master/libraries/ESPmDNS/src/ESPmDNS.cpp
  bool MDNSStatus = MDNS.begin(Name);                           //Start mDNS with the given domain name
  if (MDNSStatus) MDNS.addService("http", "tcp", 80);           //Add service to MDNS-SD
#ifdef WiFiManager_SerialEnabled
  if (MDNSStatus)
    Serial.println("WM: mDNS responder started with name '" + String(Name) + "'");
  else
    Serial.println("WM: ERROR setting up mDNS responder! with name '" + String(Name) + "'");
#endif //WiFiManager_SerialEnabled
#endif //WiFiManager_mDNS
#ifdef WiFiManager_SerialEnabled
  Serial.print("WM: Connected; SSID=" + String(ssid) + " ip=");
  Serial.println(WiFi.localIP());
#endif //WiFiManager_SerialEnabled
  Status_Done();
  return 1;
}
bool CWiFiManager::WriteEEPROM() {
  String Value;                                                 //Save to mem:
  for (byte i = 0; i < WiFiManager_Settings; i++) {
    Value += Get_Value(i + 1, true, false);                     //^     <Seperator>
    if (WiFiManager_Settings - i > 1)
      Value += EEPROM_Seperator;                                //^            <Value>  (only if there more values)
  }
  Value += char(255);                                           //^            <emthy bit> (we use a emthy bit to mark the end)
#ifdef WiFiManager_SerialEnabled
  Serial.println("WM: EEPROM WRITE; '" + Value + "'");
#endif //WiFiManager_SerialEnabled                              
  if (Value.length() > EEPROM_size)                             //If not enough room in the EEPROM
    return false;                                               //Return false; not all data is stored
  for (int i = 0; i < Value.length(); i++)                      //For each character to save
    EEPROM.write(i, (int)Value.charAt(i));                      //Write it to the EEPROM
  EEPROM.commit();
  EEPROM_USED = Value.length();
  return true;
}
bool CWiFiManager::ClearEEPROM() {
#ifdef WiFiManager_SerialEnabled
  Serial.println("WM: EEPROM CLEAR");
#endif //WiFiManager_SerialEnabled                              
  if (!EEPROM.begin(EEPROM_size))
    return false;
  for (int i = 0; i < EEPROM_size; i++)                         //For each character to save
    EEPROM.write(i, 0);                                         //Clear the EEPROM bit
  EEPROM.commit();
  return true;
}
void CWiFiManager::RunServer() {
  if (WiFi.status() != WL_CONNECTED) return;
  StartServer();                                                //Start server if we havn't yet
  server.handleClient();
}
void CWiFiManager::handle_Connect() {
  if (!SettingsEnabled) return;                                 //If settingscommand is disabled: Stop right away, and do noting
  String HTML = "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, viewport-fit=cover\"><strong>" + String(Name) + " settings</strong><br><br><form action=\"/setup?\" method=\"post\">";
  for (byte i = 1; i < WiFiManager_Settings + 1; i++)
    HTML += "<div><label>" + WiFiManager_VariableNames[i - 1] + " </label><input type=\"text\" name=\"" + i + "\" value=\"" + Get_Value(i, false, true) + "\"></div>";
  HTML += "<button>Send</button></form>"
#ifdef WiFiManager_OTA
          "<form action=\"/ota\"><button>OTA page</button></form>"
#endif //WiFiManager_OTA
          "" + String(EEPROM_USED) + "/" + String(EEPROM_size) + " Bytes used<br>"
          "MAC adress = " +  String(WiFi.macAddress());
  server.send(200, "text/html", HTML);
}
void CWiFiManager::handle_Settings() {
  if (!SettingsEnabled) return;                                 //If settingscommand is disabled: Stop right away, and do noting
  String HTML = "";
  int    Code = 200;
  for (int i = 0; i < server.args(); i++) {
    int j = server.argName(i).toInt();
    String ArgValue = server.arg(i);
    if (j > 0 and j < 255 and ArgValue != "") {
	  ArgValue.trim();
      if (Set_Value(j, ArgValue))
        HTML += "Succesfull '" + String(j) + "'='" + ArgValue + "'\n";
      else
        HTML += "ERROR Set; '" + String(j) + "'='" + ArgValue + "'\n";
    } else {
      Code = 422;   //Flag we had a error
      HTML += "ERROR ID; '" + server.argName(i) + "'='" + ArgValue + "'\n";
    }
  }
  WaitOnAPMode = false;                                         //Flag we have input data, and we can stop waiting in APmode on data
  WriteEEPROM();
  HTML += String(EEPROM_USED) + "/" + String(EEPROM_size) + " Bytes used";
  server.send(Code, "text/plain", HTML);
  for (byte i = 50; i > 0; i--) {                               //Add some delay here, to send feedback to the client, i is delay in MS to still wait
    server.handleClient();
    delay(1);
  }
  static String OldSSID = ssid;
  static String Oldpassword = password;
  if (OldSSID != String(ssid) or Oldpassword != String(password)) {
#ifdef WiFiManager_SerialEnabled
    Serial.println("WM: Auto disconnect, new SSID recieved, from " + OldSSID + " to " + String(ssid));
#endif //WiFiManager_SerialEnabled
    OldSSID = String(ssid);
    Oldpassword = String(password);
    WiFi.disconnect();                                          //we need to reconnect (user requested to switch WIFI)
  }
}
bool CWiFiManager::CheckAndReconnectIfNeeded(bool AllowAPmode) {
  if (WiFi.status() != WL_CONNECTED) {
    if (AllowAPmode)
      Start();
    else
      Connect(ConnectionTimeOutMS);
    if (WiFi.status() != WL_CONNECTED)                          //If still not connected
      return false;
  }
  return true;
}
#ifdef WiFiManager_OTA
void CWiFiManager::handle_uploadPage() {
#ifdef WiFiManager_SerialEnabled
  Serial.println("OTA_handle_UploadPage, enabled=" + OTA_Enabled ? "TRUE" : "FALSE");
#endif //WiFiManager_SerialEnabled
  if (!OTA_Enabled) return;                                     //If OTA is disabled, stop here and do not respond
  String html = "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, viewport-fit=cover\"><script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script><a href=\"" + UpdateWebpage + "\">" + UpdateWebpage + "</a><br>Code compiled on " + String(__DATE__) + " " + String(__TIME__) + "<br><br><form method='POST' action='#' enctype='multipart/form-data' id='upload_form'><input type='file' name='update'><input type='submit' value='Upload'></form><div id='prg'>progress: 0%</div><script>$('form').submit(function(c){c.preventDefault();var a=$('#upload_form')[0];var b=new FormData(a);$.ajax({url:'/update',type:'POST',data:b,contentType:false,processData:false,xhr:function(){var d=new window.XMLHttpRequest();d.upload.addEventListener('progress',function(e){if(e.lengthComputable){var f=e.loaded/e.total;$('#prg').html('progress: '+Math.round(f*100)+'%')}},false);return d},success:function(f,e){console.log('success!')},error:function(e,d,f){}})});</script>";
  server.send(200, "text/html", html);
}
void CWiFiManager::handle_update() {
#ifdef WiFiManager_SerialEnabled
  Serial.printf("WM_OTA: Update, enabled=" + OTA_Enabled ? "TRUE" : "FALSE");
#endif //WiFiManager_SerialEnabled
  if (!OTA_Enabled) return;                                     //If OTA is disabled, stop here and do not respond
  server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
  ESP.restart();
}
void CWiFiManager::handle_update2() {
  if (!OTA_Enabled) return;                                     //If OTA is disabled, stop here and do not respond
  HTTPUpload& upload = server.upload();
#ifdef WiFiManager_SerialEnabled
  if (!OTA_Enabled) return;                                     //If OTA is disabled, stop here and do not respond
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {                   //start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {              //flashing firmware to ESP
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
      Update.printError(Serial);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true))                                       //true to set the size to the current progress
      Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
    else
      Update.printError(Serial);
  }
#else
  if (upload.status == UPLOAD_FILE_START)
    Update.begin(UPDATE_SIZE_UNKNOWN);                          //start with max available size
  else if (upload.status == UPLOAD_FILE_WRITE)                  //flashing firmware to ESP
    Update.write(upload.buf, upload.currentSize);
  else if (upload.status == UPLOAD_FILE_END)
    Update.end(true);
#endif //WiFiManager_SerialEnabled
}
#endif //WiFiManager_OTA
#ifdef WiFiManager_DoRequest
byte CWiFiManager::DoRequest(char _IP[16], int _Port, String _Path, String _Json, byte TimeOut, String _Header) {
  /* Returns:
    0= Unknown error (responce out of range)  REQ_UNK
    1= Done (responce code 200)               REQ_SUCCES
    2= Cant connect to HUB                    REQ_HUB_CONCECT_ERROR
    3= Timeout recieving responce code        REQ_TIMEOUT
    4= Page not found (responce code 404)     REQ_PAGE_NOT_FOUND
    5= Manual WiFi settup required            REQ_SETUP_REQUIRED
    10-19= Unknown server error (first digit Responce=X-10, for example 4 means an 4## client error (like 403 Forbidden))
  */
#ifdef WiFiManager_SerialEnabled
  Serial.println("WM_REQ: DO REQUEST: " + String(_IP) + ":" + _Port + _Path + " _ExtraHeader=" + _Header + "_Data=" + _Json);
#endif //WiFiManager_SerialEnabled

  if (!WiFiManager.CheckAndReconnectIfNeeded(false))
    return REQ_SETUP_REQUIRED;                                  //Exit here, no connection, and could not auto connect
  WiFiClient client;
  client.setTimeout(TimeOut);
  if (!client.connect(_IP, _Port))
    return REQ_HUB_CONNECT_ERROR;                               //Stop here, no reason to move on
  client.println("PUT " + _Path + " HTTP/1.1");
  client.println("Content-Length: " + String(_Json.length()));
  client.println("Content-Type: application/json");
  if (_Header != ""){
	client.println(_Header);
  }
  client.println();                                             //Terminate headers with a blank line
  client.print(_Json);
  //Try to look for a responce code 'HTTP/1.1 200 OK' = 200
  int Responcecode  = 0;
  unsigned long StopTime = millis() + 2500;                     //After this amount of time stop waiting for a response, 500ms could be considered a normal response time
  static unsigned long _Middle = -1;                            //We just need a really big number, if more than 0 and less than this amount of ms is passed, return true)
  _Middle = _Middle / 2;                                        //Somehow declairing middle on 1 line does not work
  while (client.connected()) {
    while (client.available()) {
      byte recieved = client.read();
      if (recieved == 0x20) {                                   //If "HTTP/1.1" is paste and we now have a SPACE
        recieved = client.read();                               //Purge space
        while (recieved != 0x20) {                              //While we read numbers and not a SPACE
          Responcecode = Responcecode * 10 + (recieved - 0x30); //Convert byte to number and put it in
          recieved = client.read();                             //Read new byte
        }
        client.stop();                                          //Stop, we already have the Responce code
      }
    }
    if (millis() - StopTime <= _Middle) {                       //If we are in the timout windown, includes overflow (must be withing the _Middle window)
      client.stop();                                            //Stop waiting on not responding client
      return REQ_TIMEOUT;
    }
  }
  if (Responcecode == 200)
    return REQ_SUCCES;
  if (Responcecode == 404)
    return REQ_PAGE_NOT_FOUND;
  if (Responcecode < 1000)
    return 10 + floor(Responcecode / 100);
  return REQ_UNK;
}
#endif //WiFiManager_DoRequest