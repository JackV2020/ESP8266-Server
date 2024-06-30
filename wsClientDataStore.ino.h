/* ----------------------------------------------------------------------------

INOClientExampleDataStore.h

  An example created by wsServerConfigureWebSocketServer when it does not exist.

---------------------------------------------------------------------------- */

const char INOClientExampleDataStore[] PROGMEM = R"rawliteral(/*

Example to save data in and get data from the DataStore in the WebSocket server.

This example uses WebSocketsClient.h by Markus Sattler
Installed via
 - library manager
 - or zipfile from https://github.com/Links2004/arduinoWebSockets

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>

const char* ssid = "WIFISSID";
const char* password = "WIFIPASSWORD";
const char * host = "192.168.2.26"; // The IP of theESP8266  websocket server
uint16_t port = 80;
const char * url = "/wsServer/ws";

// Initialize the WebSocket client
WebSocketsClient webSocket;

// Variables to track connection state
bool isConnected = false;
unsigned long reconnectDelay = 5000;  // millisecs between reconnection attempts
unsigned long lastReconnectAttempt = 0;

// minimal length of SEPERATOR is 2
#define SEPERATOR "||"
#define SEPERATORSIZE 2

void sendSaveData(String name, String value) {
 if (isConnected) {
  String msg = F("21||") + name + SEPERATOR +value;
  webSocket.sendTXT(msg);
  Serial.println(F("Sent saveData request: ") + msg);
 } else {
  Serial.println(F("Not connected to server."));
 }
}

void sendFindData(String name) {
 if (isConnected) {
  String msg = F("22||")+name;
  webSocket.sendTXT(msg);
  Serial.println(F("Sent saveData request: ") + msg);
 } else {
  Serial.println(F("Not connected to server."));
 }
}

void sendWipeData(String name) {
 if (isConnected) {
  String msg = F("23||")+name;
  webSocket.sendTXT(msg);
  Serial.println(F("Sent saveData request: ") + msg);
 } else {
  Serial.println(F("Not connected to server."));
 }
}

void sendListKeys() {
 if (isConnected) {
  String msg = F("20");
  webSocket.sendTXT(msg);
  Serial.println(F("Sent saveData request: ") + msg);
 } else {
  Serial.println(F("Not connected to server."));
 }
}

String wsServerGetPart(const String &string, uint8_t part) {

  String result=F("*");
  int startIndex;
  int endIndex = - SEPERATORSIZE ; // '-length' of seperator

  for (uint8_t i = 0; i < part; i++) {
    if (endIndex == -1) { // part is missing so we return NULL
      result = "";
      break;
    }
    startIndex = endIndex + SEPERATORSIZE;
    endIndex = string.indexOf(SEPERATOR, startIndex);
  }
  if (result == F("*")) {
    if (endIndex == -1) {
      result = string.substring(startIndex);
    } else {
      result = string.substring(startIndex, endIndex);
    }
  }
  return result;
}

void webSocketEventHandleResponse (const String &response){
  Serial.println(F("Received response: ") + response);
  uint8_t wsServerTT = wsServerGetPart(response, 1).toInt();
  if ( (wsServerTT > 19) && (wsServerTT < 30) ) {
    Serial.println(F("Do something with wsServerTT: ")+String(wsServerTT));
  }
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
 switch (type) {
  case WStype_CONNECTED:
   Serial.println(F("Connected to WebSocket server!"));
   isConnected = true;
   break;
  case WStype_DISCONNECTED:
   Serial.println(F("Disconnected from WebSocket server."));
   isConnected = false;
   break;
  case WStype_TEXT: {
   // Received a text message
   String response = (char*)payload;
   webSocketEventHandleResponse(response);
   break;
  }
  default:
   // Other WebSocket events
   break;
 }
}

void setup() {
 Serial.begin(115200);

 Serial.println(F("Connecting to Wi-Fi..."));
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(F("."));
 }
 Serial.println(F("Connected to Wi-Fi!"));

// Set up WebSocket client
 webSocket.begin(host, port, url);
 webSocket.onEvent(webSocketEvent);
}

int DataStoreTimer = 0;
unsigned long currentMillis;

void loop() {
// Continuously handle WebSocket events
 webSocket.loop();

// Check if the client is disconnected
 if (!isConnected) {
 // Attempt to reconnect after a delay
  currentMillis = millis();
  if (currentMillis - lastReconnectAttempt > reconnectDelay) {
   Serial.println("Attempting to reconnect to WebSocket server...");
    webSocket.begin(host, port, url);
   lastReconnectAttempt = currentMillis;
  }
 }

// Some ws DataStore testing
 if (millis() > DataStoreTimer + 5000 ) {
  Serial.println(F("-----------------"));
  sendFindData(F("test"));
  sendSaveData(F("test"), String(millis()));
  sendFindData(F("test"));
  sendListKeys();
//  sendWipeData(F("test"));
  DataStoreTimer=millis();
 }
}
)rawliteral";

// ----------------------------------------------------------------------------