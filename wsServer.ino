/* ----------------------------------------------------------------------------

wsServer.ino : Implement Websocket Server with 2 applications and 3 examples.

  For your main app :

  Before including ESPAsyncWebServer.h you can define the maximum number
    of clients that you want to allow like :

    #define DEFAULT_MAX_WS_CLIENTS 6

    See 'sketchbook'/libraries/ESPAsyncWebServer/src/AsyncWebSocket.h for
      default values for ESP and ESP8266.
    After the maximum number of clients :

    #include <ESPAsyncTCP.h>
    #include <ESPAsyncWebServer.h>

  After creating your webserver named MyWebServer with :
    AsyncWebServer MyWebServer(80);               // Create web server
  call:
    wsServerConfigureWebSocketServer(WebServer);  // Provides Websocket Server

  NOTES:
  1 Messages to clients.
    The 2 standard ways to update clients are :
      - 'update requesting client'
      - 'update all clients'
    Since I have 2 applications I only want to :
      - 'update the group of clients that registered for that application'
      See the section 'Update Clients Group' below
  2 Applications
    Groupnumber 10 lets you control GPIO's,
      see section 'APP GPIOs'
    Groupnumber 20 lets you share data between clients,
      see section 'wsServer DataStore'
    Groupnumber 30 is expected to serve email function,
      nothing there yet...8-)
  3 Examples
    There is
    - 1 html example to control 6 GPIOs and react on interrupts
    - 1 html example to use the DataStore ( use more browsers to share data )
    - 1 ino example to use the DataStore ( shares data with the html example )

---------------------------------------------------------------------------- */

// ---------------------------------------------------------------- The examples

/*

  I use logicals to enable/disable Serial.print... statements

 This means you can find statements like   
    "wss_debug0 && Serial.print..." 
    "wss_debug2 && Serial.print..." 

  These logocals are local to each .ino section

*/

bool wss_debug0 = true; // debug messages type 0 general
bool wss_debug1 = true; // debug messages type 1 app GPIO
bool wss_debug2 = true; // debug messages type 2 app DataStore
bool wss_debug3 = true; // debug messages type 2 app Email ( not there yet )

#include "wsClientDataStore.html.h"
#include "wsClientDataStore.ino.h"
#include "wsClientDataStore2.ino.h"
#include "wsClientGPIOs.html.h"

#define wsClientDataStoreHTMLPath "/zExamples/wsClientDataStore.html"
#define wsClientDataStoreINOPath "/zExamples/wsClientDataStore.ino"
#define wsClientDataStoreINOPath2 "/zExamples/wsClientDataStore2.ino"
#define wsClientGPIOsHTMLPath "/zExamples/wsClientGPIOs.html"

// ------------------------------------------------------------ WebSocket Server

AsyncWebSocket wsServer("/wsServer/ws");

// The seperator used by all transactions
// Minimal length of SEPERATOR is 2
#define SEPERATOR "||"
#define SEPERATORSIZE 2

// -------------------------------------------------------- Update Clients Group

/*
  The standard way to update clients is to either update 1 or all.

  Another way is to only updates those which are connected to an application.

  To support this we keep a map between clients and the group they belong to.

*/

#include <map>

/*

Create a map to keep track of client groups, which is like a

'2 dimensional array' uses client as 'index' ; int value is group number

*/

std::map<AsyncWebSocketClient*, int> wsServerClientGroups;

// Function to send updates to specific group

IRAM_ATTR void wsServerUpdateGroup(int group, const String& message) {
/*
  Pseudo code :

  for (each 'index' + 'int value' in wsServerClientGroups ) :
    if (2nd element is group number) :
      send message to 1st element

*/

  for (auto const& clientPair : wsServerClientGroups) {
    if (clientPair.second == group) {
      clientPair.first->text(message);
    }
  }
}

// ------------------------------------------------------------------- APP GPIOs

#define wsServerGPIODataPath "/wsServer/GPIOData.dat"

/*

  The Websocket Server serves access to the 6 GPIOs :
    2, 4, 5, 12, 13 and 14 with interupts for all but 2
    also hass option to broadcast values ( currently A0 only)

  - Transactions 10-19 :
  10  Ask complete status overview
    used by client start like web page load
    if not registerd already, registers client in group 10,
    reply is reply 100 ( see below )
  11||GPIOnr      Set mode INPUT          (interrupt sends 101 to all clients)
  12||GPIOnr      Set mode OUTPUT
  13||GPIOnr      Set mode INPUT_PULLUP   (interrupt sends 101 to all clients)
  14||GPIOnr      Set mode PWM
  15||GPIOnr      Set mode BLINK
  16||A0||value   Attach ticker in milliseconds
                    ( 0 is detach ticker)
                    ( Sends reply 102 to all clients)

  17||GPIOnr||value  Digital mode output. LOW < 128 < HIGH
  18||GPIOnr||value PWM mode output 0..255
  19 is not in use

  Note on transactions above : 
    I could have combined setting modes and controls into somthing like :
    11||gpio||mode    for 11..15
    12||gpio||value   for 17 and 18
    but that gives an extra level in the transactions which I need to split
    and that means extra processing.
    11||gpio||mode or value||setting
    would mean another level and even more processing.

  Replies and Broadcasts :
  100(||GPIO||mode||value)*6||A0 reading||wsServerBroadcastTickerInterval
    - Reply to single client with complete status overview
      ( value = pwm value OR value = 255 * digitalRead result which is 0 or 255)
  101||GPIO||value
    - Broadcast digital reading ( this is caused by interrupt )
  102||A0||value
    - Broadcast changed A0 reading ( this is caused by enabled Ticker )
*/

// GPIO numbers
uint8_t wsServerGPIOnrs[] = { 2, 4, 5, 12, 13, 14 };
// INPUT = 0; OUTPUT = 1; INPUT_PULLUP = 2; PWM = 3; BLINK = 4
uint8_t wsServerGPIOmodes[] = { 1, 2, 2, 2, 2, 2 };
// pwm values 0-255
uint8_t wsServerGPIOpwms[] = { 0, 0, 0, 0, 0, 0 };

// Ticker to broadcast some values ( Now only changed reading of A0 )

#include <Ticker.h>
Ticker wsServerBroadcastTicker;

int wsServerBroadcastTickerInterval;
int wsServerBroadcastTickerAnalogNew = 0;
int wsServerBroadcastTickerAnalogOld = 0;

void wsServerBroadcastTickerRoutine() {
  wsServerBroadcastTickerAnalogNew = analogRead(A0);
  if (wsServerBroadcastTickerAnalogOld != wsServerBroadcastTickerAnalogNew) {
    wsServerBroadcastTickerAnalogOld = wsServerBroadcastTickerAnalogNew;
    wsServerUpdateGroup(10,
                        String(F("102||A0||")
                               + String(wsServerBroadcastTickerAnalogNew)));
  }
}

// ====== wsServerGPIOs interrupt handler for each GPIO

// default is to debounce, better is a 0,1 uF ceramic capacitor over a switch

#define wsServerDebounceMicros 250

unsigned long wsServerLastMicros = 0;

IRAM_ATTR void wsServerGPIOsSwitchInt_4() {
  if (micros() > wsServerLastMicros) {
    String reply = F("101||4||");
    reply.concat(String(digitalRead(4)));
    wsServerUpdateGroup(10, reply);
  }
  wsServerLastMicros = micros() + wsServerDebounceMicros;  // debounce switch
}
IRAM_ATTR void wsServerGPIOsSwitchInt_5() {
  if (micros() > wsServerLastMicros) {
    String reply = F("101||5||");
    reply.concat(String(digitalRead(5)));
    wsServerUpdateGroup(10, reply);
  }
  wsServerLastMicros = micros() + wsServerDebounceMicros;  // debounce switch
}
IRAM_ATTR void wsServerGPIOsSwitchInt_12() {
  if (micros() > wsServerLastMicros) {
    String reply = F("101||12||");
    reply.concat(String(digitalRead(12)));
    wsServerUpdateGroup(10, reply);
  }
  wsServerLastMicros = micros() + wsServerDebounceMicros;  // debounce switch
}
IRAM_ATTR void wsServerGPIOsSwitchInt_13() {
  if (micros() > wsServerLastMicros) {
    String reply = F("101||13||");
    reply.concat(String(digitalRead(13)));
    wsServerUpdateGroup(10, reply);
  }
  wsServerLastMicros = micros() + wsServerDebounceMicros;  // debounce switch
}
IRAM_ATTR void wsServerGPIOsSwitchInt_14() {
  if (micros() > wsServerLastMicros) {
    String reply = F("101||14||");
    reply.concat(String(digitalRead(14)));
    wsServerUpdateGroup(10, reply);
  }
  wsServerLastMicros = micros() + wsServerDebounceMicros;  // debounce switch
}

// Attach interrupt routines to the GPIO pins during startup and mode change.

void wsAttachInterrupt(int GPIO) {
  switch (GPIO) {
    case 4:
      attachInterrupt(digitalPinToInterrupt(4),
                      wsServerGPIOsSwitchInt_4, CHANGE);
      break;
    case 5:
      attachInterrupt(digitalPinToInterrupt(5),
                      wsServerGPIOsSwitchInt_5, CHANGE);
      break;
    case 12:
      attachInterrupt(digitalPinToInterrupt(12),
                      wsServerGPIOsSwitchInt_12, CHANGE);
      break;
    case 13:
      attachInterrupt(digitalPinToInterrupt(13),
                      wsServerGPIOsSwitchInt_13, CHANGE);
      break;
    case 14:
      attachInterrupt(digitalPinToInterrupt(14),
                      wsServerGPIOsSwitchInt_14, CHANGE);
      break;
  }
}

// ---------------------------------------------------------- wsServer DataStore

/*

DataStore :
  - Contains strings like key||value1||value2||....||value'n'
     where || is the SEPERATOR defined above
  - uses 2nd heap shared to store data
      so build with MMU : 16KB cache + 48KB IRAM and 2nd Heap (shared)
  - Transactions : 20 .. 29
  20  Ask complete overview of all keys
    used by client start like web page load or setup() in ino
    if not registerd already, registers client in group 20,
    returns keys seperated by SEPERATOR like 20||name1||name2||...||lastone
  21  - SaveData like : 21||key||value1||value2||....||value'n'
    saves data and returns request : 21||key||value1||value2||....||value'n'
  22  - FindData like 22||key
    finds data and returns 20||key||value1||value2||....||value'n'
  23  - WipeData like 23||key
    removes entry from the DataStore and returns 23||key

  29  - ERROR like 29||ERROR||DataStore is full

*/

#include <umm_malloc/umm_heap_select.h>

// wsServerConfigureWebSocketServer sets next value true when MMU is set ok
// so the code knows 2nd Heap (not shared) is there
bool wsServerIramHeap;

// wsDataStore array
const size_t wsDSNumRows = 100;
char** wsDataStore;

// Function to extract the key part from an array item
String wsDataStoreGetKeyFromRow(const char* item) {
  if (wsServerIramHeap) {
    char* delimiterPos = strstr(item, "||");
    if (delimiterPos != NULL) {
      size_t keyLength = delimiterPos - item;
      return String(item).substring(0, keyLength);
    }
  }
  return "";
}

// Function to allocate an array of strings
void wsDataStoreAllocateArray() {
  umm_set_heap_by_id(1);  // 1 UMM_HEAP_IRAM
  wsDataStore = (char**)malloc(wsDSNumRows * sizeof(char*));
  umm_set_heap_by_id(0);  // 0 UMM_HEAP_DRAM
  if (wsDataStore == NULL) {
    wss_debug0 && Serial.println(F(" ... Failed to allocate memory for wsDataStore array"));
  } else {
    wss_debug0 && Serial.println(F(" ... Created wsDataStore array."));
  }
  // Initialize all pointers to NULL
  for (size_t i = 0; i < wsDSNumRows; i++) {
    wsDataStore[i] = NULL;
  }
}

/*
// Function to deallocate an array of strings, tested, not in use.
void wsDataStoreDeAllocateArray() {
  if (wsDataStore != NULL) {
    umm_set_heap_by_id(1);
    for (size_t i = 0; i < wsDSNumRows; i++) {
      free(wsDataStore[i]);
    }
    free(wsDataStore);
    umm_set_heap_by_id(0);
  }
}
*/

bool wsDataStoreUpdateRow(const char* newStr) {
  if (wsServerIramHeap) {
    String keyOfItem = wsDataStoreGetKeyFromRow(newStr);
    int index = wsDataStoreGetIndexForKey(keyOfItem.c_str());
    // if not found, find free position
    if (index == -1) {
      index = 0;
      while ((index < wsDSNumRows) && (wsDataStore[index] != NULL)) {
        index++;
      }
    }
    // Free the existing string memory if it exists
    if (index < wsDSNumRows) {
      free(wsDataStore[index]);
    } else {
      wss_debug0 && Serial.println(F("wsDataStoreUpdateRow Failed , wsDataStore is full!! "));
      return false;
    }

    // Allocate new memory for the updated string
    size_t length = strlen(newStr) + 1;  // +1 for the null terminator

    umm_set_heap_by_id(1);  // 1
    if (ESP.getMaxFreeBlockSize() > length + 100) {
      wsDataStore[index] = (char*)malloc(length * sizeof(char));
      strcpy(wsDataStore[index], newStr);
    } else {
      wss_debug0 && Serial.println(F("Failed to allocate memory for string"));
      umm_set_heap_by_id(0);  // 0
      return false;
    }

    umm_set_heap_by_id(0);  // 0
    return true;
  } else { return false; }
}

bool wsDSDeleteRow(char** wsDataStore, size_t wsDSNumRows, const char* key) {
  if (wsServerIramHeap) {
    int index = wsDataStoreGetIndexForKey(key);
    if (index > -1) {
      umm_set_heap_by_id(1);  // 1
      free(wsDataStore[index]);
      wsDataStore[index] = (char*)malloc(sizeof(char));
      wsDataStore[index] = NULL;
      umm_set_heap_by_id(0);  // 0
    }
  }
  return true;
}

// Function to find a value by key
String wsDataStoreFindValue(const char* key) {
  if (wsServerIramHeap) {
    //    wss_debug0 && Serial.println("wsDataStoreFindValue"); wss_debug0 && Serial.println(key);
    size_t keyLength = strlen(key);
    for (size_t i = 0; i < wsDSNumRows; i++) {
      if (wsDataStore[i] != NULL) {
        if (strncmp(wsDataStore[i], key, keyLength) == 0
            && wsDataStore[i][keyLength] == '|'
            && wsDataStore[i][keyLength + 1] == '|') {
          // Return the value part of the string
          return String(&wsDataStore[i][keyLength + 2]);
        }
      }
    }
  }
  //    wss_debug0 && Serial.println("wsDataStoreFindValue not found");
  return "";  // Key not found
}

// Function to find the index of a key
int wsDataStoreGetIndexForKey(const char* key) {
  for (size_t i = 0; i < wsDSNumRows; i++) {
    if (wsDataStore[i] != NULL) {
      // Compare keys until the delimiter
      size_t keyLength = strlen(key);
      if (strncmp(wsDataStore[i], key, keyLength) == 0
          && wsDataStore[i][keyLength] == '|'
          && wsDataStore[i][keyLength + 1] == '|') {
        // Key found, return index
        return i;
      }
    }
  }
  return -1;  // Key not found
}

// Function to get all keys concatenated by "||"
String wsDataStoreGetAllKeys() {
  String result = "";
  if (wsServerIramHeap) {
    //  for (size_t i = 0; i < wsDSNumRows; ++i) {
    for (size_t i = 0; i < wsDSNumRows; i++) {
      if (wsDataStore[i] != NULL) {
        // Find the position of the "||" delimiter
        char* delimiterPos = strstr(wsDataStore[i], "||");
        if (delimiterPos != NULL) {
          // Extract the key by copying characters up to the delimiter
          size_t keyLength = delimiterPos - wsDataStore[i];
          if (keyLength > 0) {
            if (result.length() > 0) {
              result += "||";
            }
            result += String(wsDataStore[i]).substring(0, keyLength);
          }
        }
      }
    }
  }
  return result;
}

void heapinfo() {
  if (umm_get_current_heap_id() == 0) {
    wss_debug0 && Serial.print(F(" ... first"));
  } else {
    wss_debug0 && Serial.print(F(" ... secnd"));
  }
  wss_debug0 && Serial.print(F(" heap> FreeHeap: ")
                + String(ESP.getFreeHeap()));
  wss_debug0 && Serial.print(F(" ; HeapFragmentation: ")
                  + String(ESP.getHeapFragmentation()));
  wss_debug0 && Serial.println(F(" ; MaxFreeBlockSize: ")
                  + String(ESP.getMaxFreeBlockSize()));
}

// ------------------------------------------------------- wsServer Server setup

void wsServerConfigurationRead() {
  if (LittleFS.exists(wsServerGPIODataPath)) {
    File file = LittleFS.open(wsServerGPIODataPath, "r");
    for (uint8_t i = 0; i < 6; i++) {
      wsServerGPIOnrs[i] = file.readStringUntil('\n').toInt();
      wsServerGPIOmodes[i] = file.readStringUntil('\n').toInt();
    }
    wsServerBroadcastTickerInterval = file.readStringUntil('\n').toInt();
    file.close();
  }
}

void wsServerConfigurationWrite() {
  File file = LittleFS.open(wsServerGPIODataPath, "w");
  for (uint8_t i = 0; i < 6; i++) {
    file.println(String(wsServerGPIOnrs[i]));
    file.println(String(wsServerGPIOmodes[i]));
    //    file.println(String(wsServerGPIOpwms[i]));
  }
  file.println(String(wsServerBroadcastTickerInterval));
  file.close();
}

void wsServerConfigureWebSocketServer(AsyncWebServer& server) {

  wss_debug0 && Serial.println(F("Configuring Web Socket Server ..."));

  wsServerConfigurationRead();
  /*
Check if we can use 2nd heap to check MMU option
For Heap info see 10.3.2 in :
  https://readthedocs.org/projects/arduino-esp8266/downloads/pdf/stable/
*/
  heapinfo();
  ESP.setIramHeap();
  // (MMU == 16KB cache + 48KB IRAM and 2nd Heap (shared)) ? true : false
  wsServerIramHeap = (umm_get_current_heap_id() == 1);
  if (wsServerIramHeap) { heapinfo(); }
  ESP.setDramHeap();
  if (wsServerIramHeap) {
    wsDataStoreAllocateArray();
  } else {
    wss_debug0 && Serial.print(F(" ... To use wsDataStore... Build with MMU : "));
    wss_debug0 && Serial.println(F("16KB cache + 48KB IRAM and 2nd Heap (shared)"));
  }
  // Set GPIO modes and attach intterupts

  for (int i = 0; i < 6; i++) {

    if ((wsServerGPIOmodes[i] == 0) || (wsServerGPIOmodes[i] == 2)) {
      // mode input || input_pullup
      pinMode(wsServerGPIOnrs[i], wsServerGPIOmodes[i]);
      if (wsServerGPIOnrs[i] != 2) { wsAttachInterrupt(wsServerGPIOnrs[i]); }
    } else {
      // output mode for output, pwm or blink
      pinMode(wsServerGPIOnrs[i], 1);
    }
  }

  // Generate examples when they do not exist

  if (!LittleFS.exists(wsClientGPIOsHTMLPath)) {
    File file = LittleFS.open(wsClientGPIOsHTMLPath, "w");
    file.print(FPSTR(HTMLClientExampleGPIOs));
    file.close();
  }

  if (!LittleFS.exists(wsClientDataStoreHTMLPath)) {
    File file = LittleFS.open(wsClientDataStoreHTMLPath, "w");
    file.print(FPSTR(HTMLClientExampleDataStore));
    file.close();
  }

  if (!LittleFS.exists(wsClientDataStoreINOPath)) {
    File file = LittleFS.open(wsClientDataStoreINOPath, "w");
    file.print(FPSTR(INOClientExampleDataStore));
    file.close();
  }

  if (!LittleFS.exists(wsClientDataStoreINOPath2)) {
    File file = LittleFS.open(wsClientDataStoreINOPath2, "w");
    file.print(FPSTR(INOClientExampleDataStore2));
    file.close();
  }

  // Set websocket event handler
  wsServer.onEvent(wsServerOnEvent);

  // Add the handler to the webserver
  server.addHandler(&wsServer);

  if (wsServerBroadcastTickerInterval > 0) {
    wsServerBroadcastTicker.attach_ms(wsServerBroadcastTickerInterval,
                                      wsServerBroadcastTickerRoutine);
  }
}

// ------------------------------------------------------- Catch clients traffic

void wsServerOnEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                     AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      wss_debug0 && Serial.printf(PSTR("WebSocket client #%u connected from %s\n"),
                        client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      wss_debug0 && Serial.printf(PSTR("WebSocket client #%u disconnected\n"),
                        client->id());
      wsServerClientGroups.erase(client);
      break;
    case WS_EVT_DATA:
      {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len) {
          data[len] = 0;
          String msg = (char*)data;
          wsServerHandleMessage(client, msg);
        }
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

// ---------------------------------------------------- Process incoming traffic

String wsServerGetPart(const String& string, uint8_t part) {

  String result = "*";
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
  if (result == "*") {
    if (endIndex == -1) {
      result = string.substring(startIndex);
    } else {
      result = string.substring(startIndex, endIndex);
    }
  }
  return result;
}


void wsServerHandleMessage(AsyncWebSocketClient* client, const String& msg) {

  uint8_t wsServerTT;
  int index = 0;

  // GPIOs
  uint8_t GPIOnr;  // Transaction GPIO number
  int GPIOvalue;

  // DataStore
  String key;
  String reply;

  wss_debug0 && Serial.println(F("\nReceive: ") + msg);

  wsServerTT = wsServerGetPart(msg, 1).toInt();
  switch (wsServerTT / 10) {
    case 1:
      //      wss_debug1 && Serial.println("-----------GPIO");
      if (wsServerTT == 10) {  // 10 : asks for update

        // Store client's group based on initial message
        if (wsServerClientGroups.find(client) == wsServerClientGroups.end()) {
          wsServerClientGroups[client] = 10;
        }

        reply = F("100");
        for (uint8_t i = i; i < 6; i++) {
          reply.concat(SEPERATOR);
          reply.concat(String(wsServerGPIOnrs[i]));
          reply.concat(SEPERATOR);
          reply.concat(String(wsServerGPIOmodes[i]));
          reply.concat(SEPERATOR);
          /*
          if ( (wsServerGPIOmodes[i] == 0) || (wsServerGPIOmodes[i] == 2) ) {
            reply.concat(String(255 * digitalRead(wsServerGPIOnrs[i])));
          } else {
            reply.concat(String(wsServerGPIOpwms[i]));
          }
*/
          if (wsServerGPIOmodes[i] == 3) {  // pwm mode
            reply.concat(String(wsServerGPIOpwms[i]));
          } else {
            reply.concat(String(255 * digitalRead(wsServerGPIOnrs[i])));
          }
        }
        reply.concat(SEPERATOR);
        reply.concat(String(analogRead(A0)));
        reply.concat(SEPERATOR);
        reply.concat(String(wsServerBroadcastTickerInterval));
        wss_debug1 && Serial.println(F("Reply:   ") + reply);
        client->text(reply);          // 1 Single client only !!!!
      } else if (wsServerTT == 16) {  // Control ticker to broadcast A0 value
        wsServerBroadcastTickerInterval = wsServerGetPart(msg, 3).toInt();
        wsServerBroadcastTicker.detach();
        if (wsServerBroadcastTickerInterval > 0) {
          wsServerBroadcastTicker.attach_ms(wsServerBroadcastTickerInterval,
                                            wsServerBroadcastTickerRoutine);
        }
        wsServerUpdateGroup(10, msg);
        wsServerConfigurationWrite();
      } else {  // Configure GPIO
        GPIOnr = wsServerGetPart(msg, 2).toInt();
        index = 0;
        while ((index < 6) && (wsServerGPIOnrs[index] != GPIOnr)) {
          index += 1;
        }
        if (index < 6) {
          // INPUT = 0; OUTPUT = 1; INPUT_PULLUP = 2; PWM = 3; BLINK = 4
          reply = String(wsServerTT) + SEPERATOR + String(GPIOnr) + SEPERATOR;
          switch (wsServerTT) {
            // INPUT and INPUT_PULLUP
            case 11:
              wsServerGPIOmodes[index] = 0;
              pinMode(GPIOnr, 0);
              wsAttachInterrupt(GPIOnr);
              reply.concat(String(digitalRead(GPIOnr) * 255));
              wsServerConfigurationWrite();
              break;
            case 13:
              wsServerGPIOmodes[index] = 2;
              pinMode(GPIOnr, 2);
              wsAttachInterrupt(GPIOnr);
              reply.concat(String(digitalRead(GPIOnr) * 255));
              wsServerConfigurationWrite();
              break;

            // OUTPUT modes
            case 12:  // HIGH LOW
              detachInterrupt(digitalPinToInterrupt(GPIOnr));
              wsServerGPIOmodes[index] = 1;
              pinMode(GPIOnr, 1);
              analogWrite(GPIOnr, 0);
              reply.concat(String(digitalRead(GPIOnr) * 255));
              wsServerConfigurationWrite();
              break;
            case 14:  // PWM
              detachInterrupt(digitalPinToInterrupt(GPIOnr));
              wsServerGPIOmodes[index] = 3;
              pinMode(GPIOnr, 1);
              analogWrite(GPIOnr, wsServerGPIOpwms[index]);
              reply.concat(String(wsServerGPIOpwms[index]));
              wsServerConfigurationWrite();
              break;
            case 15:  // BLINK
              detachInterrupt(digitalPinToInterrupt(GPIOnr));
              wsServerGPIOmodes[index] = 4;
              pinMode(GPIOnr, 1);
              analogWrite(GPIOnr, 0);
              reply.concat(String(digitalRead(GPIOnr) * 255));
              wsServerConfigurationWrite();
              break;

            case 17:  // digital output
              if ((wsServerGPIOmodes[index] == 1)
                  || (wsServerGPIOmodes[index] == 4)) {
                GPIOvalue = wsServerGetPart(msg, 3).toInt();
                GPIOvalue = min(1, max(0, GPIOvalue));
                digitalWrite(GPIOnr, GPIOvalue);
                reply.concat(String(GPIOvalue));
              }
              break;
            case 18:  // pwm output
              if (wsServerGPIOmodes[index] == 3) {
                GPIOvalue = wsServerGetPart(msg, 3).toInt();
                GPIOvalue = min(255, max(0, GPIOvalue));
                analogWrite(GPIOnr, GPIOvalue);
                reply.concat(String(GPIOvalue));
                wsServerGPIOpwms[index] = GPIOvalue;
              }
              break;
          }
        }
        wsServerUpdateGroup(10, reply);
        wss_debug1 && Serial.println(F("Reply:   ") + reply);
      }
      break;
    case 2:
      //      wss_debug2 && Serial.println("-----------Handle DataStore");
      if (wsServerIramHeap) {
        if (wsServerTT == 20) {
          // List Keys
          wss_debug2 && Serial.println("List keys");
          reply = F("20||");
          reply.concat(wsDataStoreGetAllKeys());
          wss_debug2 && Serial.println(F("Reply:   ") + reply);
          client->text(reply);  // 1 Single client only !!!!

          // Store client's group based on initial message
          if (wsServerClientGroups.find(client) == wsServerClientGroups.end()) {
            wsServerClientGroups[client] = 20;
          }

        } else {
          switch (wsServerTT) {
            case 23:
              // Wipe Data
              key = msg.substring(2 + SEPERATORSIZE);
              wss_debug2 && Serial.println(F("Delete key: ")+key);
              wsDSDeleteRow(wsDataStore, wsDSNumRows, key.c_str());
              reply = msg;
              reply.concat("||Deleted");
              wss_debug2 && Serial.println(F("Reply:   ") + reply);
              wsServerUpdateGroup(20, reply);
              break;
            case 21:
              // Save Data
              wss_debug2 && Serial.println(F("Save : ") + msg.substring(2 + SEPERATORSIZE));
              if (wsDataStoreUpdateRow(
                    msg.substring(2 + SEPERATORSIZE).c_str())) {
                reply = msg;
                wss_debug2 && Serial.println(F("Reply:   ") + reply);
                wsServerUpdateGroup(20, reply);
              } else {
                reply = F("29||ERROR||DataStore is full");
                wss_debug2 && Serial.println(F("Reply:   ") + reply);
                client->text(reply);          // 1 Single client only !!!!
              }
              break;
            case 22:
              // Find Data
              key = msg.substring(2 + SEPERATORSIZE);
              wss_debug2 && Serial.println(F("Find key: ")+key);
              index = wsDataStoreGetIndexForKey(key.c_str());
              if (index > -1) {
                reply = (F("22||"));
                reply.concat(key);
                reply.concat(F("||"));
                reply.concat(wsDataStoreFindValue(key.c_str()));
              } else {
                reply = F("29||ERROR||Not found: ");
                reply.concat(msg);
              }
              wss_debug2 && Serial.println(F("Reply:   ") + reply);
              client->text(reply);  // 1 Single client only !!!!
          }
          break;
        }
      } else {
        wss_debug2 && Serial.print(F(" ... To use wsDataStore ... Build with MMU :"));
        wss_debug2 && Serial.println(F("16KB cache + 48KB IRAM and 2nd Heap (shared)"));
        reply = F("20||Build with MMU option_3 16KB ... 2nd Heap (shared)");
        client->text(reply);  // 1 Single client only !!!!
      }
      break;
    case 3:
      wss_debug3 && Serial.println(F("-----------Handle email"));
      break;
  }
}

// ------------------------------------------------------ Functions for main app

void wsServerClearAllClients() {
  wsServer.cleanupClients();
}

uint32_t wsServer2ndHeap(String what) {
  if (wsServerIramHeap) {
    //    ESP.setIramHeap();
    // The next does the same and is much faster,
    // and does not crash when refreshing json page by pressing F5
    umm_set_heap_by_id(1);  // 1
    if (what == F("getFreeHeap")) {
      return ESP.getFreeHeap();
    } else if (what == F("getHeapFragmentation")) {
      return ESP.getHeapFragmentation();
    } else if (what == F("getMaxFreeBlockSize")) {
      return ESP.getMaxFreeBlockSize();
    } else {
      return 1;
    }
    //    ESP.setDramHeap();  // Switch back to DRAM heap
    umm_set_heap_by_id(0);  // 0
  } else {
    return 0;
  }
}

String wsServerClientsInfo() {
  String result = "";
  uint8_t count = 0;
  for (auto const& clientPair : wsServerClientGroups) {
    result.concat(String(clientPair.first->id()));  // unique id
    result.concat(F(";"));
    result.concat(String(clientPair.first->status()));
    result.concat(F(";"));
    result.concat(String(clientPair.second));  // group
    result.concat(F(";"));
    result.concat(clientPair.first->remoteIP().toString());
    result.concat(F(";"));
    result.concat(String(clientPair.first->remotePort()));
    result.concat(F("|"));
    count++;
  }
  result.concat(F("Count;"));
  result.concat(String(count));
  result.concat("|");
  return result;
}

String wsServerJsonInfo() {
  String json = "";
  json.concat(F("\" , \"wsServer_ClientsInfo\":\""));
  json.concat(wsServerClientsInfo());
  if (wsServer2ndHeap("Check") == 1) {
    json.concat(F("\" , \"wsServer_2nd_Heap\":\""));
    json.concat("Enabled");
    json.concat(F("\" , \"IFreeHeap\":\""));
    json.concat(String(wsServer2ndHeap("getFreeHeap")));
    json.concat(F("\" , \"IHeapFragmentation\":\""));
    json.concat(String(wsServer2ndHeap("getHeapFragmentation")));
    json.concat(F("\" , \"IMaxFreeBlockSize\":\""));
    json.concat(String(wsServer2ndHeap("getMaxFreeBlockSize")));
  } else {
    json.concat(F("\" , \"wsServer_2nd_Heap\":\""));
    json.concat("Disabled");
  }
  return json;
}

// ----------------------------------------------------------------------------