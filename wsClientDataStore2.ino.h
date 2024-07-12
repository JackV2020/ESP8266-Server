/* ----------------------------------------------------------------------------

wsClientDataStore2.ino.h

  An example created by wsServerConfigureWebSocketServer when it does not exist.

---------------------------------------------------------------------------- */

const char INOClientExampleDataStore2[] PROGMEM = R"rawliteral(/*

Example to save, get and delete data in the DataStore in the websocket server.

I created this extra example because wsClientDataStore.ino comes with
compiler warnings althoug it uses a library which can be installed in
Arduino IDE via library manager.

Just change 'WiFi Settings' below, compile and run.

When running this example you will see some errors because data does not exist.
This is for demo purpose. When you read the code below you will get it.

This example uses WebSocketsClient.h by Markus Sattler
Installed via zipfile from https://github.com/Links2004/arduinoWebSockets

I included the same webserver as used by the websocket Server to show you can.
When you have the websocket server running you already did the next :
Install 'ESPAsyncTCP' by dvarrel and 'ESP Async WebServer' by Me-No-Dev
via the library manager. Note the spaces in the name 'ESP Async WebServer'

*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsClient.h>

// ----- Control printing of console messages

bool debug_send = false;
bool debug_receive = true;

// -------------------------------------------------------------- WiFi Settings

const char* ssid = "WiFiSSID";          // Your home WiFi name
const char* password = "WiFiPassword";  // Your home WiFi password

// ----------------------------------------------------------------- Web Server

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer WebServer(80);

void notFound(AsyncWebServerRequest* request) {
  Serial.println(F("Client:")
                 + request->client()->remoteIP().toString()
                 + F(" ") + request->url());
  request->send(404, "text/plain", F("Not found"));
}

void HomePage(AsyncWebServerRequest* request) {
  String html_response = F("Hello World!");
  request->send(200, "text/html", html_response);
}

// ----------------------------------------------------------- WebSocket Client

WebSocketsClient webSocket;

// ----- Websocket server details

const char* host = "192.168.2.26";
const uint16_t port = 80;
const char* url = "/wsServer/ws";

// ----- Variables to track connection state

bool isConnected = false;
unsigned long reconnectDelay = 5000;  // millisecs between reconnection attempts
unsigned long lastReconnectAttempt = 0;

// ----- WsDataStore variables

// Seperator varaibles for Datastore are the same as in the websocket server

#define SEPERATOR "||"
#define SEPERATORSIZE 2

int DataStoreTimer = 0;
unsigned long currentMillis;

// ----- Functions to send data to the websocket server

void sendSaveData(String name, String value) {
  if (isConnected) {
    String msg = F("21||") + name + SEPERATOR + value;
    webSocket.sendTXT(msg);
    debug_send&& Serial.println(F("Sent saveData request: ") + msg);
  } else {
    Serial.println(F("Not connected to server."));
  }
}

void sendFindData(String name) {
  if (isConnected) {
    String msg = F("22||") + name;
    webSocket.sendTXT(msg);
    debug_send&& Serial.println(F("Sent findData request: ") + msg);
  } else {
    Serial.println(F("Not connected to server."));
  }
}

void sendWipeData(String name) {
  if (isConnected) {
    String msg = F("23||") + name;
    webSocket.sendTXT(msg);
    debug_send&& Serial.println(F("Sent wipeData request: ") + msg);
  } else {
    Serial.println(F("Not connected to server."));
  }
}

void sendListKeys() {
  if (isConnected) {
    String msg = F("20");
    webSocket.sendTXT(msg);
    debug_send&& Serial.println(F("Sent listKeys request: ") + msg);
  } else {
    Serial.println(F("Not connected to server."));
  }
}

// ----- Functions to receive data from the websocket server

String wsServerGetPart(const String& string, uint8_t part) {
  // string is like 22||variable||value....
  // part is index 1,2,...
  String result = F("*");
  int startIndex;
  int endIndex = -SEPERATORSIZE;  // '-length' of seperator

  for (uint8_t i = 0; i < part; i++) {
    if (endIndex == -1) {  // part is missing so we return NULL
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

void webSocketEventHandleResponse(const String& response) {

  uint8_t wsServerTT = wsServerGetPart(response, 1).toInt();
  switch (wsServerTT) {
    case 20:
      debug_receive&& Serial.println(F("Received key list: ") + response);
      break;
    case 21:
      debug_receive&& Serial.println(F("Received save reply: ") + response);
      break;
    case 22:
      debug_receive&& Serial.println(F("Received find reply: ") + response
                                     + F("; variable: ") + wsServerGetPart(response, 2)
                                     + F("; value: ") + wsServerGetPart(response, 3));
      break;
    case 23:
      debug_receive&& Serial.println(F("Received wipe confirm of: ") + response);
      break;
    case 29:
      debug_receive&& Serial.println(F("Received Error: ") + response);
      break;
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
    case WStype_TEXT:
      {
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

// ---------------------------------------------------------------------- Setup

void setup() {
  Serial.begin(115200);

  // ----- WiFi

  Serial.println(F("Connecting to Wi-Fi..."));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F("Connected to Wi-Fi and my IP is : ") + WiFi.localIP().toString());

  // ----- Websocket Client

  webSocket.onEvent(webSocketEvent);

  // ----- Web Server

  WebServer.onNotFound(notFound);

  WebServer.on("/", HTTP_GET, HomePage);

  WebServer.begin();

  // ----- Setup complete

  Serial.println(F("Setup complete"));
}

// ----------------------------------------------------------------------- Loop

void loop() {

  // Continuously handle WebSocket events

  webSocket.loop(); delay(500);

  // Check if the client is disconnected

  if (isConnected) {

    // ----- Some ws DataStore testing

    if (millis() > DataStoreTimer + 10000) {
      Serial.println(F("-----------------"));
      sendListKeys();
      sendFindData(F("BootTime"));
      sendSaveData(F("test"), String(millis()));
      sendFindData(F("test"));
      sendListKeys();
      delay(5000);
      sendWipeData(F("test"));
      sendFindData(F("test"));
      DataStoreTimer = millis();
    }
  } else {

    // ----- Attempt to (re)connect after a delay

    currentMillis = millis();
    if (currentMillis - lastReconnectAttempt > reconnectDelay) {
      Serial.println(F("Attempting to (re)connect to WebSocket server..."));
      webSocket.begin(host, port, url);
      lastReconnectAttempt = currentMillis;
    }
  }
}
)rawliteral";

// ----------------------------------------------------------------------------