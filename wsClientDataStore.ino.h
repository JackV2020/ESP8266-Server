/* ----------------------------------------------------------------------------

wsClientDataStore.ino.h

  An example created by wsServerConfigureWebSocketServer when it does not exist.

---------------------------------------------------------------------------- */

const char INOClientExampleDataStore[] PROGMEM = R"rawliteral(/*

Example to save, get and delete data in the DataStore in the websocket server.

Just change 'WiFi Settings' below, compile and run.

When running this example you will see some errors because data does not exist.
This is for demo purpose. When you read the code below you will get it.

This example uses ArduinoWebsockets by Gil Maimon
Installed via library manager

I included the same webserver as used by the websocket server to show you can.
When you have the websocket server running you already did the next :
Install 'ESPAsyncTCP' by dvarrel and 'ESP Async WebServer' by Me-No-Dev
via the library manager. Note the spaces in the name 'ESP Async WebServer'

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoWebsockets.h>

using namespace websockets;

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

WebsocketsClient webSocket;

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

void sendListKeys() {
  if (isConnected) {
    String msg = F("20");
    webSocket.send(msg);
    debug_send && Serial.println(F("Sent listKeys request: ") + msg);
  } else {
    Serial.println(F("Not connected to server."));
  }
}

void sendSaveData(String name, String value) {
  if (isConnected) {
    String msg = F("21||") + name + SEPERATOR + value;
    webSocket.send(msg);
    debug_send && Serial.println(F("Sent saveData request: ") + msg);
  } else {
    Serial.println(F("Not connected to server."));
  }
}

void sendFindData(String name) {
  if (isConnected) {
    String msg = F("22||") + name;
    webSocket.send(msg);
    debug_send && Serial.println(F("Sent findData request: ") + msg);
  } else {
    Serial.println(F("Not connected to server."));
  }
}

void sendWipeData(String name) {
  if (isConnected) {
    String msg = F("23||") + name;
    webSocket.send(msg);
    debug_send && Serial.println(F("Sent wipeData request: ") + msg);
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

void onMessageCallback(WebsocketsMessage message) {

  uint8_t wsServerTT = wsServerGetPart(message.data(), 1).toInt();
  switch (wsServerTT) {
    case 20:
      debug_receive && Serial.println(F("Received key list: ") + message.data());
      break;
    case 21:
      debug_receive && Serial.println(F("Received save reply: ") + message.data());
      break;
    case 22:
      debug_receive && Serial.println(F("Received find reply: ") + message.data()
                     + F("; variable: ") + wsServerGetPart(message.data(), 2)
                     + F("; value: ") + wsServerGetPart(message.data(), 3));
      break;
    case 23:
      debug_receive && Serial.println(F("Received wipe confirm of: ") + message.data());
      break;
    case 29:
      debug_receive && Serial.println(F("Received Error: ") + message.data());
      break;
  }
}

void onEventsCallback(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    isConnected = true;
    Serial.println(F("Connected to WebSocket server!"));
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    isConnected = false;
    Serial.println(F("Disconnected from WebSocket server."));
 } else if (event == WebsocketsEvent::GotPing) {
    Serial.println(F("Got a Ping!"));
  } else if (event == WebsocketsEvent::GotPong) {
    Serial.println(F("Got a Pong!"));
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

  webSocket.onMessage(onMessageCallback);
  webSocket.onEvent(onEventsCallback);

  // ----- Web Server

  WebServer.onNotFound(notFound);

  WebServer.on("/", HTTP_GET, HomePage);

  WebServer.begin();

  // ----- Setup complete

  Serial.println(F("Setup complete"));
}

// ----------------------------------------------------------------------- Loop

void loop() {

  // ----- Check if the client is connected

  if (isConnected) {

    // ----- Let the websocket client check for incoming messages

    if (webSocket.available()) {
      webSocket.poll();
    }

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
      webSocket.connect(host, port, url);
      lastReconnectAttempt = currentMillis;
    }
  }
}
)rawliteral";

// ----------------------------------------------------------------------------