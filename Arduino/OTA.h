/* Written by JelleWho https://github.com/jellewie

*/

#ifndef OTA_h                               //This prevents including this file more than once
#define OTA_h
#include <Update.h>
class COTA {
  public:
    String UpdateWebpage = "<a href=\"https://github.com/jellewie/Arduino-Smart-light/releases\">Github.com/jellewie/Arduino-Smart-light/releases</a>";
    void handle_uploadPage() {
#ifdef OTA_SerialEnabled
      Serial.println("OTA_handle_UploadPage");
#endif //OTA_SerialEnabled
      const String html = "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>" + UpdateWebpage + "<br><br><form method='POST' action='#' enctype='multipart/form-data' id='upload_form'><input type='file' name='update'><input type='submit' value='Upload'></form><div id='prg'>progress: 0%</div><script>$('form').submit(function(c){c.preventDefault();var a=$('#upload_form')[0];var b=new FormData(a);$.ajax({url:'/update',type:'POST',data:b,contentType:false,processData:false,xhr:function(){var d=new window.XMLHttpRequest();d.upload.addEventListener('progress',function(e){if(e.lengthComputable){var f=e.loaded/e.total;$('#prg').html('progress: '+Math.round(f*100)+'%')}},false);return d},success:function(f,e){console.log('success!')},error:function(e,d,f){}})});</script>";
      server.send(200, "text/html", html);
    }
    void handle_update() {
#ifdef OTA_SerialEnabled
      Serial.printf("OTA: Update");
#endif //OTA_SerialEnabled
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }
    void handle_update2() {
      //if (server.argName(0) != "admin") return;               //Just for later when I implement a 'secret auth code'

      HTTPUpload& upload = server.upload();

#ifdef OTA_SerialEnabled
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {               //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {          //flashing firmware to ESP
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
          Update.printError(Serial);
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true))                                   //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        else
          Update.printError(Serial);
      }
#else
      if (upload.status == UPLOAD_FILE_START)
        Update.begin(UPDATE_SIZE_UNKNOWN);                      //start with max available size
      else if (upload.status == UPLOAD_FILE_WRITE)              //flashing firmware to ESP
        Update.write(upload.buf, upload.currentSize);
      else if (upload.status == UPLOAD_FILE_END)
        Update.end(true);
#endif //OTA_SerialEnabled
    }
};
COTA OTA;

void OTA_handle_uploadPage() {
  OTA.handle_uploadPage();
}
void OTA_handle_update() {
  OTA.handle_update();
}
void OTA_handle_update2() {
  OTA.handle_update2();
}

#endif
