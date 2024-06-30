/* ----------------------------------------------------------------------------

OTA.ino : 'Over The Air' allows you to upload sketches remotely.

  For your main app :

  After creating your webserver named MyWebServer with :
    AsyncWebServer MyWebServer(80);         // Create web server
  call:
    OTAConfigureWebServer(MyWebServer);     // Provides web page /start_ota

  NOTES:
    /start_ota shows an informational web page and redirects to
    the real OTA on /update. The information explains to use the
    WiFi user and password to logon to /update

Based on :
  - https://randomnerdtutorials.com/esp8266-nodemcu-ota-over-the-air-arduino/

In Arduino IDE install AsyncElegantOTA by searching in "Manage Libraries"
Note :  The compiler may advise you to move to newer ElegantOTA library which
        now comes with an Async Mode. This is from the same person Ayush Sharma
        but does not seem to work for me.

---------------------------------------------------------------------------- */ 
#warning ..... Ignore 'AsyncElegantOTA library is deprecated' ..... below

#include <AsyncElegantOTA.h>

#warning ..... Ignore 'AsyncElegantOTA library is deprecated' ..... above


// ====== OTA_Start_html web page redirects to /update after 5 seconds.

const char OTA_Start_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
<html lang="en">
<head>
<title>OTA Startup</title>
<meta http-equiv="refresh" content="5; url=/update"/>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<link rel="icon" type="image/x-icon" href="/favicon.ico">
</head>
<body style='background-color:#E6E6FA;'ondblclick="window.location.href='/';">
<center><br><br>
<h1>OTA startup</h1>
Starting ElegantOTA update
<br><br>in a 5 seconds....
<br><br>Username = '%WiFiSSID%'
<br><br>Password = 'WiFi Password'
</body>
</html>
)rawliteral";

// =====  Configure web server

void OTAConfigureWebServer(AsyncWebServer &server) {

  _SERIAL_PRINTLN(F("Configuring OTA ..."));

  // Start ElegantOTA on url http://any_ip/update requesting WiFi creadentials
  AsyncElegantOTA.begin(&WebServer,WiFi.SSID().c_str(),WiFi.psk().c_str());

  // redirects to /update after giving message and waiting some seconds
  server.on("/start_ota", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, F("text/html"), OTA_Start_html, OTA_Start_processor);
  });
}

// =====  Processor for the Configuration Manager function below

String OTA_Start_processor(const String& var) {
  if (var == F("WiFiSSID")) { return WiFi.SSID();  }
  return String();
}

// ----------------------------------------------------------------------------