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
/*

  I use logicals to enable/disable Serial.print... statements

 This means you can find statements like   
    "CfgMgr_debug1 && Serial.print..." 
    "CfgMgr_debug2 && Serial.print..." 

  These logocals are local to each .ino section

*/

bool CfgMgr_debug1 = true; // debug messages type 1
//bool CfgMgr_debug2 = true; // debug messages type 2

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
  if(!file){ CfgMgr_debug1 && Serial.println(F("- failed to open file for writing")); return; }
  if(!file.print(message)){ CfgMgr_debug1 && Serial.println(F("- frite failed")); }
  file.close();
}

// =====  Save all settings and use ecryption for some.

void  CfgMgrSaveConfig() {

      CfgMgr_WriteFile( CfgMgrWiFihostnamePath,
        CfgMgr_sEncDec(CfgMgrWiFihostname).c_str());
      CfgMgr_WriteFile( CfgMgrWiFissidPath,
        CfgMgr_sEncDec(CfgMgrWiFissid).c_str());
      if (CfgMgrWiFipass != F("") ) {
        CfgMgr_WriteFile( CfgMgrWiFipassPath,
          CfgMgr_sEncDec(CfgMgrWiFipass).c_str());
      };

      CfgMgr_WriteFile( CfgMgruserPath,
        CfgMgr_sEncDec(CfgMgruser).c_str());
      if (CfgMgrpassword != F("") ) {
        CfgMgr_WriteFile( CfgMgrpasswordPath,
          CfgMgr_sEncDec(CfgMgrpassword).c_str());
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

  String tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgruserPath));
  if (tmpstr !=F("") ) {CfgMgruser = tmpstr ;};

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrpasswordPath));
  if (tmpstr !=F("") ) {CfgMgrpassword = tmpstr ;};

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrWiFissidPath));
  if (tmpstr !=F("") ) {CfgMgrWiFissid = tmpstr ;};

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrWiFipassPath));
  if (tmpstr !=F("") ) {CfgMgrWiFipass = tmpstr ;};

  tmpstr = CfgMgr_sEncDec(CfgMgr_ReadFile( CfgMgrWiFihostnamePath));
  if (tmpstr !=F("") ) {CfgMgrWiFihostname = tmpstr ;};

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

  LittleFS.remove(CfgMgruserPath);
  LittleFS.remove(CfgMgrpasswordPath);

  LittleFS.remove(F("/index.html"));
  LittleFS.remove(F("/index.html.gz"));

  LittleFS.remove(F("/LittleFsWeb/LittleFsWeb.html"));
  LittleFS.remove(F("/LittleFsWeb/LittleFsWeb.html.gz"));

  delay(1000);
  ESP.restart();
}

// ====== Check authentication

bool CfgMgrCheckLogon(AsyncWebServerRequest *request) {
/*
  if (!request->authenticate(CfgMgruser.c_str(), CfgMgrpassword.c_str())) {
    request->requestAuthentication();
    return false;
  }
  return true;
*/
return request->authenticate(CfgMgruser.c_str(), CfgMgrpassword.c_str());
}

// =====  Configure web server

void CfgMgrConfigureWebServer(AsyncWebServer &server, String &username, String &password) {

  CfgMgr_debug1 && Serial.println(F("Configuring ConfigurationManager ..."));

  CfgMgruser = username;
  CfgMgrpassword = password;  

  server.on("/ConfigurationManager", HTTP_ANY, CfgMgrActions); // GET and POST

  // Logs you out

  server.on("/CfgMgr/logout", HTTP_GET, CfgMgrLogout);

}
// ====== Logout

void CfgMgrLogout(AsyncWebServerRequest *request) {
  request->requestAuthentication();
  request->send(401);
}

// =====  Processor for the Configuration Manager function below

String CfgMgrProcessor(const String& var) {

  if (var == F("CfgMgrWiFihostname"))    { return CfgMgrWiFihostname;  }
  if (var == F("CfgMgrWiFissid"))        { return CfgMgrWiFissid; }

  if (var == F("CfgMgruser"))            { return CfgMgruser; }

  if (var == F("CfgMgrNAPTssid"))        { return CfgMgrNAPTssid; }
  if (var == F("CfgMgrNAPTnet"))         { return CfgMgrNAPTnet; }
  if (var == F("CfgMgrNAPTDNS"))         { return CfgMgrNAPTDNS; }

  return String();
}

// ======  handle Configuration Manager

void CfgMgrActions(AsyncWebServerRequest *request) {

  if (CfgMgrCheckLogon(request)) {
    int params = request->params();
    if(params == 0){ // There is a GET so show the Configuration Management page
        request->send(200, "text/html", CfgMgr_html, CfgMgrProcessor);
    } else {  // This is the POST of the Configuration Management page
      for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST all other values
          if (p->name() == F("CfgMgrWiFihostname"))
            { CfgMgrWiFihostname        = p->value().c_str(); }
          if (p->name() == F("CfgMgrWiFissid"))
            { CfgMgrWiFissid            = p->value().c_str(); }
          if (p->name() == F("CfgMgrWiFipass"))
            { CfgMgrWiFipass            = p->value().c_str(); }
          if (p->name() == F("CfgMgruser"))
            { CfgMgruser                = p->value().c_str(); }
          if (p->name() == F("CfgMgrpassword"))
            { CfgMgrpassword            = p->value().c_str(); }
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
      CfgMgrSaveConfig();
      request->send(200, "text/html", CfgMgr_Saved);
      CfgMgrRestartTicker.attach(1, ESP.restart);
    }
  } else {
    CfgMgr_debug1 && Serial.println(F("Authentication Failed"));
    return request->requestAuthentication();
  }
}

// ----------------------------------------------------------------------------
