/* ----------------------------------------------------------------------------

ntp.ino : time information without using RTC hardware.

  For your main app :

  After connecting to WiFi :
    ntpInit();          // Start ntp synchronisation
  and after this you can use the next calls in any ino file :
    ntpTime()           // current time "YYYY-MM-DD HH:mm:ss"
    ntpBootTime()       // boot time "YYYY-MM-DD HH:mm:ss"
    ntpUpTime()         // uptime in seconds (long int)

  NOTES:

    ntpInit creates the template configuration "/Configntp/ntp.txt" when it
      does not exist. So in case you mess it up you can delete it and reboot.

    ntpInit() configures the NTP settings,starts ntp sync and starts a Ticker
      to run ntpBootTimeTriggerRoutine.
    ntpBootTimeTriggerRoutine() is a routine which is run by a Ticker which
      stops as soon as it has set the local variable NTPBootTime.

    ntp itself runs with a default interval of 60000ms which can be changed
      with ntp.updateInterval(<milliseconds>)
    In the code below I put it on 3600000 which is 1 hour.

In Arduino IDE install NTP by Stefan Staub by searching in "Manage Libraries"
This gives you NTP.h (Uppercase NTP, that's why I use lowercase for ntp.ino)

WiFiUdp.h, time.h and Ticker.h are part of the esp8266 core for Arduino IDE.

ntp.h is the file which goes with this ntp.ino

---------------------------------------------------------------------------- */ 

#include <WiFiUdp.h>
#include <time.h>
#include "NTP.h"
#include <Ticker.h>

#include "ntp.h"

#define ntpPath "/Configntp/ntp.txt"

String NTPBootTime="-";
time_t NTPBootTimeEpoch; // remember this at boottime to calculate uptime

WiFiUDP wifiUdp;
NTP ntp(wifiUdp);

Ticker ntpBootTimeTrigger;
int ntpBootTimeTriggerInterval = 5; // seconds

// ====== ntp trigger stops when NTPBootTime is found.

void ntpBootTimeTriggerRoutine(){
    _SERIAL_PRINTLN(F("ntpBootTimeTriggerRoutine triggered"));
    ntp.update();
    if (ntpTime().substring(0,4) != "1970" ) {
      NTPBootTimeEpoch = ntp.epoch();
      NTPBootTime = ntpTime();
      _SERIAL_PRINTLN(F("Boot time epoch:")+String(NTPBootTimeEpoch));
      _SERIAL_PRINTLN(F("Boot time:")+NTPBootTime);
//      ntp.updateInterval(60000); // back to standard 60 seconds
      ntp.updateInterval(3600000); // to 1 hour
      ntpBootTimeTrigger.detach();
// Create an entry in the websocket datastore
      wsDataStoreUpdateRow((F("BootTime||")+NTPBootTime).c_str());
    };
  }

// ===== ntp init

void ntpInit() {

  _SERIAL_PRINTLN(F("Configuring NTP ..."));

  if (!LittleFS.exists(ntpPath)) {
    File file = LittleFS.open(ntpPath, "w");
    file.print(FPSTR(ntp_defaults));
    file.close();
  }
// Read ntp configuration
  String Summer;
    int8_t sweek; int8_t swday; int8_t smonth; int8_t shour; int stzOffset;
  String Winter;
    int8_t wweek; int8_t wwday; int8_t wmonth; int8_t whour; int wtzOffset;
  File file = LittleFS.open(ntpPath, "r");
  String line; int counter=0;
  while (file.available()) {
    line = file.readStringUntil('\n');
    if (line.substring(0,1) != "#") {
      line = line.substring(0,line.indexOf("#"));
      line.replace(" ","");
      switch (counter) {
      case  0: Summer=line; break;
      case  1: sweek=line.toInt();break;
      case  2: swday=line.toInt();break;
      case  3: smonth=line.toInt();break;
      case  4: shour=line.toInt();break;
      case  5: stzOffset=line.toInt();break;
      case  6: Winter=line; break;
      case  7: wweek=line.toInt();break;
      case  8: wwday=line.toInt();break;
      case  9: wmonth=line.toInt();break;
      case 10: whour=line.toInt();break;
      case 11: wtzOffset=line.toInt();break;
      }
      counter++;
    }
  }
  file.close();

  ntp.ruleDST(Summer.c_str(), sweek, swday, smonth, shour, stzOffset);

  ntp.ruleSTD(Winter.c_str(), wweek, wwday, wmonth, whour, wtzOffset);
  ntp.begin();
  ntp.updateInterval(5000); // start updating every 5 seconds
  ntpBootTimeTrigger.attach(ntpBootTimeTriggerInterval,ntpBootTimeTriggerRoutine);
}

// ======  ntp.... functions

String ntpTime() {
  return ntp.formattedTime("%Y-%m-%d %T"); // YYYY-MM-DD HH:mm:ss
}

String ntpBootTime() {
  return NTPBootTime; // YYYY-MM-DD HH:mm:ss
}

long int ntpUpTime() {
  return (ntp.epoch() - NTPBootTimeEpoch); // seconds
}

String ntpJsonInfo() {
  String json = "";
  json.concat(F("\" , \"ntp_Time\":\""));
  json.concat(ntpTime());
  json.concat(F("\" , \"ntp_BootTime\":\""));
  json.concat(ntpBootTime());
  json.concat(F("\" , \"ntp_UpTime\":\""));
  json.concat(String(ntpUpTime()));
  return json;
}
// ----------------------------------------------------------------------------