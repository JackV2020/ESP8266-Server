/* ----------------------------------------------------------------------------

FOTA.ino : 'Firmware On The Air' upload sketches remotely.

  For your main app :

    Install 'ESPAsyncTCP' by dvarrel and 'ESP Async WebServer' by Me-No-Dev
    via the library manager. Note the spaces in the name 'ESP Async WebServer'

  After creating your webserver named MyWebServer with :
    AsyncWebServer MyWebServer(80);
  call:
    FOTAConfigureWebServer(MyWebServer,User,Password); 
  and you can use /FOTA/FOTA

  NOTES:

    This FOTA works for Wemos D1 mini Pro which is ESP8266 based.
    May work for other ESP8266 devices too.
    I only have Wemos D1 mini Pro so can not develop and test for 
    other devices based on ESP8266, ESP etc.
    Feel free to adopt for your needs.

---------------------------------------------------------------------------- */

#include "FOTA.h"

// =====  Configure web server

void FOTAConfigureWebServer(AsyncWebServer &server, String &username, String &password) {

  Serial.println(F("\nConfiguring FOTA ..."));

  FOTA_user = username;
  FOTA_password = password;

  // ----- The 'Update Firmware On the air' web page

  server.on("/FOTA/FOTA", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!FOTA_isAuthenticated(request)) {
      return;
    }
    request->send(200, F("text/html"), FOTA_html);
  });

  // ----- Perform the firmware upload

  server.on(
    "/FOTA/upload", HTTP_POST,
    // This is part 1 and is executed after Part 2 below (upload) finishes
    [](AsyncWebServerRequest *request) {
      if (!FOTA_isAuthenticated(request)) {
        return;
      }
      Serial.println("FOTA Close Connection");
      AsyncWebServerResponse *response = request->beginResponse((Update.hasError()) ? 400 : 200, "text/plain", (Update.hasError()) ? FOTA_update_error.c_str() : "Update oke");
      response->addHeader("Connection", "close");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
      if (Update.hasError()) {
        Serial.println("FOTA Update Error");
      } else {
        Serial.println("FOTA Update Oke");
      }
    },
    // This is part 2 and is the actual upload and is executed before Part 1 above
    [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      if (!FOTA_isAuthenticated(request)) {
        return;
      }
      if (!index) {
        // Initialise when we receive the first data frame
        FOTA_bytes_written = 0;
        FOTA_update_error = "";
        Update.runAsync(true);
        if (!Update.begin(((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))) {
          Serial.println("FOTA Start Not Ok");
          FOTA_update_error = Update.getErrorString(); 
          FOTA_update_error.concat("\n");
        } else {
          Serial.println("FOTA Start Oke");
        }
      }

      if (len) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        if (Update.write(data, len) != len) {
          return request->send(400, "text/plain", "Failed to save update");
        }
        FOTA_bytes_written += len;
        Serial.printf_P(PSTR("\nFOTA Saving data len: %d ; total: %d"), len, FOTA_bytes_written);
      }

      if (final) {
        Serial.println("\nFOTA bytes written: " + String(FOTA_bytes_written));
        digitalWrite(LED_BUILTIN,1);
        if (!Update.end(true)) {  // true sets the size to the amount of saved bytes
          FOTA_update_error = Update.getErrorString(); 
          FOTA_update_error.concat("\n");
        }
      } else {
        return;
      }
    });

  // ----- Serve the reboot button on the FOTA web page

  WebServer.on("/FOTA/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!FOTA_isAuthenticated(request)) {
      return;
    }
    Serial.println("Reboot");
    ESP.restart();
  });

}

// ===== Check authentication

bool FOTA_isAuthenticated(AsyncWebServerRequest *request) {
  if (!request->authenticate(FOTA_user.c_str(), FOTA_password.c_str())) {
    request->requestAuthentication();
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------