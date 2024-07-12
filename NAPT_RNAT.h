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
# Changes to this file need a reboot to be effective.
#
# Use Configuration Manager 'Save & Boot' button.
#
# You can have up to 10 reservations but....
# The NAPT router supports 4 concurrent connections only.
#
# IP addresses for reservations are 192.168.x.y
#     x : the subnet you configured
#     y : 100 for the first reservation, 101 for the next etc.
#
# Addresses for unregistered devices start after the last reservation.
#
# Format : MAC|Name (max 20 characters)|Any remark you want to add.
#          Lines starting with '#' are comment lines
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
# Changes to this file need a reboot to be effective.
#
# Use Configuration Manager 'Save & Boot' button.
#
# Format : field1|field2|field3|field4|field5
#   field 1 : UDP/TCP
#   field 2 : External Port
#   field 3 : Internal Address Host Part
#   field 4 : Internal Port
#   field 5 : Optional comment
#
# Note that the first 3 bytes of the 'Internal Address' are
#  the network and you only specify the last byte in the rule.
#
#                       WARNING :
#
# Forwarding port 80 to port 80 on another device disables access
#   to this NAPT server from the WiFi !!!
#
#       It would be better to forward port 81 to port 80 like below.
#
TCP|81|102|80|forward external TCP to port 81 to x.y.z.102 port 80
UDP|81|102|80|forward external UDP to port 81 to x.y.z.102 port 80
#
# End of this file
#
)rawliteral";

// =====  NAPTClientsPage

const char NAPT_RNAT_NAPTClientsPage[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
<html lang="en">
<head>
 <title>NAPT Clients</title>
 <meta name='viewport' content='width=device-width, height=device-height, initial-scale=1, user-scalable=no'>
 <link rel="icon" type="image/x-icon" href="/favicon.ico">
 <style>
  :root {
   --scale-factor: 1;
  }
  .rnd_btn {
   background-color: lightgrey;
   border-radius: 50%;
   border-width: 3px;
   border-color: gold;
   color: blue;
   width: 100px;
   height: 50px;
   text-align: center;
  }
  body {
   background-color: #E6E6FA;
   margin: 0;
   padding: 0;
   height: 100vh;
   display: flex;
   justify-content: center;
//   align-items: center;
   overflow: hidden;
  }
  .container {
   text-align: center;
   transform-origin: top;
   transform: scale(var(--scale-factor));
  }
  .scrollable {
   height: auto;
   display: block;
   overflow: auto;
  }
  th, td {
    padding-left: 10px;
    padding-right: 10px;
    border: 1px solid black;
  }
  .centered-content {
   display: flex;
   justify-content: center;
   align-items: center;
   height: 100;
  }
  #napttable {
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: 100px; /* Adjust as necessary */
  }
  table {
    margin: auto; /* Centers the table */
  }
 </style>
</head>
<body onload='updateScaleFactor();NAPTinfo()'>
 <div class="container">
  <h1>NAPT Clients</h1>
  
  <a href="/"><button class="rnd_btn">Home</button></a>
  <button class="rnd_btn" onclick="NAPTinfo()">Refresh</button>
  <button class="rnd_btn" onclick="NAPTInfoToggle()">Info</button>

  <span id='info' style="display: none;">
  <br><hr><br>
- This screen shows all devices with a<br>
connection to this NAPT router.<br><br>
- Up to 4 concurrent clients are supported.<br>
- Up to 10 DHCP reservations are supported.<br>
- Edit reservations in '/NAPT_RNAT/DHCPReservations.txt'<br>
- DHCP reservations start at 192.168.0.100<br>
- Unregistered devices start after reservations.<br><br>
....Reservations are activated at startup....<br>
....So to activate reservations you need a restart....<br><br>
  <hr>
  </span>
  <p id='time'></p>
  <p id="naptcountheader"></p>
  <div id="napttable" class="centered-content"></div>
 </div>
 <script>
function _(el) {
 return document.getElementById(el);
}

var NAPTInfo = false;

function NAPTInfoToggle() {
 NAPTInfo = !NAPTInfo;
 if (NAPTInfo) {
  _('info').style.display = 'block'
 } else {
  _('info').style.display = 'none'
 }
}

function NAPTinfo() {
 xmlhttp = new XMLHttpRequest();
 xmlhttp.open("GET", "/NAPTinfo", false);
 xmlhttp.send();

 var mainArray = xmlhttp.responseText.split("|");

 var table = ""
 table += "<table id='clientstable' style='border: 1px solid black;";
 table += "border-collapse: collapse; background-color: #F6F6FA'>";
 table += "<tr><th>MAC</th><th>IP</th><th>Device</th></tr>"

 for (var i = 0; i < mainArray.length - 1; i++) {
  var fields = mainArray[i].split(";");
  if (fields[0] == 'NTPTime'){
   _("time").innerHTML = fields[1];
  } else if (fields[0] == 'Count'){
   _("naptcountheader").innerHTML = "<h3>" + fields[1] + " Clients<h3>";
  } else {
   table += "<tr>"
   table += "<td>" + fields[1] + "</td>"
   table += "<td>" + fields[0] + "</td>"
   table += "<td nowrap align='center'>" + fields[2] + "</td>"
   table += "</tr>"
  }
 }
 if (mainArray.length > 3) {
  table +=  "</table>"
  _("napttable").innerHTML = table;
 } else {
  _("napttable").innerHTML = "";
 }
}

function updateScaleFactor() {
 const vh = window.innerHeight; // Get viewport height
 const vw = window.innerWidth; // Get viewport width
 if (vh > vw) { // If in portrait mode (height > width)
  const scaleFactor = (vh + 200) / 1000; // Calculate scale factor based on height
  document.documentElement.style.setProperty('--scale-factor', scaleFactor); // Set CSS variable for scale factor
  document.body.style.overflow = 'hidden'; // Disable scrolling in portrait mode
  document.body.style.height = '100vh'; // Set body height to 100% of viewport height
  document.body.style.display = 'flex'; // Use flexbox layout
  document.body.style.justifyContent = 'center'; // Center content horizontally
//  document.body.style.alignItems = 'center'; // Center content vertically
  document.querySelector('.container').classList.remove('scrollable'); // Remove scrolling class from container
  window.scrollTo(0, 0); // Scroll to the top
 } else { // If in landscape mode (width >= height)
  document.documentElement.style.setProperty('--scale-factor', 1); // Reset scale factor to 1 (no scaling)
  document.body.style.overflow = 'auto'; // Enable scrolling in landscape mode
  document.body.style.height = 'auto'; // Set body height to auto
  document.body.style.display = 'block'; // Use block layout
  document.body.style.justifyContent = 'unset'; // Reset horizontal alignment
//  document.body.style.alignItems = 'unset'; // Reset vertical alignment
  document.querySelector('.container').classList.add('scrollable'); // Add scrolling class to container
 }
}
window.addEventListener('resize', updateScaleFactor); // Update scale factor on window resize
window.addEventListener('load', updateScaleFactor); // Update scale factor when page loads
</script>
</body>
</html>
)rawliteral";

// ----------------------------------------------------------------------------