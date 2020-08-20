/* Written by JelleWho https://github.com/jellewie
   HTML written by JesperTheEnd https://github.com/jespertheend
*/

//#define Server_SerialEnabled      //define this to enable serial feedback

//<ip>/set?PreFix=Value[&....]      //These are currently HARDCODED into the HTML page, so shouldn't be changed if you want to use the webpage
#define PreFixSetColorValueRed "r"
#define PreFixSetColorValueGreen "g"
#define PreFixSetColorValueBlue "b"
#define PreFixSetBrightness "l"
#define PreFixSetAutoBrightness "i"
#define PreFixSetAutoBrightnessN "n"
#define PreFixSetAutoBrightnessP "p"
#define PreFixSetModeTo "m"
#define PreFixSetBootMode "bm"
#define PreFixSetDoHourlyAnimation "ha"
#define PreFixSetDoublePressMode "dm"
#define PreFixSetClockHourLines "hl"
#define PreFixSetClockHourAnalog "a"
#define PreFixSetClockOffset "o"
#define PreFixSetClockAnalog "c"

//<ip>/time[?PreFix=Value][&....]     //These are currently HARDCODED into the HTML page, so shouldn't be changed if you want to use the webpage
#define PreFixTimeHour "h"
#define PreFixTimeMin  "m"
#define PreFixTimeSec  "s"

#define EEPROMSaveDelayMS 30000

void handle_Set() {
  String ERRORMSG;                //emthy=dont change
  bool DoWriteToEEPROM = false;
  int NewR = -1, NewG = -1, NewB = -1;
#ifdef Server_SerialEnabled
  Serial.print("SV: /SET?");
#endif //Server_SerialEnabled
  for (int i = 0; i < server.args(); i++) {
    String ArguName = server.argName(i);
    ArguName.toLowerCase();
    String ArgValue = server.arg(i);
    ArgValue.toLowerCase();
#ifdef Server_SerialEnabled
    Serial.print(" " + ArguName + "=" + ArgValue);
#endif //Server_SerialEnabled
    if (ArguName == PreFixSetColorValueRed) {
      NewR = constrain((ArgValue.toInt()), 0, 255);
    } else if (ArguName == PreFixSetColorValueGreen) {
      NewG = constrain((ArgValue.toInt()), 0, 255);
    } else if (ArguName == PreFixSetColorValueBlue) {
      NewB = constrain((ArgValue.toInt()), 0, 255);
    } else if (ArguName == PreFixSetModeTo) {
      LastMode = -1;                                   //Make sure we init the new mode
      Mode = ConvertModeToInt(ArgValue);
    } else if (ArguName == PreFixSetBrightness) {
      if (Mode == ON) Mode = WIFI;      //If we are on manual, switch to WIFI
      AutoBrightness = false;
      FastLED.setBrightness(constrain((ArgValue.toInt()), 1, 255));
    } else if (ArguName == PreFixSetAutoBrightness) {
      AutoBrightness = IsTrue(ArgValue);
      UpdateBrightness(true);
      DoWriteToEEPROM = true;
    } else if (ArguName == PreFixSetAutoBrightnessN) {
      AutoBrightnessN = constrain((ArgValue.toInt()), 1, 255);
      DoWriteToEEPROM = true;
    } else if (ArguName == PreFixSetAutoBrightnessP) {
      AutoBrightnessP = constrain((ArgValue.toInt()), 1, 255);
      DoWriteToEEPROM = true;
    } else if (ArguName == PreFixSetBootMode) {
      BootMode = ConvertModeToInt(ArgValue);
      DoWriteToEEPROM = true;
    } else if (ArguName == PreFixSetDoHourlyAnimation) {
      DoHourlyAnimation = IsTrue(ArgValue);
      DoWriteToEEPROM = true;
    } else if (ArguName == PreFixSetDoublePressMode) {
      DoublePressMode = ConvertModeToInt(ArgValue);
      DoWriteToEEPROM = true;
    } else if (ArguName == PreFixSetClockHourLines) {
      ClockHourLines = constrain((ArgValue.toInt()), 0, 255);
      DoWriteToEEPROM = true;
    } else if (ArguName == PreFixSetClockHourAnalog) {
      ClockHourAnalog = IsTrue(ArgValue);
      DoWriteToEEPROM = true;
    } else if (ArguName == PreFixSetClockOffset) {
      ClockOffset = constrain((ArgValue.toInt()), 0, TotalLEDs);
      DoWriteToEEPROM = true;
    } else if (ArguName == PreFixSetClockAnalog) {
      ClockAnalog = IsTrue(ArgValue);
      DoWriteToEEPROM = true;
    } else
      ERRORMSG += "Unknown arg '" + ArguName + "' with value '" + ArgValue + "'\n";
  }
#ifdef Server_SerialEnabled
  Serial.println();
#endif //Server_SerialEnabled

  if (Mode == WIFI) AnimationCounter = 0;
  if (AnimationCounter != 0) {            //Animation needs to be shown
    if (NewR != -1) RGBColor[0] = NewR;   //Set animation color
    if (NewG != -1) RGBColor[1] = NewG;
    if (NewB != -1) RGBColor[2] = NewB;
  } else {
    if (NewR != -1) {
      Mode = WIFI;
      LEDs[0].r = NewR;
    }
    if (NewG != -1) {
      Mode = WIFI;
      LEDs[0].g = NewG;
    }
    if (NewB != -1) {
      Mode = WIFI;
      LEDs[0].b = NewB;
    }
  }
  if (DoWriteToEEPROM) {                //If we need to write to EEPROM
    RemoveTasksByID(SAVEEEPROM);        //Remove old EEPROM write command if they exist
    TASK TempTask;
    TempTask.ID = SAVEEEPROM;           //Create a new EEPROM write command
    TempTask.ExectuteAt.Ticks = millis() + EEPROMSaveDelayMS; //Schedule to write data to EEPROM
    AddTask(TempTask);                  //Add the command to the task list
  };
  if (Mode == WIFI)
    fill_solid(&(LEDs[0]), TotalLEDs, LEDs[0]);   //Change the whole LED strip to have the color of the first set LED
  else if (Mode == RESET) {
    server.send(200, "text/plain", "OK");
    for (int i = 0; i < 100; i++) {      //Just wait for a few ms to make sure the "reset command recieved" has been send
      server.handleClient();
      delay(1);
    }
    ESP.restart();                                //Restart the ESP
  }
  if (ERRORMSG == "") {
    UpdateLEDs = true;
    loopLEDS();
    handle_Getcolors();
  } else {
    server.send(400, "text/plain", ERRORMSG);
#ifdef Server_SerialEnabled
    Serial.println("SV: 400 Set " + ERRORMSG);
#endif //Server_SerialEnabled
  }
}
void handle_Getcolors() {
  String ans = "{\"m\":\"" + ConvertModeToString(Mode) + "\","
               "\"bm\":\"" + ConvertModeToString(BootMode) + "\","
               "\"dm\":\"" + ConvertModeToString(DoublePressMode) + "\","
               "\"i\":\"" + IsTrueToString(AutoBrightness) + "\","
               "\"ha\":\"" + IsTrueToString(DoHourlyAnimation) + "\","
               "\"hl\":\"" + ClockHourLines + "\","
               "\"a\":\"" + IsTrueToString(ClockHourAnalog) + "\","
               "\"c\":\"" + IsTrueToString(ClockAnalog) + "\",";

  byte r = LEDs[0].r, g = LEDs[0].g, b = LEDs[0].b;
  if (AnimationCounter != 0) {  //Animation needs to be shown (this is used to show animation color, instead of mostly black)
    r = RGBColor[0];
    g = RGBColor[1];
    b = RGBColor[2];
  }

  ans += "\"RGBL\":[{\"R\":" + String(r) + ",\"G\":" + String(g) + ",\"B\":" + String(b) + ",\"L\":" + String(FastLED.getBrightness()) + "}]}";

  server.send(200, "application/json", ans);
#ifdef Server_SerialEnabled
  Serial.println("SV: 200 Getcolors " + ans);
#endif //Server_SerialEnabled
}
void handle_OnConnect() {
  if (!WiFiManager_connected) {
    WiFiManager_handle_Connect();       //Since we have no internet/WIFI connection, handle request as an APmode request
    return;
  }
  /*HTML USEFULL STEPS:
    Compress the code (line enter and spaces) https://htmlcompressor.com/compressor/ https://www.textfixer.com/html/compress-html-compression.php (This can save like 66% of the bytes!)
    Replace " with \"
    static String html = "<code>";
  */
  static String html = "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no, viewport-fit=cover\"><link rel=\"manifest\" href=\"manifest.json\"><style>body,"
                       "html{font-family:Helvetica,Arial,sans-serif;margin:0;background:var(--selected-color);color:var(--ui-color)}"
                       "#sliders{width:80%;max-width:400px;top:50%;left:50%;position:absolute;transform:translate(-50%,-50%)}"
                       ".sliderContainer{margin:30px 0;text-align:center}"
                       ".slideIn{position:absolute;border-radius:5px;box-shadow:2px 2px var(--ui-color-trans2);transition:opacity .2s,transform .2s;background:var(--selected-color);border:1px solid var(--ui-color)}"
                       ".toast{z-index:160;right:0;top:0;font-size:14pt;margin:30px;padding:10px}"
                       ".toast.error{background:#ff5026}.toast.hidden{transform:translateY(-30%);opacity:0}"
                       "input[type=\"range\"]{display:block;width:100%;margin:0;-webkit-appearance:none;background:transparent}"
                       "input[type=\"range\"]:focus{outline:0}"
                       "input[type=\"range\"]::-webkit-slider-runnable-track{background:var(--ui-color-trans2);height:5px;margin:30px 0;border-radius:5px}"
                       "@media(max-height:540px){input[type=\"range\"]::-webkit-slider-runnable-track{margin:10px 0}}"
                       "input[type=\"range\"]:hover::-webkit-slider-runnable-track{background:var(--ui-color-trans1)}"
                       "input[type=\"range\"]::-webkit-slider-thumb{-webkit-appearance:none;background:var(--ui-color);width:10px;height:10px;border-radius:30px;margin-top:-3px;transition:width .2s,height .2s,margin-top .2s}"
                       "input[type=\"range\"]:hover::-webkit-slider-thumb{width:30px;height:30px;margin-top:-12.5px}"
                       "#curtain{position:absolute;left:0;right:0;top:0;bottom:0;z-index:100;background:#00000082;opacity:1;transition:opacity .2s}"
                       "#curtain.hidden{opacity:0;pointer-events:none}"
                       "#settingsContainer{position:absolute;width:90%;left:50%;top:50%;transform:translate(-50%,-50%);text-align:center;z-index:150}"
                       "#settingsContainer.hidden{transform:translate(-50%,calc( - 50% - 10px));opacity:0;pointer-events:none}"
                       "#settingsBtn{bottom:0;right:0;position:absolute;background:url(settings.svg);width:30px;height:30px;margin:10px;cursor:pointer}"
                       ".selectLabel{margin:5px;display:block}"
                       ".selectContainer{display:inline-block;position:relative}"
                       "select{-webkit-appearance:none;background:0;color:var(--ui-color);border:solid 1px var(--ui-color);border-radius:5px;margin:3px;padding:2px}"
                       "select:focus{outline:0}"
                       "select,.dropdownFakeValue{font-size:11px}"
                       ".dropdownFakeValue{display:inline-block;position:absolute;left:50%;top:50%;transform:translate(-50%,-50%);pointer-events:none}"
                       "option{color:initial}"
                       "button{-webkit-appearance:none;background:var(--ui-color);border:0;padding:3px 5px;margin:3px;border-radius:5px;color:var(--selected-color)}"
                       "</style><style id=\"colorVars\"></style></head><body>"
                       "<div id=\"sliders\"></div>"
                       "<div id=\"curtain\" class=\"hidden\"></div>"
                       "<div id=\"settingsContainer\" class=\"slideIn hidden\"></div>"
                       "<svg viewBox=\"0 0 50 50\" xmlns=\"http://www.w3.org/2000/svg\" id=\"settingsBtn\"><defs><mask id=\"mask\"><g><path d=\"M10 31.5 L2 29 L2 21 L10 18.5 L6 11 L11 6 L18.5 10 L21 2 L29 2 L31.5 10 L39 6 L44 11 L40 18.5 L48 21 L48 29 L40 31.5 L44 39 L39 44 L31.5 40 L29 48 L21 48 L18.5 40 L11 44 L6 39\" fill=\"white\"/><circle cx=\"25\" cy=\"25\" r=\"18\" fill=\"white\"/><circle cx=\"25\" cy=\"25\" r=\"10\" fill=\"black\"/></g></mask></defs><rect x=\"0\" y=\"0\" width=\"100\" height=\"100\" mask=\"url(#mask)\" style=\"fill:var(--ui-color)\"/></svg>"
                       "<script>let slidersEl = document.getElementById(\"sliders\");"
                       "let curtainEl = document.getElementById(\"curtain\");"
                       "let settingsContainerEl = document.getElementById(\"settingsContainer\");"
                       "let settingsBtn = document.getElementById(\"settingsBtn\");"
                       "let colorVarsEl = document.getElementById(\"colorVars\");"
                       "class Slider{constructor(name, forceRgb = true){this.name = name;this.containerEl = document.createElement(\"div\");this.containerEl.classList.add(\"sliderContainer\");slidersEl.appendChild(this.containerEl);this.textEl = document.createElement(\"div\");this.textEl.classList.add(\"sliderText\");this.containerEl.appendChild(this.textEl);this.rangeEl = document.createElement(\"input\");this.rangeEl.type = \"range\";this.rangeEl.min = \"0\";this.rangeEl.max = \"255\";this.containerEl.appendChild(this.rangeEl);this.rangeEl.addEventListener(\"input\", (_) =>{this.update();});this.rangeEl.addEventListener(\"change\", (_) =>{setServerState(forceRgb);});this.updateText();}get value(){return this.rangeEl.value;}set value(v){this.rangeEl.value = v;this.update();}get value01(){return this.value / 255;}update(){this.updateText();updateCssColor();}updateText(){this.textEl.textContent = this.name + \": \" + this.value;}}"
                       "class DropDown{constructor({name=\"\",setParamName=\"\",possibleValues=[],modifySendParams=null}={}){let labelEl=document.createElement(\"label\");labelEl.classList.add(\"selectLabel\");labelEl.textContent=name+\":\";settingsContainerEl.appendChild(labelEl);this.name=name;this._value=\"\";this.setParamName=setParamName;this.modifySendParams=modifySendParams;let selectContainer=document.createElement(\"div\");selectContainer.classList.add(\"selectContainer\");labelEl.appendChild(selectContainer);this.el=document.createElement(\"select\");selectContainer.appendChild(this.el);this.el.addEventListener(\"change\",(_)=>{this.onChange();});for(const val of possibleValues){let optionEl=document.createElement(\"option\");optionEl.value=optionEl.textContent=val;this.el.appendChild(optionEl);}this.el.value=\"\";this.fakeValueEl=document.createElement(\"div\");this.fakeValueEl.classList.add(\"dropdownFakeValue\");selectContainer.appendChild(this.fakeValueEl);}async onChange(){this.value=this.el.value;this.el.value=\"\";let sendParams={};sendParams[this.setParamName]=this.value;if(this.modifySendParams){let newSendParams=this.modifySendParams(sendParams);if(newSendParams)sendParams=newSendParams;}let success=await sendServerState(sendParams);if(success){doToastMessage(this.name+\" Updated\");}}get value(){return this._value;}set value(v){this._value=v;this.fakeValueEl.textContent=v;}}"
                       "class Button{constructor(label,cb){this.el=document.createElement(\"button\");this.el.textContent=label;settingsContainer.appendChild(this.el);this.el.addEventListener(\"click\",(_)=>{cb();});}}"
                       "class RequestButton extends Button{constructor(label,endpoint,successMessage){super(label,async(_)=>{try{let response=await fetch(endpoint);if(response.ok){doToastMessage(successMessage);}else{doToastMessage(await response.body(),true);}}catch(_){doToastMessage(\"Failed to connect\",true);}});}}"
                       "class LinkButton extends Button{constructor(label,url){super(label,_=>{window.open(url,\"_blank\");});}}"

                       "let Sl=new Slider(\"Brightness\",false);"
                       "let Sr=new Slider(\"Red\");"
                       "let Sg=new Slider(\"Green\");"
                       "let Sb=new Slider(\"Blue\");"

                       "let Dm=new DropDown({name:\"Mode\",setParamName:\"m\",possibleValues:[\"OFF\",\"ON\",\"WIFI\",\"CLOCK\",\"BLINK\",\"BPM\",\"CONFETTI\",\"FLASH\",\"GLITTER\",\"JUGGLE\",\"MOVE\",\"RAINBOW\",\"SINELON\",\"SINELON2\"],modifySendParams:(oldParams)=>{if(Dm.value==\"WIFI\"){let extraData=this.getServerStateMessageData(true);return{...oldParams,...extraData};}},});"
                       "let Dbm=new DropDown({name:\"Bootmode\",setParamName:\"bm\",possibleValues:[\"OFF\",\"ON\",\"WIFI\",\"CLOCK\"]});"
                       "let Ddm=new DropDown({name:\"Doublepress mode\",setParamName:\"dm\",possibleValues:[\"WIFI\",\"CLOCK\",\"BLINK\",\"BPM\",\"CONFETTI\",\"FLASH\",\"GLITTER\",\"JUGGLE\",\"MOVE\",\"RAINBOW\",\"SINELON\",\"SINELON2\"]});"

                       "settingsContainer.appendChild(document.createElement(\"br\"));"
                       "let Di=new DropDown({name:\"Auto brightness\",setParamName:\"i\",possibleValues:[\"FALSE\",\"TRUE\"]});"

                       "settingsContainer.appendChild(document.createElement(\"br\"));"
                       "let Dha=new DropDown({name:\"Hourly animation\",setParamName:\"ha\",possibleValues:[\"FALSE\",\"TRUE\"]});"
                       "let Dhl=new DropDown({name:\"Hourly lines\",setParamName:\"hl\",possibleValues:[\"FALSE\",\"1\",\"2\",\"4\",\"8\",\"16\",\"32\"]});"        //This one is actually a Byte
                       "let Da=new DropDown({name:\"Analog hours\",setParamName:\"a\",possibleValues:[\"FALSE\",\"TRUE\"]});"
                       "let Dc=new DropDown({name:\"Analog clock\",setParamName:\"c\",possibleValues:[\"FALSE\",\"TRUE\"]});"

                       "settingsContainer.appendChild(document.createElement(\"br\"));"
                       "let Bo=new RequestButton(\"Enable OTA\",\"/ota\",\"OTA enabled\");"
                       "let Bt=new RequestButton(\"Sync time\",\"/time\",\"Time updated\");"
                       "let Br=new RequestButton(\"Reset\",\"/set?m=RESET\",\"ESP restarting\");"

                       "settingsContainer.appendChild(document.createElement(\"br\"));"
                       "new LinkButton(\"Info\",\"/info\");"
                       "new LinkButton(\"Taks\",\"/task\");"
                       "new LinkButton(\"Saved settings\",\"/ip\");"

                       "curtainEl.addEventListener(\"click\",_=>this.setSettingsVisibility(false));settingsBtn.addEventListener(\"click\",_=>this.setSettingsVisibility(true));function setSettingsVisibility(visible){curtainEl.classList.toggle(\"hidden\",!visible);settingsContainerEl.classList.toggle(\"hidden\",!visible);}"
                       "function updateCssColor(){"
                       "let r=Math.pow(Sr.value01*Sl.value01,1/2.2)*255;"
                       "let g=Math.pow(Sg.value01*Sl.value01,1/2.2)*255;"
                       "let b=Math.pow(Sb.value01*Sl.value01,1/2.2)*255;"
                       "let selectedColor=`rgb(${r},${g},${b})`;"
                       "let perceivedBrightness=Math.round((r*299+g*587+b*114)/1000);"
                       "let uiBrightness=perceivedBrightness>125?0:255;"
                       "let uiColRGB=uiBrightness+\",\"+uiBrightness+\",\"+uiBrightness;"
                       "let uiColor=`rgb(${uiColRGB})`;let uiColorTrans1=`rgba(${uiColRGB},0.8)`;"
                       "let uiColorTrans2=`rgba(${uiColRGB},0.5)`;"
                       "colorVarsEl.textContent=`:root{--selected-color:${selectedColor};--ui-color:${uiColor};--ui-color-trans1:${uiColorTrans1};--ui-color-trans2:${uiColorTrans2};}`;}"

                       "updateCssColor();async function getServerState(){let response=await fetch(\"/get\");await updateServerStateFromResponse(response);}getServerState();setInterval(async(_)=>{getServerState();},10*1000);function getServerStateMessageData(forceRgb=true){let state={l:Sl.value};if(Dm.value==\"WIFI\"||forceRgb){state={...state,r:Sr.value,g:Sg.value,b:Sb.value};}return state;}async function setServerState(forceRgb=true){let state=getServerStateMessageData(forceRgb);await sendServerState(state);}async function sendServerState(params,endPoint=\"/set\"){let searchParams=new URLSearchParams(params);try{let response=await fetch(endPoint+\"?\"+searchParams);if(response.ok){await updateServerStateFromResponse(response);return true;}else{doToastMessage(await response.body(),true);}}catch(e){doToastMessage(\"Failed to connect\",true);}}"
                       "async function updateServerStateFromResponse(response){if(!response.ok)return;let json=await response.json();"
                       "Dm.value=json.m;"
                       "Dbm.value=json.bm;"
                       "Ddm.value=json.dm;"
                       "Di.value=json.i;"
                       "Dha.value=json.ha;"
                       "Dhl.value=json.hl;"
                       "Da.value=json.a;"
                       "Dc.value=json.c;"
                       "let col=json.RGBL[0];Sr.value=col.R;Sg.value=col.G;Sb.value=col.B;Sl.value=col.L;}"

                       "async function timeout(ms){await new Promise((r)=>setTimeout(r,ms));}"
                       "function recalculateStyle(elem){window.getComputedStyle(elem).getPropertyValue(\"top\");}"
                       "async function doToastMessage(message,error=false){let el=document.createElement(\"div\");el.classList.add(\"toast\",\"hidden\");el.classList.toggle(\"error\",error);el.textContent=message;document.body.appendChild(el);recalculateStyle(el);el.classList.remove(\"hidden\");await timeout(5000);el.classList.add(\"hidden\");await timeout(200);el.parentElement.removeChild(el);}"
                       "async function installSw(){if(!(\"serviceWorker\"in window.navigator))return;await navigator.serviceWorker.register(\"sw.js\");}installSw();</script></body></html>";
  server.send(200, "text/html", html);
#ifdef Server_SerialEnabled
  Serial.println("SV: 200 OnConnect");
#endif //Server_SerialEnabled
}
void handle_EnableOTA() {
  OTA_setup();
  String message = "OTA enabled '" + String(OTA_Name) + "' at '" + IpAddress2String(WiFi.localIP()) + "'";
  server.send(200, "text/html", message);
#ifdef Server_SerialEnabled
  Serial.println("SV: 200 EnableOTA" + message);
#endif //Server_SerialEnabled
}
void handle_UpdateTime() {
  String ERRORMSG;
  String message = String(TimeCurrent.HH) + ":" + String(TimeCurrent.MM) + ":" + String(TimeCurrent.SS);
  bool TimeUpdated = false;
  if (server.args() > 0) {                      //If manual time given
    for (int i = 0; i < server.args(); i++) {
      String ArguName = server.argName(i);
      ArguName.toLowerCase();
      byte ArgValue = server.arg(i).toInt();
      if (ArguName == PreFixTimeHour) {
        TimeUpdated = true;
        TimeCurrent.HH = ArgValue;
      } else if (ArguName == PreFixTimeMin) {
        TimeUpdated = true;
        TimeCurrent.MM = ArgValue;
      } else if (ArguName == PreFixTimeSec) {
        TimeUpdated = true;
        TimeCurrent.SS = ArgValue;
      } else
        ERRORMSG += "Unknown arg '" + ArguName + "' with value '" + ArgValue + "'\n";
    }
  } else {                                    //If no manual time given, just get it from a time server
    if (UpdateTime())         //Update the time, and if not posible..
      TimeUpdated = true;
    else
      ERRORMSG += "Could not get updated time from the server\n";
  }

  if (TimeUpdated) {   //If time has updated
    message = "Time has updated from " + message + " to " + String(TimeCurrent.HH) + ":" + String(TimeCurrent.MM) + ":" + String(TimeCurrent.SS);
    TimeSet = true;
  } else {
    if (ERRORMSG == "")
      ERRORMSG = "Nothing to update the time to\n";
    message = "Current time is " + message;
  }

  if (ERRORMSG != "")
    server.send(400, "text/plain", ERRORMSG + message);
  else
    server.send(200, "text/plain", message);

#ifdef Server_SerialEnabled
  Serial.println("SV: 200/400 UpdateTime" + ERRORMSG + message);
#endif //Server_SerialEnabled
}
void handle_Info() {
  POT L = LIGHT.ReadStable(PotMinChange, PotStick, AverageAmount);
  String Message = "Code compiled on " +  String(__DATE__) + " " + String(__TIME__) + "\n"
                   "MAC adress = " +  String(WiFi.macAddress()) + "\n"
                   "IP adress = " + IpAddress2String(WiFi.localIP()) + "\n"
                   "OTA_Enabled = " + IsTrueToString(OTA_Enabled) + "\n"
                   "AutoBrightness Value raw = " + String(L.Value) + " (inverse)\n"
                   "AutoBrightness Value math = " + String(GetAutoBrightness(L.Value)) + " =(raw-N)*P+O\n"
                   "Current time =" + String(TimeCurrent.HH) + ":" + String(TimeCurrent.MM) + ":" + String(TimeCurrent.SS) + ""

                   "\nSOFT_SETTINGS\n";
  for (byte i = 3; i < WiFiManager_Settings + 1; i++)
    Message += WiFiManager_VariableNames[i - 1] + " = " + WiFiManager_Get_Value(i, false, true) + "\n";

#ifdef SerialEnabled
  Message += "\nSerial is enabled";
#endif //SerialEnabled

  server.send(200, "text/plain", Message);
#ifdef Server_SerialEnabled
  Serial.println("SV: 200 Info" + Message);
#endif //Server_SerialEnabled
}
void handle_NotFound() {
  String Message = "ERROR URL NOT FOUND: '";
  Message += (server.method() == HTTP_GET) ? "GET" : "POST";
  Message += server.uri();
  if (server.args() > 0) Message += "?";
  for (byte i = 0; i < server.args(); i++) {
    if (i != 0)
      Message += "&";
    Message += server.argName(i) + "=" + server.arg(i);
  }
  server.send(404, "text/plain", Message);
#ifdef Server_SerialEnabled
  Serial.println("SV: 404 " + Message);
#endif //Server_SerialEnabled
}
