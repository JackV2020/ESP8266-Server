/* ----------------------------------------------------------------------------

CfgMgr.h holds configuration variables and 3 PROGMEM's for the Management page

  CfgMgr_html           the Configuration Manager web page
  CfgMgr_Saved          shows after successfully saving data
  CfgMgr_Wrong_Password shows after trying to save with wrong password

---------------------------------------------------------------------------- */

// ====== Configuration Manager Password

String CfgMgrpass = "secret"; // Initial password to submit changes
#define CfgMgrpassPath "/CfgMgr/CfgMgrpass.enc"

// ====== WiFi Settings

String CfgMgrWiFissid = "CfgMgrWiFiSSID";
String CfgMgrWiFipass = "CfgMgrWiFipassword";
String CfgMgrWiFihostname ="Setup";
#define CfgMgrWiFissidPath "/CfgMgr/CfgMgrWiFissid.enc"
#define CfgMgrWiFipassPath "/CfgMgr/CfgMgrWiFipass.enc"
#define CfgMgrWiFihostnamePath "/CfgMgr/CfgMgrWiFihostname.enc"

// ====== LittleFSWeb Settings

String CfgMgrLittleFSWebuser = "admin";
String CfgMgrLittleFSWebpassword = "admin";
#define CfgMgrLittleFSWebuserPath "/LittleFSWeb/CfgMgrLittleFSWebuser.enc"
#define CfgMgrLittleFSWebpasswordPath "/LittleFSWeb/CfgMgrLittleFSWebpass.enc"

// ====== NAPT Settings  uses other folder to store settings

String CfgMgrNAPTssid="NAPTSSID";
String CfgMgrNAPTpass="NAPTPASS";
String CfgMgrNAPTnet="0"; // Use 0..9 for 192.168.CfgMgrNAPTnet.0/24
String CfgMgrNAPTDNS="Standard WiFi";
#define CfgMgrNAPTnetPath "/NAPT_RNAT/CfgMgrNAPTnet.enc"
#define CfgMgrNAPTssidPath "/NAPT_RNAT/CfgMgrNAPTssid.enc"
#define CfgMgrNAPTpassPath "/NAPT_RNAT/CfgMgrNAPTpass.enc"
#define CfgMgrNAPTDNSPath "/NAPT_RNAT/CfgMgrNAPTDNS.enc"

// ----------------------------------------------------------------------------

// ====== Configuration Manager web page

const char CfgMgr_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
<html lang="en">
<head>
 <title>Configuration Manager</title>
 <meta name='viewport' content='width=device-width, initial-scale=1'>
 <link rel="icon" type="image/x-icon" href="/favicon.ico">
 <title>Configuration Manager</title>
 <style>
  .rnd_btn {background-color:lightgrey;border-radius:50%%;border-width:3;
    border-color:gold;color:blue;width:100px;height:50px;text-align:center}
 </style>
</head>
<body id='body' style='background-color: #E6E6FA;' onload='scaleMe()'>
<center><br>
 <h1>Configuration</h1>
 %LANIP%<br><br>
 <a href='/'><button class="rnd_btn">Home</button></a>
 <a href='/start_ota'><button class="rnd_btn">OTA</button></a>
 <button class="rnd_btn" onclick="cfgmgrInfoToggle()"> Info </button>
 <br>
 <p id='info'></p>
 <form action='/ConfigurationManager' method='POST'>
 <table>
 <tr><td align='right'>Hostname WiFi </td><td>
  <input type='text' name='CfgMgrWiFihostname' value='%CfgMgrWiFihostname%'>
  </td></tr>
 <tr><td align='right'>SSID WiFi </td><td>
  <input type='text' name='CfgMgrWiFissid' value='%CfgMgrWiFissid%'>
  </td></tr>
 <tr><td align='right'>Pwd WiFi </td><td>
  <input type='password' name='CfgMgrWiFipass'>
  </td></tr>
 <tr style="height:10px;"><td></td></tr>

 <tr><td align='right'>User LittleFSWeb </td><td>
<input type='text' name='CfgMgrLittleFSWebuser' value='%CfgMgrLittleFSWebuser%'>
  </td></tr>
 <tr><td align='right'>Pwd LittleFSWeb </td><td>
  <input type='password' name='CfgMgrLittleFSWebpassword'></td></tr>
 <tr style="height:10px;"><td></td></tr>

 <tr><td align='right'>SSID NAPT </td><td>
  <input type='text' name='CfgMgrNAPTssid' value='%CfgMgrNAPTssid%'>
  </td></tr>
 <tr><td align='right'>Pwd NAPT </td><td>
  <input type='password' name='CfgMgrNAPTpass'>
  </td></tr>
 <tr><td align='right'>Network NAPT </td>
  <td>192.168.<select id="CfgMgrNAPTnet" name="CfgMgrNAPTnet"></select>.0
  </td></tr>
 <tr><td align='right'>DNS NAPT </td><td>
  <select style="text-align: center;" id="CfgMgrNAPTDNS" name="CfgMgrNAPTDNS">
  </select></td></tr>
 <tr style="height:10px;"><td></td></tr>

 <tr><td align='right'>New Pwd CfgMgr </td>
  <td><input type='password' name='newpassCFGMGR'></td></tr>
 </table>
 <br><input type ='submit' class="rnd_btn" value ='Save'>
 <br><br><input style="border-color: gold" type='password' id ='passCFGMGR'
  name='passCFGMGR'  size="25" placeholder='Configuration Manager Password'>
 <br><br>
 </form>
</center>
<script>

// Short for document.getElementById function
function _(el) {
 return document.getElementById(el);
}

// Info

cfgmgrInfo = false;
littlefswebInfo = false;
function cfgmgrInfoToggle() {
 cfgmgrInfo = !cfgmgrInfo;
 if (cfgmgrInfo) {
  _('body').style.heigth= '200vh'
  _('info').innerHTML = '<hr>'
  + '<font color=blue><h3>Remember</h3>'
  + '<h5> Did you forget Configuration Manager Password,<br>'
  + 'LittleFSWeb user and or password,<br>'
  + 'broke /index.html or /LittleFSWeb/LittleFSWeb.html?</h5>'
  + '- Connect D8 (GPIO15) to 3.3v while powered on<br>'
  + '(this will reset the above only and restart)</font><br>'
  + '<h5>Remark on \'Network NAPT\'</h5>'
  + '- Select a setting so \'Network NAPT\' differs from your WiFi<br>'
  + '<h3>Developer, to add your own settings :</h3>'
  + '<h5>1 CfgMgr.h is where you:</h5>'
  + '- define your settings<br>'
  + '- and where to save them<br>'
  + '- find a form with the fields you see below'
  + '<h5>2 CfgMgr.ino is where you:</h5>'
  + '- update CfgMgrActions to process settings<br>'
  + '- update CfgMgrProcessor to insert values<br>'
  + '- update CfgMgrReadConfig<br>'
  + '- update CfgMgrSaveConfig<br>'
  + '<br><hr>'
 } else {
  _('body').style.heigth= '100vh'
  _('info').innerHTML = '';
 }
}

// Setup page elements

// Get the subnet select element

var selectElement = _('CfgMgrNAPTnet');
// Values to be added to the select element
var values = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
// Loop through the values and create options
values.forEach(function(value) {
 var option = document.createElement('option');
 option.value = value;
 option.text = value;

// Check if the current value matches CfgMgrNAPTnet variable
 if (value === %CfgMgrNAPTnet%) {
  option.selected = true; // Preselect the option if it matches CfgMgrNAPTnet
 }

// Append the option to the select element
 selectElement.appendChild(option);
});

// Get the DNS select element

selectElement = _('CfgMgrNAPTDNS');
// Values to be added to the select element
values = ["Standard WiFi",
 "Google",
 "Control D",
 "Quad9",
 "OpenDNS Home",
 "Cloudflare",
 "AdGuard DNS",
 "CleanBrowsing",
 "AlterNAPTe DNS"];
values.forEach(function(value) {
 var option = document.createElement('option');
 option.value = value;
 option.text = value;
 if (value === "%CfgMgrNAPTDNS%") {
  option.selected = true; // Preselect the option if it matches CfgMgrNAPTDNS
 }
// Append the option to the select element
 selectElement.appendChild(option);
});

window.addEventListener('orientationchange', function() {
 scaleMe()
});

function scaleMe() {
 if (isMobileDevice()) {
  document.body.style.zoom = Math.round(screen.width / 4) / 100 ;
 }
}

function isMobileDevice() {
 const devices = [
  "Android",
  "webOS",
  "iPhone",
  "iPad",
  "iPod",
  "BlackBerry",
  "IEMobile",
  "Opera Mini"
 ];
// The 'i' flag is used to make the regex case-insensitive.
 const teststr = new RegExp(devices.join('|'), 'i');
 return teststr.test(navigator.userAgent);
}
</script>
</body>
</html>
)rawliteral";

// ====== Configuration Manager web page after save data

const char CfgMgr_Saved[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
<html lang="en">
<script>
function startCountdown() {
 var timeLeft = 20;
 var countdownElement = document.getElementById("countdown");
 countdownElement.innerHTML = timeLeft;

 var countdownTimer = setInterval(function() {
  timeLeft--;
  countdownElement.innerHTML = timeLeft;
  if (timeLeft <= 0) {
   clearInterval(countdownTimer);
   window.location.href = '/';
  }

 }, 1000);
}
window.onload = startCountdown;
</script>
<body style='background-color: #E6E6FA;'>
<center>
<h1>
Saved Data<br><br>
Wait <span id="countdown">20</span> seconds for reboot to finish.<br><br>
Connect to the right WiFi if needed...</h1>
</center>
</body>
</html>
)rawliteral";

// ====== Configuration Manager web page after wrong password

const char CfgMgr_Wrong_Password[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
<html lang="en">
<script>
window.setTimeout(function(){window.location.href = '/ConfigurationManager';}, 5000);
</script>
<body style='background-color: #E6E6FA;'>
<center><h1>Wrong 'Configuration Manager Password' ....</h1></center>
</body>
</html>
)rawliteral";

// ----------------------------------------------------------------------------