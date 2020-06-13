//<ip>/ani?PreFix=Value[&....]
//#define PreFixAnimationMode "m"     //Animation ID in value
//#define PreFixAnimationtime "t"     //Animation show time
//
//server.on("/ani",         handle_ShowAnimation);
//
//void handle_ShowAnimation() {
//  byte AnimationID = random(0, TotalAnimations);
//  int AnimationTime = AnimationCounterTime;
//  String ERRORMSG;
//  if (server.args() > 0) {                      //If manual time given
//    for (int i = 0; i < server.args(); i++) {
//      String ArguName = server.argName(i);
//      ArguName.toLowerCase();
//      byte ArgValue = server.arg(i).toInt();
//      if (ArguName == PreFixAnimationMode)
//        AnimationID = ArgValue;
//      else if (ArguName == PreFixAnimationtime)
//        AnimationTime = ArgValue;
//      else
//        ERRORMSG += "Unknown arg '" + ArguName + "' with value '" + ArgValue + "'" + char(13);
//    }
//  }
//  StartAnimation(AnimationID, AnimationTime);       //Start a random Animation
//  String message = "Animation " + String(AnimationID) + " started for " + String(AnimationTime) + " seconds";
//  if (ERRORMSG != "")
//    server.send(400, "text/plain", ERRORMSG + message);
//  else
//    server.send(200, "text/plain", message);
//
//#ifdef Server_SerialEnabled
//  Serial.println("SV: 200/400 " + ERRORMSG + message);
//#endif //Server_SerialEnabled
//}

//bool IsCurrentlyOn() {
//  if (LEDs[0].r != 0 or LEDs[0].g != 0 or LEDs[0].b != 0)
//    return true;
//  return false;
//}

//bool TickEveryMS(int _Delay) {
//  static unsigned long _LastTime = 0;       //Make it so it returns 1 if called for the FIST time
//  if (millis() > _LastTime + _Delay) {
//    _LastTime = millis();
//    return true;
//  }
//  return false;
//}


//Tried to make a option to make a Progressive Web App from it. turns out we need https for it, so f*ck that
//this would cost 4124 Bytes of program storage space
//
//server.on("/icon.svg",      APP_icon);
//server.on("/sw.js",         APP_sw);
//server.on("/manifest.json", APP_manifest);
//
//void APP_icon() {
//  String message = "<?xmlversion=\"1.0\"encoding=\"utf-8\"?><!--SvgVectorIcons:http://www.onlinewebfonts.com/icon--><!DOCTYPEsvgPUBLIC\"-//W3C//DTDSVG1.1//EN\"\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\"><svgversion=\"1.1\"xmlns=\"http://www.w3.org/2000/svg\"xmlns:xlink=\"http://www.w3.org/1999/xlink\"x=\"0px\"y=\"0px\"viewBox=\"0010001000\"enable-background=\"new0010001000\"xml:space=\"preserve\"><metadata>SvgVectorIcons:http://www.onlinewebfonts.com/icon</metadata><g><pathd=\"M586.9,809.2H413c-13.5,0-24.4,10.9-24.4,24.4c0,13.5,10.9,24.4,24.4,24.4h173.9c13.5,0,24.4-10.9,24.4-24.4C611.3,820.1,600.3,809.2,586.9,809.2z\"/><pathd=\"M455.2,951.3c3.3,21.9,22,38.7,44.8,38.7c22.8,0,41.5-16.8,44.8-38.7c29.8-1.8,53.6-26.3,53.6-56.6H401.5C401.5,925,425.3,949.5,455.2,951.3z\"/><pathd=\"M500,150.7c18,0,32.5-14.6,32.5-32.5V42.5c0-18-14.5-32.5-32.5-32.5c-18,0-32.5,14.6-32.5,32.5v75.6C467.5,136.1,482,150.7,500,150.7z\"/><pathd=\"M286.7,198c6.4,8.8,16.3,13.4,26.4,13.4c6.6,0,13.3-2,19.1-6.2c14.5-10.5,17.8-30.9,7.2-45.4l-44.5-61.2c-10.6-14.5-30.9-17.8-45.4-7.2c-14.5,10.5-17.7,30.9-7.2,45.4L286.7,198z\"/><pathd=\"M713.3,674.6c-10.5-14.5-30.9-17.7-45.4-7.2c-14.5,10.6-17.7,30.9-7.2,45.4l44.5,61.2c6.3,8.7,16.3,13.4,26.3,13.4c6.6,0,13.3-2,19.1-6.2c14.5-10.6,17.7-30.9,7.2-45.4L713.3,674.6z\"/><pathd=\"M207.5,307.1l-72-23.4c-17.2-5.6-35.4,3.8-41,20.9c-5.6,17.1,3.8,35.4,20.9,41l72,23.4c3.4,1.1,6.7,1.6,10.1,1.6c13.7,0,26.4-8.7,30.9-22.5C233.9,331,224.6,312.6,207.5,307.1z\"/><pathd=\"M884.6,527.1l-71.9-23.4c-17.1-5.5-35.4,3.8-41,20.9c-5.6,17.1,3.8,35.4,20.9,41l72,23.4c3.3,1.1,6.7,1.6,10,1.6c13.7,0,26.4-8.7,30.9-22.5C911,551,901.7,532.7,884.6,527.1z\"/><pathd=\"M187.4,503.7l-72,23.3c-17.1,5.5-26.4,23.9-20.9,41c4.4,13.7,17.2,22.5,30.9,22.5c3.3,0,6.7-0.5,10-1.6l72-23.3c17.1-5.5,26.5-23.9,20.9-41C222.8,507.6,204.6,498.2,187.4,503.7z\"/><pathd=\"M802.6,370.5c3.3,0,6.7-0.5,10-1.6l71.9-23.4c17.1-5.6,26.4-23.9,20.9-41c-5.6-17.1-24-26.4-41-20.9l-72,23.4c-17.1,5.6-26.4,23.9-20.9,41C776.2,361.8,788.9,370.5,802.6,370.5z\"/><pathd=\"M286.7,674.6l-44.5,61.2c-10.6,14.5-7.4,34.9,7.2,45.4c5.7,4.2,12.4,6.2,19.1,6.2c10.1,0,20-4.6,26.3-13.4l44.5-61.2c10.6-14.5,7.4-34.9-7.2-45.4C317.6,656.8,297.2,660.1,286.7,674.6z\"/><pathd=\"M705.2,98.6l-44.5,61.2c-10.5,14.5-7.3,34.9,7.2,45.4c5.8,4.2,12.5,6.2,19.1,6.2c10.1,0,20-4.6,26.4-13.4l44.5-61.2c10.5-14.5,7.3-34.9-7.2-45.4C736,80.8,715.8,84.1,705.2,98.6z\"/><pathd=\"M734.8,437.5c0-129.7-105.1-234.9-234.8-234.9c-129.7,0-234.9,105.2-234.9,234.9c0,54.7,18.8,105,50.3,144.9c27.6,35,53.6,73.1,76.3,111.5v78H607V696c22.4-37.3,51.4-80.4,77.5-113.6C716,542.5,734.8,492.3,734.8,437.5z\"/></g></svg>";
//  server.send(200, "application/svg+xml", message);
//  Serial.println("APP_icon" + message);
//}
//
//void APP_sw() {
//  String message = "async function onInstall(){await self.skipWaiting()}async function openCache(){return await caches.open(\"mainCache\")}self.addEventListener(\"install\",e=>{e.waitUntil(onInstall())}),self.addEventListener(\"fetch\",e=>{let t=(async t=>{let a=null;try{a=await fetch(e.request)}catch(e){}if(a&&a.ok){return(await openCache()).put(e.request,a.clone()),a}return await caches.match(e.request)})();e.respondWith(t)});";
//  server.send(200, "application/javascript", message);
//  Serial.println("APP_sw" + message);
//}
//
//void APP_manifest() {
//  String message = "{\"name\":\"Light Control\",\"short_name\":\"Light Control\",\"start_url\":\".\",\"display\":\"standalone\",\"description\":\"\",\"icons\":[{\"src\":\"icon.svg\",\"type\":\"image/png\",\"sizes\":\"150x150\"}],\"background_color\":\"#ffffff\",\"theme_color\":\"#ffffff\"}";
//  server.send(200, "application/json", message);
//  Serial.println("APP_manifest" + message);
//}
