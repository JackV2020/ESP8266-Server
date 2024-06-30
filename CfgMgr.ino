/* ----------------------------------------------------------------------------

CfgMgr.ino : A web page to save configuration settings

  For your main app :

  You need to include CfgMgr.h in your main ino file so you can use its data.

  After creating your webserver named MyWebServer with :
    AsyncWebServer MyWebServer(80);         // Create web server
  call :
    CfgMgrConfigureWebServer(MyWebServer);  // Provide /ConfigurationManager

  Change settings on /ConfigurationManager, anter the password and click Save.
  This saves the settings and starts a reboot after 3 seconds via a Ticker.


  NOTES :

    When you want to add your own settings fields to CfgMgr
      like I did for LittleFSWeb and NAPT :

      1 CfgMgr.h is where you:
        define your settings
        and where to save them
        find a form with the fields you see below

      2 CfgMgr.ino is where you:
        update CfgMgrActions to process settings
        update CfgMgrProcessor to insert values
        update CfgMgrReadConfig
        update CfgMgrSaveConfig

Ticker.h is part of the esp8266 core for Arduino environment.
CfgMgr.h is the file which goes with this CfgMgr.ino

---------------------------------------------------------------------------- */

#include <Ticker.h>

Ticker CfgMgrRestartTicker;

// ===== Symmetric Encryption Decryption

String CfgMgr_sEncDec(String input) {
// Symmetric encryption uses same keys and algorithm to encrypt and decrypt
  String result=F("");
// Buffer to convert 1 string character of 'input' to char
// (length 2 bytes byte0=value, byte1=0) 
  char temp[2];
// Generate keys with values from 128 to 256 based on the chip id of the ESP
  char enc0, enc1, enc2, enc3;
  enc0 = ESP.getChipId()%124 + 128; // max value right after % is 128
  enc1 = ESP.getChipId()%121 + 128; // max value right after % is 128
  enc2 = ESP.getChipId()%127 + 128; // max value right after % is 128
  enc3 = ESP.getChipId()%111 + 128; // max value right after % is 128
  for (int i = 0 ; i < input.length(); i++ ) {
    input.substring(i,i+1).toCharArray(temp, 2);// put next character in array
    switch (i % 4) {
      case 0: temp[0] = temp[0] ^ enc0; break;  // XoR the character with key 0
      case 1: temp[0] = temp[0] ^ enc1; break;  // XoR the character with key 1
      case 2: temp[0] = temp[0] ^ enc2; break;  // XoR the character with key 2
      case 3: temp[0] = temp[0] ^ enc3; break;  // XoR the character with key 3
    }
    result.concat(String(temp[0]));     // append it to the result
  }
  return result;
}

// ===== Read File

String CfgMgr_ReadFile( const char * path){
  File file = LittleFS.open(path, "r");
  if(!file || file.isDirectory()){ return String(); }
  String fileContent = file.readString();
  file.close();
  return fileContent;
}

// ===== Write file

void CfgMgr_WriteFile(const char * path, const char * message){
  File file = LittleFS.open(path, "w");
  if(!file){ _SERIAL_PRINTLN(F("- failed to open file for writing")); return; }
  if(!file.print(message)){ _SERIAL_PRINTLN(F("- frite failed")); }
  file.close();
}

// =====  Save all settings and use ecryption for some.

void  CfgMgrSaveConfig(String newpassCFGMGR) {

      CfgMgr_WriteFile( CfgMgrWiFihostnamePath,
        CfgMgr_sEncDec(CfgMgrWiFihostname).c_str());
      CfgMgr_WriteFile( CfgMgrWiFissidPath,
        CfgMgr_sEncDec(CfgMgrWiFissid).c_str());
      if (CfgMgrWiFipass != F("") ) {
        CfgMgr_WriteFile( CfgMgrWiFipassPath,
          CfgMgr_sEncDec(CfgMgrWiFipass).c_str());
      };

      CfgMgr_WriteFile( CfgMgrLittleFSWebuserPath,
        CfgMgr_sEncDec(CfgMgrLittleFSWebuser).c_str());
      if (CfgMgrLittleFSWebpassword != F("") ) {
        CfgMgr_WriteFile( CfgMgrLittleFSWebpasswordPath,
          CfgMgr_sEncDec(CfgMgrLittleFSWebpassword).c_str());
      };

      if (newpassCFGMGR != F("") ) {
        CfgMgr_WriteFile( CfgMgrpassPath,
          CfgMgr_sEncDec(newpassCFGMGR).c_str());
      };

      CfgMgr_WriteFile( CfgMgrNAPTssidPath,
        CfgMgr_sEncDec(CfgMgrNAPTssid).c_str());
      if (CfgMgrNAPTpass != F("") ) {
        CfgMgr_WriteFile( CfgMgrNAPTpassPath,
          CfgMgr_sEncDec(CfgMgrNAPTpass).c_str());
      };
      CfgMgr_WriteFile( CfgMgrNAPTnetPath,
        CfgMgr_sEncDec(CfgMgrNAPTnet).c_str());
      CfgMgr_WriteFile( CfgMgrNAPTDNSPath,
        CfgMgr_sEncDec(CfgMgrNAPTDNS).c_str());

}

// =====  Read saved settings and decrypt some

void  CfgMgrReadConfig() {

  String tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrpassPath));
  if (tmpstr !=F("") ) {CfgMgrpass = tmpstr ;};

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrWiFissidPath));
  if (tmpstr !=F("") ) {CfgMgrWiFissid = tmpstr ;};

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrWiFipassPath));
  if (tmpstr !=F("") ) {CfgMgrWiFipass = tmpstr ;};

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrWiFihostnamePath));
  if (tmpstr !=F("") ) {CfgMgrWiFihostname = tmpstr ;};

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrLittleFSWebuserPath));
  if (tmpstr !=F("") ) {CfgMgrLittleFSWebuser = tmpstr ;}

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrLittleFSWebpasswordPath));
  if (tmpstr !=F("") ) {CfgMgrLittleFSWebpassword = tmpstr ;}

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrNAPTssidPath));
  if (tmpstr !=F("") ) {CfgMgrNAPTssid = tmpstr ;};

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrNAPTpassPath));
  if (tmpstr !=F("") ) {CfgMgrNAPTpass = tmpstr ;};

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrNAPTnetPath));
  if (tmpstr !=F("") ) {CfgMgrNAPTnet = tmpstr ;};

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrNAPTDNSPath));
  if (tmpstr !=F("") ) {CfgMgrNAPTDNS = tmpstr ;};

}

// =====  Reset configuration ( activated in loop when D8 is 3.3 Volts)

void CfgMgrResetConfig() {

  LittleFS.remove(CfgMgrpassPath);
  LittleFS.remove(CfgMgrLittleFSWebuserPath);
  LittleFS.remove(CfgMgrLittleFSWebpasswordPath);

  LittleFS.remove(F("/index.html"));
  LittleFS.remove(F("/index.html.gz"));

  LittleFS.remove(F("/LittleFsWeb/LittleFsWeb.html"));
  LittleFS.remove(F("/LittleFsWeb/LittleFsWeb.html.gz"));

  delay(1000);
  ESP.restart();
}

// =====  Configure web server

void CfgMgrConfigureWebServer(AsyncWebServer &server) {

  _SERIAL_PRINTLN(F("Configuring ConfigurationManager ..."));

  server.on("/ConfigurationManager", HTTP_ANY, CfgMgrActions); // GET and POST

}

// =====  Processor for the Configuration Manager function below

String CfgMgrProcessor(const String& var) {

  if (var == F("LANIP")) {
    if (WiFi.status() == WL_CONNECTED) {
      return (F("My IP address on ") + WiFi.SSID() +
              F(" is ") + WiFi.localIP().toString());
    } else {
      return F("Connect to WiFi and find the IP of this device on this page.");
    }
  }

  if (var == F("INFOIPADDRESS")) {
    if (WiFi.status() == WL_CONNECTED) {
      return (WiFi.localIP().toString());
    } else {
      return F("192.168.4.1");
    }
  }

  if (var == F("CfgMgrWiFihostname"))    { return CfgMgrWiFihostname;  }
  if (var == F("CfgMgrWiFissid"))        { return CfgMgrWiFissid; }

  if (var == F("CfgMgrLittleFSWebuser")) { return CfgMgrLittleFSWebuser; }

  if (var == F("CfgMgrNAPTssid"))        { return CfgMgrNAPTssid; }
  if (var == F("CfgMgrNAPTnet"))         { return CfgMgrNAPTnet; }
  if (var == F("CfgMgrNAPTDNS"))         { return CfgMgrNAPTDNS; }

  return String();
}

// ======  handle Configuration Manager

void CfgMgrActions(AsyncWebServerRequest *request) {

  int params = request->params();
  if(params == 0){ // There is a GET so show the Configuration Management page
      request->send_P(200, "text/html", CfgMgr_html, CfgMgrProcessor);
  } else {  // This is the POST of the Configuration Management page
    String passCFGMGR, newpassCFGMGR;
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST passCFGMGR value
        if (p->name() == F("passCFGMGR")) { passCFGMGR = p->value().c_str(); }
      }
    }
    if (passCFGMGR == CfgMgrpass) { // passCFGMGR == right password
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST all other values
          if (p->name() == F("CfgMgrWiFihostname"))
            { CfgMgrWiFihostname        = p->value().c_str(); }
          if (p->name() == F("CfgMgrWiFissid"))
            { CfgMgrWiFissid            = p->value().c_str(); }
          if (p->name() == F("CfgMgrWiFipass"))
            { CfgMgrWiFipass            = p->value().c_str(); }
          if (p->name() == F("CfgMgrLittleFSWebuser"))
            { CfgMgrLittleFSWebuser     = p->value().c_str(); }
          if (p->name() == F("CfgMgrLittleFSWebpassword"))
            { CfgMgrLittleFSWebpassword = p->value().c_str(); }
          if (p->name() == F("newpassCFGMGR"))
            { newpassCFGMGR             = p->value().c_str(); }
          if (p->name() == F("CfgMgrNAPTssid"))
            { CfgMgrNAPTssid            = p->value().c_str(); }
          if (p->name() == F("CfgMgrNAPTpass"))
            { CfgMgrNAPTpass            = p->value().c_str(); }
          if (p->name() == F("CfgMgrNAPTnet"))
            { CfgMgrNAPTnet             = p->value().c_str(); }
          if (p->name() == F("CfgMgrNAPTDNS"))
            { CfgMgrNAPTDNS             = p->value().c_str(); }
        }
      }
// Save data and restart
      CfgMgrSaveConfig(newpassCFGMGR);
      request->send(200, "text/html", CfgMgr_Saved);
      CfgMgrRestartTicker.attach(1, ESP.restart);
    } else {
// Wrong password so send back to CfgMgr screen
      request->send(200, "text/html", CfgMgr_Wrong_Password);
    }
  }
}

// ----------------------------------------------------------------------------
