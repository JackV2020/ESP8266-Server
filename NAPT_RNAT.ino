/* ----------------------------------------------------------------------------

NAPT_RNAT.ino : 
  - NAPT server for 4 concurrent clients and up to 10 DHCP reservations.
  - Reverse NAT 

  For your main app : 

  After creating your webserver named MyWebServer with :
    AsyncWebServer MyWebServer(80);       // Create web server
  Call :
    NAPT_RNAT_init(ssid, pass, net, DNS);       // Setup NATP server
    NAPTConfigureWebServer(MyWebServer);  // Provide overview /NAPTClients

  NOTES:

    NAPTinit creates the template "/NAPT_RNAT/DHCPReservations.txt" when it
      does not exist. So in case you mess it up you can delete it and reboot.

  Based on :
    Example : Arduino IDE > File > Examples > ESP8266WiFi > RangeExtender-NAPT
    DHCP Leases : https://github.com/esp8266/Arduino/issues/6031

The lwip is part of the esp8266 core for Arduino environment.
NAPT.h is the file which goes with this NAPT.ino

---------------------------------------------------------------------------- */ 
/*

  I use logicals to enable/disable Serial.print... statements

 This means you can find statements like   
    "NAPT_RNAT_debug1 && Serial.print..." 
    "NAPT_RNAT_debug2 && Serial.print..." 

  These logocals are local to each .ino section

*/

bool NAPT_RNAT_debug1 = true; // debug messages type 1
//bool NAPT_RNAT_debug2 = true; // debug messages type 2

#include <lwip/napt.h>
#include <lwip/dns.h>

/*

from ./.arduino15/packages/esp8266/hardware/esp8266/3.1.2
          /tools/sdk/lwip2/include/lwip/napt.h
 Max size of the tables used for NAPT
#define IP_NAPT_MAX 512
#define IP_PORTMAP_MAX 32

As I understand :
IP_NAPT    : max number of entries 'clients * IP_PORTMAP' in the NAPT table 
IP_PORTMAP : max number of ports per client entries in the NAPT table 

Max clients is 4 and 20 ports per clients so I use values 80 and 20.

Values in examples are like 1000 and 10 but this is very, very heap consuming
  so keep the next values low !!!!

*/ 

#define IP_NAPT_VALUE 100    // IP_NAPT_MAX 512
#define IP_PORTMAP_VALUE 10  // IP_PORTMAP_MAX 32

#include "NAPT_RNAT.h"

// ------------------------------------------------------------------------ NAPT

String NAPTssid;
String NAPTpass;
String NAPTnet;
String NAPTDNS;
String NAPTDNSIP;

#define NAPTDHCPReservationsPath "/NAPT_RNAT/DHCPReservations.txt"
#define NAPT_MAX_DHCP_RESERVATIONS 10 // 10 is the max
String NAPT_RNAT_DHCPReservationMACs[NAPT_MAX_DHCP_RESERVATIONS];  
String NAPT_RNAT_NAPT_DHCPReservationNames[NAPT_MAX_DHCP_RESERVATIONS]; 

// ----------------------------------------------------------------- Reverse NAT

/*

Protocol constants if not defined by the library

https://en.wikipedia.org/wiki/List_of_IP_protocol_numbers

I guess we only need TCP and UDP
*/

#ifndef IP_PROTO_TCP
#define IP_PROTO_TCP 6  // TCP protocol number
#endif

#ifndef IP_PROTO_UDP
#define IP_PROTO_UDP 17 // UDP protocol number
#endif

#define NAPT_RNAT_NAPTReverseNATPath "/NAPT_RNAT/ReverseNAT.txt"

// --------------------------------------------------------- NAPT Initialisation

// ===== Functions used by NAPT_RNAT_init() below

void NAPT_RNAT_readDHCPReservations() {

  int indexMACArray; // Index for the MAC array
  String line;
  int separatorIndex;
  String macPart;
  String namePart;

  File file = LittleFS.open(NAPTDHCPReservationsPath, "r");
  
  indexMACArray = 0; // Index for the MAC array
  
  while (file.available() && indexMACArray < NAPT_MAX_DHCP_RESERVATIONS) {

    line = file.readStringUntil('\n');

    if (line.substring(0,1) != "#") {
      // Split the line into parts based on the '|' separator
      separatorIndex = line.indexOf('|');      

      macPart = line.substring(0, separatorIndex);
      macPart.toUpperCase();
      NAPT_RNAT_DHCPReservationMACs[indexMACArray] = macPart;

      namePart = line.substring(separatorIndex+1);
      // Remove optional comment
      separatorIndex = namePart.indexOf('|');
      if (separatorIndex > 0) {
        namePart = namePart.substring(0, separatorIndex);
      }
      NAPT_RNAT_NAPT_DHCPReservationNames[indexMACArray++] = namePart;

      // Add the MAC address to the MAC table

      // Parse the MAC address
      uint8_t macBytes[6];
      int byteIndex = 0;
      int colonIndex = 0;
      for (int i = 0; i < macPart.length(); i++) {
        if (macPart.charAt(i) == ':') {
          String byteString = macPart.substring(colonIndex, i);
          macBytes[byteIndex++] = strtoul(byteString.c_str(), nullptr, 16);
          colonIndex = i + 1;
        }
      }
      // Parse the last byte
      String byteString = macPart.substring(colonIndex);
      macBytes[byteIndex] = strtoul(byteString.c_str(), nullptr, 16);

      // Add the MAC address to the MAC table
      wifi_softap_add_dhcps_lease(macBytes);
    }
  }
  
  file.close();
}

void NAPT_RNAT_readReverseNATRules() {

  String line;

  int separatorIndex;
  
  String protocol;
  u16_t External_Port;
  IPAddress Internal_IP;
  u16_t Internal_Port;

  String Network;
  Network = F("192.168.")+NAPTnet+F(".");

  File file = LittleFS.open(NAPT_RNAT_NAPTReverseNATPath, "r");
  
  while (file.available()) {

    line = file.readStringUntil('\n');

    if (line.substring(0,1) != "#") {
      // Split the line into parts based on the '|' separator
      separatorIndex = line.indexOf('|');      

      protocol = line.substring(0, separatorIndex);
      protocol.toUpperCase();

      line = line.substring(separatorIndex+1);
      separatorIndex = line.indexOf('|');      
      External_Port = line.substring(0, separatorIndex).toInt();

      line = line.substring(separatorIndex+1);
      separatorIndex = line.indexOf('|');      
      Internal_IP.fromString(Network+line.substring(0, separatorIndex));

      line = line.substring(separatorIndex+1);
      separatorIndex = line.indexOf('|');      
      Internal_Port = line.substring(0, separatorIndex).toInt();

      NAPT_RNAT_debug1 && Serial.println(F(" ... Reverse NAT: ")+protocol+F(" port ")+String(External_Port) 
                  +F(" to ")+Internal_IP.toString()+F(" port ")+String(Internal_Port));

      if (protocol == F("TCP")) { 
        ip_portmap_add(IP_PROTO_TCP, WiFi.localIP(), External_Port,
          Internal_IP, Internal_Port);
      } else if (protocol == F("UDP")) {
        ip_portmap_add(IP_PROTO_UDP, WiFi.localIP(), External_Port,
          Internal_IP, Internal_Port);
      }
    }
  }  
  file.close();
}

// ===== Initialize NAPT

void NAPT_RNAT_init(String& ssid, String& pass, String& net, String& DNS) {

  NAPTssid  = ssid;
  NAPTpass = pass;
  NAPTnet = net;
  NAPTDNS = DNS;

  NAPT_RNAT_debug1 && Serial.println(F("Configuring NAPT and RNAT..."));

  if (!LittleFS.exists(NAPT_RNAT_NAPTReverseNATPath)) {
    File file = LittleFS.open(NAPT_RNAT_NAPTReverseNATPath, "w");
    file.print(FPSTR(Reverse_NAT_template)); 
    file.close();
  }

  if (!LittleFS.exists(NAPTDHCPReservationsPath)) {
    File file = LittleFS.open(NAPTDHCPReservationsPath, "w");
    file.print(FPSTR(NAPT_dhcp_defaults)); 
    file.close();
  }

  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
// Define local ip, gateway and subnet
  WiFi.softAPConfig( IPAddress(192, 168, NAPTnet.toInt(), 1), 
    IPAddress(192, 168, NAPTnet.toInt(), 1), IPAddress(255, 255, 255, 0));

// Create NAPT Access Point (add hostname and WiFi IP to NAPTssid)
  WiFi.softAP(NAPTssid + F(" (")+CfgMgrWiFihostname+F(" ")+ 
    WiFi.localIP().toString()+F(")"), NAPTpass);

  NAPT_RNAT_readDHCPReservations();
/*
 By default, DNS option will point to the interface IP
 Instead, point it to a real DNS server.
 Notice that:
 - DhcpServer class only supports IPv4
 - Only a single IP can be set
*/
  auto& NAPTserver = WiFi.softAPDhcpServer();

  if (NAPTDNS == F("Standard WiFi")) 
    { NAPTserver.setDns(WiFi.dnsIP(0)); }
  if (NAPTDNS == F("Google"))        
    { NAPTserver.setDns(IPAddress(8,8,8,8)); }
  if (NAPTDNS == F("Control D"))     
    { NAPTserver.setDns(IPAddress(76,76,2,0)); }
  if (NAPTDNS == F("Quad9"))         
    { NAPTserver.setDns(IPAddress(9,9,9,9)); }
  if (NAPTDNS == F("OpenDNS Home"))  
    { NAPTserver.setDns(IPAddress(208,67,222,222)); }
  if (NAPTDNS == F("Cloudflare"))    
    { NAPTserver.setDns(IPAddress(1,1,1,1)); }
  if (NAPTDNS == F("AdGuard DNS"))   
    { NAPTserver.setDns(IPAddress(94,140,14,14)); }
  if (NAPTDNS == F("CleanBrowsing")) 
    { NAPTserver.setDns(IPAddress(185,228,168,9)); }
  if (NAPTDNS == F("AlterNAPTe DNS")) 
    { NAPTserver.setDns(IPAddress(76,76,19,19)); }

  ip4_addr_t ip = NAPTserver.getDns();
  NAPTDNSIP = String(ip4_addr1(&ip)) + F(".") + String(ip4_addr2(&ip)) + 
    F(".")  + String(ip4_addr3(&ip)) + F(".") + String(ip4_addr4(&ip));

  NAPT_RNAT_debug1 && Serial.printf_P(PSTR(" ... first heap before napt init: %d\n"),
   ESP.getFreeHeap());
  err_t ret = ip_napt_init(IP_NAPT_VALUE, IP_PORTMAP_VALUE);
  NAPT_RNAT_debug1 && Serial.printf_P(PSTR(" ... ip_napt_init(%d,%d): ret=%d (OK=%d)\n"),
     IP_NAPT_VALUE, IP_PORTMAP_VALUE, (int)ret, (int)ERR_OK);

  if (ret == ERR_OK) {
    ret = ip_napt_enable_no(SOFTAP_IF, 1);
    NAPT_RNAT_debug1 && Serial.printf_P(PSTR(" ... ip_napt_enable_no(1): ret=%d (OK=%d)\n"),
      (int)ret, (int)ERR_OK);
    if (ret == ERR_OK) { NAPT_RNAT_debug1 && Serial.println(F(" ... WiFi Network '") + 
      WiFi.softAPSSID() + F("' with password '") + NAPTpass +
      F("' is now NAPTed behind '") + CfgMgrWiFissid + F("'")); }
  }
  NAPT_RNAT_debug1 && Serial.printf_P(PSTR(" ... first heap after napt init:  %d\n"),
    ESP.getFreeHeap());
  if (ret != ERR_OK) { NAPT_RNAT_debug1 && Serial.println(F(" ... NAPT initialization failed")); }

  NAPT_RNAT_readReverseNATRules();
}

// --------------------------------------------------------------- NAPT Web Page

// =====  Configure web server

void NAPT_RNAT_ConfigureWebServer(AsyncWebServer &server) {

  NAPT_RNAT_debug1 && Serial.println(F("Configuring NAPT Web Page..."));

  server.on("/NAPTClients", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(200,F("text/html"), NAPT_RNAT_NAPTClientsPage, NAPT_RNAT_NAPTprocessor);
  });

  // Next /NAPTinfo is used by /NAPTClients above to refresh details
  server.on("/NAPTinfo", HTTP_GET, NAPT_RNAT_NAPTinfo);

}

// =====  Processor for NAPTClientsPage

String NAPT_RNAT_NAPTprocessor(const String& var) {

  if (var == F("NAPTDHCPReservationsPath")) {return NAPTDHCPReservationsPath;}
  if (var == F("NAPTnet")) { return NAPTnet;  }

  return String();
}

// =====  Return NAPT clients info

void NAPT_RNAT_NAPTinfo(AsyncWebServerRequest * request) {

  /*
  return format : 
    NTPTime;YYYY-MM-DD HH:mm:ss|client info|Count;#|
  return example: 
    NTPTime;2024-05-30 11:08:19|192.168.0.101;AC:72:89:C3:2D:05;Laptop|Count;1|
  */
  String result = F("NTPTime;");
  result.concat(ntpTime());
  result.concat(F("|"));
  result.concat(NAPT_RNAT_NAPTClientsInfo());

  request->send(200, "text/html", result);}

// Next is used by NAPT_RNAT_NAPTinfo above and by json page

String NAPT_RNAT_NAPTClientsInfo(){
  String result = "";

  unsigned char number_client;

  struct station_info *stat_info;
  struct station_info *next_info;
  char wifiClientMac[18];
  bool reservation;

  int i = 0;
  stat_info = wifi_softap_get_station_info();
/*

stat_info is a pointer to the first element

+-----------------+       +-----------------+       +-----------------+
| station_info    |       | station_info    |       | station_info    |
| IP: 192.168.4.2 | ----> | IP: 192.168.4.3 | ----> | IP: 192.168.4.4 |
| MAC: xx:xx:xx   |       | MAC: xx:xx:xx   |       | MAC: xx:xx:xx   |
| next: --------> |       | next: --------> |       | next: NULL      |
+-----------------+       +-----------------+       +-----------------+

*/

  while (stat_info != NULL) {
    result.concat(IPAddress(stat_info->ip).toString());
    result.concat(F(";"));
    sprintf(wifiClientMac,
      "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(stat_info->bssid));
    result.concat(wifiClientMac);
    result.concat(F(";"));
    reservation = false;
    for (int j = 0; j < NAPT_MAX_DHCP_RESERVATIONS; j++) {
      if (NAPT_RNAT_DHCPReservationMACs[j] == wifiClientMac) {
        result.concat(NAPT_RNAT_NAPT_DHCPReservationNames[j]);
        result.concat(F("|"));
        reservation = true;
        break;
      }
    }
    if (!reservation) {
      result.concat(F("?|")); //  result = "Count;";
    }
    next_info = STAILQ_NEXT(stat_info, next);
    free(stat_info);  // Free the current station_info structure
    stat_info = next_info;
    i++;
  }

  number_client = wifi_softap_get_station_num();
  result.concat(F("Count;"));
  result.concat(String(number_client));
  result.concat(F("|"));
  return result;
}

String NAPT_RNAT_JsonInfo() {
  String json = "";

  json.concat(F("\" , \"napt_SSID\":\""));
  json.concat(WiFi.softAPSSID());
  json.concat(F("\" , \"napt_IP\":\""));
  json.concat(WiFi.softAPIP().toString());
  json.concat(F("\" , \"napt_MAC_address\":\""));
  json.concat(WiFi.softAPmacAddress());
  json.concat(F("\" , \"napt_DNS\":\""));
  json.concat(NAPTDNS);
  json.concat(F("\" , \"napt_DNSIP\":\""));
  json.concat(NAPTDNSIP);
  json.concat(F("\" , \"napt_ClientsInfo\":\""));
  json.concat(NAPT_RNAT_NAPTClientsInfo());

  return json;
}

// ----------------------------------------------------------------------------
