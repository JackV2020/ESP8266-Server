/* ----------------------------------------------------------------------------

NAPT_RNAT.h contains 3 PROGMEM's :

  The DHCP template file which is created by NAPT_RNATinit when it does not exist.
  The Reverse NAT template file which is created by NAPT_RNATinit when it does not exist.
  The NAPT clients web page.

---------------------------------------------------------------------------- */ 

// ===== DHCP template file is created when it does not exist.

const char NAPT_dhcp_defaults[] PROGMEM = R"rawliteral(#
# DHCP Reservations
#
# You can have up to 10 reservations but....
# The NAPT router supports 4 concurrent connections only.
#
# IP addresses for reservations are 192.168.x.y
#     x : the subnet you configured
#     y : 100 for the first reservation, 101 for the next etc.
#
# Addresses for unregistered devices start after the reservations.
#
# Format : MAC|Name (max 20 characters)|Any remark you want to add.
#          Lines starting with '#' are comment lines
#
# Changes to this file need a reboot to be effective.
#
00:11:22:33:44:50|My Laptop|This get address 192.168.x.100
00:11:22:33:44:51|My Phone|This get address  192.168.x.101
F4:CF:A2:48:41:5B|ESP8266 1|Test ESP with web server...102
00:11:22:33:44:53|Barometer home|ESP8266 to send data to my Raspberry Pi
00:11:22:33:44:54|Barometer garden|ESP8266 to send data to my Raspberry Pi
00:11:22:33:44:55|Barometer greenhouse|ESP8266 to send data to my Raspberry Pi
00:11:22:33:44:56|Barometer greenhouse|ESP8266 to send data to my Raspberry Pi
00:11:22:33:44:57|Barometer greenhouse|ESP8266 to send data to my Raspberry Pi
00:11:22:33:44:58|Barometer greenhouse|ESP8266 to send data to my Raspberry Pi
00:11:22:33:44:59|Barometer greenhouse|ESP8266 to send data to my Raspberry Pi
#
# End of this file
#
)rawliteral";

// ===== Reverse NAT template is created when it does not exist.

const char Reverse_NAT_template[] PROGMEM = R"rawliteral(#
# Reverse NAT
#
# Format :
#  UDP/TCP|External Port|Internal Address Host Part|Internal Port|Comment
#
# The 'Internal Address' needs to be a DHCP reservation I think.
# I did not test with fixed addresses in devices on the NAPT side.
#
# Note that the first 3 bytes of the 'Internal Address' are the network and
# you only specify the last byte in the rule.
#
# DHCP reservations start at x.y.z.100 so an example is :
#
#  TCP|80|100|80
#
# to forward external TCP to port 80 to x.y.z.100 port 80
#
#                       WARNING :
#
# Forwarding port 80 to port 80 on a device disables access this NAPT server !!!
#
#       It would be better to forward port 81 to port 80 like below.
#
TCP|81|102|80|forward external TCP to port 81 to x.y.z.102 port 80
UDP|81|102|80|forward external UDP to port 81 to x.y.z.102 port 80
#
# Changes to this file need a reboot to be effective.
#
# End of this file
#
)rawliteral";

// =====  NAPTClientsPage

const char NAPT_RNAT_NAPTClientsPage[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
<html lang="en">
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" type="image/x-icon" href="/favicon.ico">
<meta charset="UTF-8">
<title>NAPT Clients</title>
<style>
.rnd_btn {background-color:lightgrey;border-radius:50%%;border-width:3;
  border-color:gold;color:blue;width:100px;height:50px;text-align: center}
th, td {
  padding-left: 10px;
  padding-right: 10px;
  border: 1px solid black;
}
</style>
</head>
<body id='body' style='background-color: #E6E6FA;' onload='NAPTinfo()'>
<center>
<p><h1>NAPT Clients</h1>
<a href="/"><button class="rnd_btn">Home</button></a>
<button class="rnd_btn" onclick="NAPTinfo()">Refresh</button>
<button class="rnd_btn" onclick="NAPTInfoToggle()">Info</button>
</p>
<p id='info'></p>
<p id='time'></p>
<p id="detailsheader"></p>
<p id="details"></p>
</center>

<script>
// Short for document.getElementById function
function _(el) {
 return document.getElementById(el);
}

var NAPTInfo = false;

function NAPTInfoToggle() {
 NAPTInfo = !NAPTInfo;
 if (NAPTInfo) {
  _('info').innerHTML = '<hr><br>'
  + '- This screen shows all devices with a<br><br>'
  + 'connection to this NAPT router.<br><br>'
  + '- Up to 4 concurrent clients are supported.<br><br>'
  + '- Up to 10 DHCP reservations are supported.<br><br>'
  + '- DHCP reservations start at 192.168.%NAPTnet%.100<br><br>'
  + '- Unregistered devices start after reservations.<br><br>'
  + '- Edit reservations in \'%NAPTDHCPReservationsPath%\'<br><br>'
  + '....Reservations are activated at startup....<br>'
  + '....So to activate reservations you need a restart....<br>'
  + '<br><hr>'
 } else {
  _('info').innerHTML = '';
 }
}

function NAPTinfo() {
 xmlhttp=new XMLHttpRequest();
 xmlhttp.open("GET", "/NAPTinfo", false);
 xmlhttp.send();

 var mainArray = xmlhttp.responseText.split("|");

 var table="<div id='clientsdiv' style='height: 450px; overflow-y: auto;'>"
 table += "<table id='clientstable' style='border: 1px solid black;";
 table += "border-collapse: collapse; background-color: #F6F6FA'>";
 table += "<tr><th>MAC</th><th>IP</th><th>Device</th></tr>"

 for (var i = 0; i < mainArray.length - 1; i++) {
  var fields = mainArray[i].split(";");
  if (fields[0] == 'NTPTime'){
   _("time").innerHTML =fields[1];
  } else if (fields[0] == 'Count'){
   _("detailsheader").innerHTML = "<h3>"+fields[1]+" Clients<h3>";
  } else {
   table = table + "<tr>"
   table = table + "<td>"+fields[1]+"</td>"
   table = table + "<td>"+fields[0]+"</td>"
   table = table + "<td nowrap align='center'>"+fields[2]+"</td>"
   table += "</tr>"
  }
 }
 if (mainArray.length >3) {
  table +=  "</table></div>"
  _("details").innerHTML = table;
  _("clientsdiv").style.width = (_("clientstable").offsetWidth) +"px";
  scaleMe("clientsdiv")
 } else {
  _("details").innerHTML = "";
 }
}

window.addEventListener('orientationchange',
 function() { scaleMe("clientsdiv");}
);

function scaleMe(div) {
 if (isMobileDevice()) {
  var divwidth = _(div).style.width;
  divwidth = divwidth.substring(0, divwidth.length - 2);
  document.body.style.zoom = Math.round(screen.width / divwidth * 95) / 100 ;
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

// ----------------------------------------------------------------------------