/* ----------------------------------------------------------------------------

ESP8266-Servers.ino : Some sections integrated into one app :

  - Every section has its own tab(s)
    ... To make it is easy to copy and use it in your other apps
    ... The header of each section explains what to do

  - ESP8266_Servers.ino brings them all together in one app
    ... It is this one you are reading 8-)

  - Configuration Manager to manage setting
    ... The initial installation has no settings but provides an access point
    ...... SSID 'Setup 192.168.4.1 1234567890'
    ...... Logon with password 1234567890 and browse to 192.168.4.1
    ... On the home page you click Configure
    ... Click the Info button and read what's there
    ... Fill in the fields you want to change
    ... Fill in the password in the bottom field
    ... Click 'Save'

  - LittleFSWeb to manage the LittleFS file system on a web page
    ... Access your device via WiFi
    ... On the home page you click LittleFSWeb
    ... Logon
    ... Click the Info button and read what's there
    ...... Create new files and (subdirectories)
    ...... Select a folder and upload files
    ...... Download files
    ...... Rename and delete a file or directory(tree)
    ...... Edit files in web based editor

  - NAPT_RNAT implements a simple router
    ... NAPT for 4 clients
    ... DHCP reservations template which can be editted with LittleFSWeb
    ... Web page showing IP, MAC and name of connected NAPT clients
    ... Reverse NAT to access clients from 'outside'
    ... Reverse NAT template which can be editted with LittleFSWeb
    
  - Websocket server with 2 applications in it:
    ... Control 6 GPIOs, react on interrupts of 5 GPIO's and receive A0 reading
    ... wsDataStore to share data between clients in a sort of 'MQTT-like-way'
    ...... you can share data between clients on NAPT and WiFi side
    ...... the app itself can also share in wsDataStore  (ntp saves boottime)

  - NTP client:
    ... The internal clock is synchronised with ntp
    ... Has functions to give you boottime, uptime and current time

  - OTA, 'Over The Air' allows you to upload sketches remotely.
    ... No need to connect to your computer when you have an upgrade

  - Examples and templates is not a real section
    ... Files are created after startup but only when they do not exist
    ... When you wreck one, delete it and reboot and the template is created
    ... Template for ntp configuration (Summer and winter time)
    ... Template for DHCP reservations on NAPT subnet
    ... /zExamples folder with 3 websocket client examples
    ...... html to share data with other clients
    ...... ino to share data with other clients ( also with html clients )
    ...... html to control GPIOs, react on interrupt and receive A0 readings

  - The main webserver
    ... Has a function in it so it can replace strings
        in your html, css and js files with other strings.
    ...... See the proc_processor.html.proc example in /zExamples for that.
    ...... Note the extra file extension .proc to trigger the replacing.
    ... Prefers to serve gzip files so it soes not have to send that much.
    ...... Try the next :
    ...... Download /LittleFSWeb/LittleFSWeb.html
    ...... On your computer gzip (with for example 'gzip -k LittleFSWeb.html')
    ...... Select the folder /LittleFSWeb
    ...... Upload '/LittleFSWeb/LittleFSWeb.html.gz'
    ...... Now the LittleFSWeb page may start faster.
    ... Creates /LittleFSWeb/LittleFSWeb.html and also /index.html when
        they do not exist. So when you want the original back.....
        Just delete them using LittleFSWeb and reboot.
    ... Also serves a json page with information you may be interested in like
      heap info, network and time details

  NOTES:

  1) To use the memory efficiently the wsDataStore is in 2nd Heap 
    and MMU setting needs to be option 3 to enable wsDataStore:
    "16KB cache + 48KB IRAM and 2nd Heap (shared)" 

  2) Every section explains which libraries to install and sometimes more

  3) You may tune : 
    - SERIAL_PRINT                        in this file below
    - DEFAULT_MAX_WS_CLIENTS              in this file below
    - chunkSize                           in LittleFSWeb.h
    - IP_NAPT_VALUE and IP_PORTMAP_VALUE  in NAPT_RNAT.ino
    - wsDSNumRows                         in wsServer.ino 

    LittleFS.h is included in Arduino IDE

    ESPAsyncTCP.h and ESPAsyncWebServer.h
      both be installed using the Arduino Library Manager
    OR....
      the next zip files could be downloaded:
        https://github.com/me-no-dev/ESPAsyncTCP/archive/master.zip
        https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip
      .... and installed
        Unzip files into the folder libraries (which you may need to create) in
        the folder you specified in > File > Preferences > Sketchbook location:
      ... OR
        Unzip by > Sketch > Include Library > Add .ZIP Library... (2nd from top)

  Also check the other .ino files to see what libraries you need to add.

  - Kindly meant : As long as you keep this notice in here you are 
      licensed to do whatevery you want to do with this all and ...
      my guarantee for it to work ends as soon as you implement any of this.

---------------------------------------------------------------------------- */

#define APPVERSION "V1.0.0"

/*
  I use compiler macros to include / exclude all Serial.print... statements
 This means you will not find any Serial.print... statements in my code but
 you will find  _SERIAL_PRINT statements 
 
 Set to true when compiling for debugging during development
 Set to false when compiling for production
*/

#define SERIAL_PRINT true

#if SERIAL_PRINT
#define _SERIAL_BEGIN(z) Serial.begin(z)
#define _SERIAL_PRINT(z) Serial.print(z)
#define _SERIAL_PRINTLN(z) Serial.println(z)
#define _SERIAL_PRINTF_P1(y, z) Serial.printf_P(y, z)        // P1 : 1 parameter
#define _SERIAL_PRINTF_P2(x, y, z) Serial.printf_P(x, y, z)  // P2 : 2 etc.
#define _SERIAL_PRINTF_P3(w, x, y, z) Serial.printf_P(w, x, y, z)
#define _SERIAL_PRINTF_P4(v, w, x, y, z) Serial.printf_P(v, w, x, y, z)
#else
#define _SERIAL_BEGIN(z)
#define _SERIAL_PRINT(z)
#define _SERIAL_PRINTLN(z)
#define _SERIAL_PRINTF_P1(y, z)
#define _SERIAL_PRINTF_P2(x, y, z)
#define _SERIAL_PRINTF_P3(w, x, y, z)
#define _SERIAL_PRINTF_P4(v, w, x, y, z)
#endif

// ------------------------------------- Include modules used by more ino files

// See wsServer.ino for the next one
#define DEFAULT_MAX_WS_CLIENTS 6

#include <LittleFS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ------------------------------------------ Include the Configuration Manager

#include "CfgMgr.h"

/*
Did you forget Configuration Manager Password, LittleFSWeb user and or password,
  broke /index.html or /LittleFSWeb/LittleFSWeb.html?
  
  - Connect D8 (GPIO15) to 3.3v while powered on
     (this will reset the above only and restart)
*/

#define CfgMgrResetGPIO D8

// ------------------------------------------------------------- 2 PROGMEM pages

/*

  Page 1 in PROGMEM 'indexhtml' :
  Initial contents for /index.html when it is missing after a boot.
  You can change /index.html for your own needs using the editor in LittleFSWeb.

  Page 2 in PROGMEM 'proc_processor_example' :
  An example to use proc_processor to replace strings in html, css and js files.
  Created as /proc_processor_example.html.proc when file is missing after boot.
    
*/

const char indexhtml[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
<html lang="en">
<head>
<title>Home</title>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<link rel="icon" type="image/x-icon" href="/favicon.ico">
<style>
.rnd_btn {background-color:lightgrey;border-radius:50%;border-width:3;
  border-color:gold;color:blue;width:100px;height:50px;text-align:center}
</style>
</head>
<body style='background-color: #E6E6FA;'>
<center>
<h1>Configuring</h1>
WiFi and other settings.<br>
Initial password: <font color='#9900FF'>secret</font><br><br>
<a href='/ConfigurationManager'><button class="rnd_btn">Configure</button></a>
<h1>LittleFSWeb</h1>
Web interface to manage files.<br>
Initial username: <font color='#9900FF'>admin</font><br>
Initial password: <font color='#9900FF'>admin</font><br><br>
<a href='/LittleFSWeb/LittleFSWeb.html'>
  <button class="rnd_btn">LittleFSWeb</button></a>
<h1>NAPT and JSON</h1>
<a href='/NAPTClients'><button class="rnd_btn">NAPT Clients</button></a><br><br>
<a href='/JsonWebPage'><button class="rnd_btn">JSON</button></a>
<p id='info'></p>
</center>
</body>
</html>
)rawliteral";


const char proc_processor_example[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
<html lang="en">
<head>
</head>
<body>
<center>
The web server has a function named proc_processor<br>
which can replace strings in your html, js or css by actual values.<br>
The standard supported strings with their actual values are:<br><br>
LED_BUILTIN: %LED_BUILTIN%<br>
GPIO2: %GPIO2%<br>
GPIO4: %GPIO4%<br>
GPIO5: %GPIO5%<br>
GPIO12: %GPIO12%<br>
GPIO13: %GPIO13%<br>
GPIO14: %GPIO14%<br>
A0: %A0%<br>
KEY1: %KEY1%<br>
KEY2: %KEY2%<br>
KEY3: %KEY3%<br>
KEY4: %KEY4%<br>
BootTime: %BOOTTIME%<br>
UpTime: %UPTIME%<br>
<br>
To use that function you add the extension .proc to the name of your file.<br>
To get a string replacement you put a string between 2 %%'s.
<br>
Example : to get the value of LED_BUILTIN you use %%LED_BUILTIN%%<br>
( only in your .html.proc, .js.proc or .css.proc file)<br>
<br>
Notes:<br>
- Any other single %% needs to be replaced by 2 %%'s<br>
as you can see when you edit this file.<br>
- KEY1..KEY4 are random numbers which are created just after reboot<br>
and can be used to encrypt traffic.<br>
- And of course you can add what you want in the proc_processor function.
</center>
</body>
</html>
)rawliteral";

// ----------------------------------------------------------------- Web Server

AsyncWebServer WebServer(80);

// ---------------------------------------------------------------------- Setup

void setup() {

  _SERIAL_BEGIN(115200);  // any '_SERIAL_....' is replaced/removed by compiler.
  delay(5000);
  _SERIAL_PRINTLN(F("\n\nStartup begin...\n"));

  InitFS();

  CfgMgrReadConfig();

  LittleFSWebinit(CfgMgrLittleFSWebuser, CfgMgrLittleFSWebpassword);

  if (initWiFi()) {

    ntpInit();

    NAPT_RNAT_init(CfgMgrNAPTssid, CfgMgrNAPTpass, CfgMgrNAPTnet, CfgMgrNAPTDNS);

  } else {

    WiFi.mode(WIFI_AP_STA);

    WiFi.softAP(CfgMgrWiFihostname
                  + F(" 192.168.4.1 1234567890"),
                F("1234567890"));
  }

  // --- Begin WebServer setups

  _SERIAL_PRINTLN(F("Webserver Setup..."));

  WebServer.onNotFound(notFound);

  if (!LittleFS.exists(F("/index.html"))) {
    File file = LittleFS.open(F("/index.html"), "w");
    file.print(FPSTR(indexhtml));
    file.close();
  }

  WebServer.on("/JsonWebPage", HTTP_GET, JsonWebPage);

  // Configuration Manager
  CfgMgrConfigureWebServer(WebServer);

  // LittleFS Web Interface
  LittleFSWebConfigureWebServer(WebServer);

  // Network Address and Port Translation for extra routed WiFi network
  NAPT_RNAT_ConfigureWebServer(WebServer);

  // On The Air upgrades
  OTAConfigureWebServer(WebServer);

  // Websocket Server
  wsServerConfigureWebSocketServer(WebServer);

  // Anything you create using LittleFSWeb
  WebServerConfigure();

  WebServer.begin();

  // ---- End WebServer setups

  _SERIAL_PRINTLN(F("\nStartup complete...\n"));

  ShowSystemInfo();

  pinMode(CfgMgrResetGPIO, INPUT);
}
// ----------------------------------------------------------------------- Loop

unsigned long prevmicros = 0;

void loop() {

  // Reset when essential things are gone
  if (digitalRead(CfgMgrResetGPIO)) { CfgMgrResetConfig(); }

  // Required by websocket server
  wsServerClearAllClients();

  // Your additional code goes here :
}

// ----------------------------------------------------------------------- WiFi

bool initWiFi() {
  _SERIAL_PRINTLN(F("Connect to WiFi ... \n"));
  pinMode(LED_BUILTIN, OUTPUT);
  WiFi.persistent(false);  // do not store WiFi credentials in flash
  WiFi.mode(WIFI_STA);
  WiFi.hostname(CfgMgrWiFihostname);
  WiFi.begin(CfgMgrWiFissid, CfgMgrWiFipass);
  int count = 0;
  while ((WiFi.status() != WL_CONNECTED) && (count <= 50)) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(200);
    count = count + 1;
  }
  digitalWrite(LED_BUILTIN, 1);
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
    _SERIAL_PRINTLN(F("No WiFi ..."));
    return false;
  }
  return true;
}

// ------------------------------------------------------------------------ Web


// ====== notFound handeled by WebServerConfigure()

void notFound(AsyncWebServerRequest *request) {
  _SERIAL_PRINTLN(F("Client:")
                  + request->client()->remoteIP().toString()
                  + F(" ") + request->url());
  request->send(404, "text/plain", F("Not found"));
}

/*
// Function to check if the request comes from a mobile device
// needed for brotli support which I disabled. see more below. Search for .br
bool isMobileClient(AsyncWebServerRequest *request) {
  if (request->hasHeader(F("User-Agent"))) {
    String userAgent = request->header(F("User-Agent"));
    userAgent.toLowerCase();
    // Common substrings in mobile user agents
    if (userAgent.indexOf(F("mobile")) != -1 
      || userAgent.indexOf(F("android")) != -1 
      || userAgent.indexOf(F("iphone")) != -1 
      || userAgent.indexOf(F("ipad")) != -1) {
      return true;
    }
  }
  return false;
}
*/

String getContentType(String path) {
  if (path.endsWith(F(".html"))
      || path.endsWith(F(".html.gz"))
      // || path.endsWith(F(".html.br"))
      || path.endsWith(F(".html.proc"))) {
    return F("text/html");
  } else if (path.endsWith(F(".css"))
             || path.endsWith(F(".css.gz"))
             // || path.endsWith(F(".css.br"))
             || path.endsWith(F(".css.proc"))) {
    return F("text/css");
  } else if (path.endsWith(F(".js"))
             || path.endsWith(F(".js.gz"))
             // || path.endsWith(F(".js.br"))
             || path.endsWith(F(".js.proc"))) {
    return F("application/javascript");
  } else if (path.endsWith(F(".gz"))) {
    // Show raw gz file in stead of downloading it.
    return F("text/html");
  } else if (path.endsWith(F(".json"))) {
    return F("application/json");
  } else if (path.endsWith(F(".png"))) {
    return F("image/png");
  } else if (path.endsWith(F(".jpg"))) {
    return F("image/jpeg");
  } else if (path.endsWith(F(".ico"))) {
    return F("image/x-icon");
  } else if (path.endsWith(F(".svg"))) {
    return F("image/svg+xml");
  }
  return F("text/plain");
}

int key1;
int key2;
int key3;
int key4;

void WebServerConfigure() {

  _SERIAL_PRINTLN(F("Configuring WebServer ..."));

  WebServer.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
    ESP.restart();
  });

  WebServer.on("/*", HTTP_GET, [](AsyncWebServerRequest *request) {
    String path = request->url();

    //    _SERIAL_PRINTLN("About to serve: "+path);
    // Check if the path contains a "." indicating a file
    if ((path.indexOf(F(".")) == -1) && (path != F("/"))) { path += F("/"); }
    // Check if the path ends with a slash, indicating a directory
    if (path.endsWith(F("/"))) { path += F("index.html"); }

    // When using compression make sure your browser supports it.

    /*

    // To enable brotli over http in FireFox 
    //  - about:config and look for network.http.accept-encoding 
    // and change to : gzip, deflate, br, zstd

    // Check for Brotli compressed version
    if ( (! isMobileClient(request) ) && (LittleFS.exists(path + F(".br"))) ) {
      AsyncWebServerResponse *response = 
      request->beginResponse(LittleFS, path + F(".br"), getContentType(path));
      response->addHeader(F("Content-Encoding"), F("br"));
      _SERIAL_PRINTLN(F("Serve Brotli : ") + path + F(".br"));
      request->send(response);
    } else
*/
    // Check for Gzip compressed version
    if (LittleFS.exists(path + F(".gz"))) {
      AsyncWebServerResponse *response =
        request->beginResponse(LittleFS, path + F(".gz"), getContentType(path));
      response->addHeader(F("Content-Encoding"), F("gzip"));
      _SERIAL_PRINTLN(F("Serve Gzip : ") + path + F(".gz"));
      request->send(response);
    }

    // Check for uncompressed version
    else if (LittleFS.exists(path)) {
      _SERIAL_PRINTLN(F("Serve Uncompressed : ") + path);
      if (path.endsWith(F(".proc"))) {
        request->send(LittleFS, path, getContentType(path),
                      false,  // false enable processor | true send as template.
                      proc_processor);
      } else {
        request->send(LittleFS, path, getContentType(path));
      }
    }
    // If no file found, return 404
    else {
      _SERIAL_PRINTLN(F("File Not Found: ") + path);
      //request->send(404, "text/plain", "File Not Found");
      request->send(200, "text/html",
                    F("<meta http-equiv='refresh' content='2; URL=/'>"));
    }
  });

  // Generate 4 random keys  which are used in the proc_processor below.
  srand(micros());
  key1 = rand() % (127) + 1;
  key2 = rand() % (127) + 1;
  key3 = rand() % (127) + 1;
  key4 = rand() % (127) + 1;

  if (!LittleFS.exists(F("/zExamples/proc_processor.html.proc"))) {
    File file = LittleFS.open(F("/zExamples/proc_processor.html.proc"), "w");
    file.print(FPSTR(proc_processor_example));
    file.close();
  }
}

// ======  Processor for the 'WebServer.on("/*"...' above


// Replaces strings between two % signs in html.proc, js.proc and css.proc files
// Example : <h1>%LED_BUILTIN%</h1>
// Note that other single % signs have to be replaced by 2 % signs like :
//   'border-radius: 50%;' becomes 'border-radius: 50%%;'

String proc_processor(const String &var) {
  if (var == F("LED_BUILTIN")) {
    return String(digitalRead(LED_BUILTIN));
  } else if (var == F("GPIO2")) {
    return String(digitalRead(2));
  } else if (var == F("GPIO4")) {
    return String(digitalRead(4));
  } else if (var == F("GPIO5")) {
    return String(digitalRead(5));
  } else if (var == F("GPIO12")) {
    return String(digitalRead(12));
  } else if (var == F("GPIO13")) {
    return String(digitalRead(13));
  } else if (var == F("GPIO14")) {
    return String(digitalRead(14));
  } else if (var == F("A0")) {
    return String(analogRead(A0));
  } else if (var == F("KEY1")) {
    return String(key1);
  } else if (var == F("KEY2")) {
    return String(key2);
  } else if (var == F("KEY3")) {
    return String(key3);
  } else if (var == F("KEY4")) {
    return String(key4);
  } else if (var == F("BOOTTIME")) {
    return ntpBootTime();
  } else if (var == F("UPTIME")) {
    return String(ntpUpTime());
  }
  return String();
}

// ------------------------------------------------------------ showNetworkInfo

void ShowSystemInfo() {
  // You may want to add more info to be shown on startup.
  _SERIAL_PRINTLN(F("--------------------------------------------------"));
  _SERIAL_PRINTF_P1(PSTR("           WiFi IP: %s\n"),
                    WiFi.localIP().toString().c_str());
  _SERIAL_PRINTF_P1(PSTR("     WiFi hostname: %s\n"),
                    WiFi.hostname().c_str());
  _SERIAL_PRINTF_P1(PSTR("         WiFi SSID: %s\n"),
                    WiFi.SSID().c_str());
  _SERIAL_PRINTF_P1(PSTR("          WiFi Pwd: %s\n"),
                    WiFi.psk().c_str());
  _SERIAL_PRINTF_P1(PSTR("       Wifi Status: %d\n"),
                    WiFi.status());
  _SERIAL_PRINTF_P1(PSTR("       Wifi Signal: %d dBm\n"),
                    WiFi.RSSI());
  _SERIAL_PRINTF_P1(PSTR("          WiFi MAC: %s\n"),
                    WiFi.macAddress().c_str());
  _SERIAL_PRINTF_P1(PSTR("      WiFi Gateway: %s\n"),
                    (WiFi.gatewayIP()).toString().c_str());
  _SERIAL_PRINTF_P1(PSTR("       WiFi Subnet: %s\n"),
                    (WiFi.subnetMask()).toString().c_str());
  _SERIAL_PRINTF_P1(PSTR("        WiFi DNS 1: %s\n"),
                    (WiFi.dnsIP(0)).toString().c_str());
  _SERIAL_PRINTF_P1(PSTR("        WiFi DNS 2: %s\n"),
                    (WiFi.dnsIP(1)).toString().c_str());
  _SERIAL_PRINTF_P1(PSTR("        WiFi DNS 3: %s\n"),
                    (WiFi.dnsIP(2)).toString().c_str());
  _SERIAL_PRINTF_P1(PSTR("           AP SSID: %s\n"),
                    WiFi.softAPSSID().c_str());
  _SERIAL_PRINTF_P1(PSTR("            AP Pwd: %s\n"),
                    CfgMgrNAPTpass.c_str());
  _SERIAL_PRINTF_P1(PSTR("            AP MAC: %s\n"),
                    WiFi.softAPmacAddress().c_str());
  _SERIAL_PRINTF_P1(PSTR("             AP IP: %s\n"),
                    WiFi.softAPIP().toString().c_str());
  _SERIAL_PRINTF_P1(PSTR("            AP DNS: %s\n"),
                    CfgMgrNAPTDNS.c_str());
  _SERIAL_PRINTF_P1(PSTR("  LittleFSWeb User: %s\n"),
                    CfgMgrLittleFSWebuser.c_str());
  _SERIAL_PRINTF_P1(PSTR("   LittleFSWeb Pwd: %s\n"),
                    CfgMgrLittleFSWebpassword.c_str());
  _SERIAL_PRINTF_P1(PSTR(" Configuration Pwd: %s\n"),
                    CfgMgrpass.c_str());
  _SERIAL_PRINTLN(F("--------------------------------------------------"));
}

// ------------------------------------------------------------------- LittleFS

// ====== Initialize LittleFS

void InitFS() {
  if (!LittleFS.begin()) {
    _SERIAL_PRINTLN(F("LittleFS mount failed\nFormatting LittleFS filesystem"));
    LittleFS.format();
    if (!LittleFS.begin()) {
      _SERIAL_PRINTLN(F("Formatting LittleFS filesystem failed"));
    }
  }
}

// ====== json page

void JsonWebPage(AsyncWebServerRequest *request) {

  String html_response = F("{");
  html_response.concat(systemJsonInfo());

  html_response.concat(WiFiJsonInfo());
  html_response.concat(LittleFSWebJsonInfo());
  html_response.concat(NAPT_RNAT_JsonInfo());
  html_response.concat(wsServerJsonInfo());
  html_response.concat(ntpJsonInfo());
  html_response.concat(F("\"}"));

  request->send(200, "text/html", html_response);
}

String systemJsonInfo() {
  String json = "";
  json.concat(F("\"Hostname\":\""));
  json.concat(CfgMgrWiFihostname);
  json.concat(F("\" , \"Version\":\""));
  json.concat(APPVERSION);
  json.concat(F("\" , \"SDK Version\":\""));
  json.concat(ESP.getSdkVersion());
  json.concat(F("\" , \"SERIAL_PRINT\":\""));
  json.concat(SERIAL_PRINT);
  json.concat(F("\" , \"getChipId\":\""));
  json.concat(String(ESP.getChipId()));
  json.concat(F("\" , \"led\":\""));
  if (digitalRead(LED_BUILTIN)) {
    json.concat(F("off"));
  } else {
    json.concat(F("on"));
  }
  json.concat(F("\" , \"FreeContStack\":\""));
  json.concat(String(ESP.getFreeContStack()));
  json.concat(F("\" , \"DFreeHeap\":\""));
  json.concat(String(ESP.getFreeHeap()));
  json.concat(F("\" , \"DHeapFragmentation\":\""));
  json.concat(String(ESP.getHeapFragmentation()));
  json.concat(F("\" , \"DMaxFreeBlockSize\":\""));
  json.concat(String(ESP.getMaxFreeBlockSize()));
  return json;
}

String WiFiJsonInfo() {
  String json = "";
  json.concat(F("\" , \"wifi_dBm\":\""));
  json.concat(String(WiFi.RSSI()));
  json.concat(F("\" , \"wifi_SSID\":\""));
  json.concat(WiFi.SSID());
  json.concat(F("\" , \"wifi_IP\":\""));
  json.concat(WiFi.localIP().toString());
  json.concat(F("\" , \"wifi_MAC_address\":\""));
  json.concat(WiFi.macAddress());
   
  json.concat(F("\" , \"wifi_DNSIP1\":\""));
  json.concat(WiFi.dnsIP(0).toString());
  json.concat(F("\" , \"wifi_DNSIP2\":\""));            
  json.concat(WiFi.dnsIP(1).toString());
  json.concat(F("\" , \"wifi_DNSIP3\":\""));            
  json.concat(WiFi.dnsIP(2).toString());

  return json;
}

// ----------------------------------------------------------------------------