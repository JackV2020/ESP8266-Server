/* ----------------------------------------------------------------------------

CfgMgr.h holds configuration variables and 3 PROGMEM's for the Management page

  CfgMgr_html           the Configuration Manager web page
  CfgMgr_Saved          shows after successfully saving data
  CfgMgr_Wrong_Password shows after trying to save with wrong password

---------------------------------------------------------------------------- */

// ====== Configuration Manager Password

String CfgMgruser = "admin";
String CfgMgrpassword = "admin";
#define CfgMgruserPath "/CfgMgr/CfgMgruser.enc"
#define CfgMgrpasswordPath "/CfgMgr/CfgMgrpass.enc"

// ====== WiFi Settings

String CfgMgrWiFissid = "CfgMgrWiFiSSID";
String CfgMgrWiFipass = "CfgMgrWiFipassword";
String CfgMgrWiFihostname ="Setup";
#define CfgMgrWiFissidPath "/CfgMgr/CfgMgrWiFissid.enc"
#define CfgMgrWiFipassPath "/CfgMgr/CfgMgrWiFipass.enc"
#define CfgMgrWiFihostnamePath "/CfgMgr/CfgMgrWiFihostname.enc"

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
 <meta name='viewport' content='width=device-width, height=device-height, initial-scale=1, user-scalable=no'>
 <link rel="icon" type="image/x-icon" href="/favicon.ico">
 <style>
  :root {
   --scale-factor: 1;
  }
  .rnd_btn {
   background-color: lightgrey;
   border-radius: 50%%;
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
   align-items: flex-start; /* Align items to the top */
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
  .centered-content {
   display: flex;
   flex-direction: column;
   justify-content: center;
   align-items: center;
  }
  .form-container {
   display: flex;
   flex-direction: column;
   align-items: center;
  }
</style>
</head>
<body>
 <div class="container">
  <h1>Configuration Manager</h1>
 <a href='/'><button class="rnd_btn">Home</button></a>
 <a href='/FOTA/FOTA'><button class="rnd_btn">FOTA</button></a>
 <button class="rnd_btn" onclick="CfgMgrInfoToggle()">Info</button><br>
 <!-- activating the Logout button gives a clean logout -->
 <!-- button class="rnd_btn" onclick="logout()">Logout</button -->
  <span id='info' style="display: none;">
  <br><hr>
<font color=blue><h3>Remember</h3>
<h5>Did you forget Admin User and/or Password,<br>
broke /index.html or /LittleFSWeb/LittleFSWeb.html?</h5>
- Connect D8 (GPIO15) to 3.3v while powered on<br>
(this will reset the above only and restart)</font><br>
<h5>Remark on 'Network NAPT'</h5>
- Select a setting so 'Network NAPT' differs from your WiFi<br>
  <br><hr>
  </span>
  <br>
 <form id="cfgmgrform" class="centered-content" action='/ConfigurationManager' method='POST'>
  <div class="form-container">
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

    <tr><td align='right'>Admin User</td><td>
     <input type='text' name='CfgMgruser' value='%CfgMgruser%'>
     </td></tr>
    <tr><td align='right'>Admin Password</td><td>
     <input type='password' name='CfgMgrpassword'></td></tr>
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
   </table>
   <input type='submit' class="rnd_btn" value ="Save & Boot">
  </div>
 </form>
 </div>
 <script>
function _(el) {
 return document.getElementById(el);
}

var CfgMgrInfo = false;

function CfgMgrInfoToggle() {
 CfgMgrInfo = !CfgMgrInfo;
 if (CfgMgrInfo) {
  _('info').style.display = 'block'
 } else {
  _('info').style.display = 'none'
 }
}
// ----- Logout function
function logout() {
 // Supply invalid credentials to logout and redirect to root
 var xhr = new XMLHttpRequest();
 xhr.open("GET", "/CfgMgr/logout", true, "invaliduser", "invalidpassword");
 xhr.onreadystatechange = function () {
  if (xhr.readyState == 4) {
   window.location.href = "/";
  }
 }
 xhr.send();
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
 if (value === 0) {
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
 if (value === "Quad9") {
  option.selected = true; // Preselect the option if it matches CfgMgrNAPTDNS
 }
 // Append the option to the select element
 selectElement.appendChild(option);
});

function updateScaleFactor() {
 const vh = window.innerHeight; // Get viewport height
 const vw = window.innerWidth; // Get viewport width
 if (vh > vw) { // If in portrait mode (height > width)
  const scaleFactor = (vh + 200) / 1000; // Calculate scale factor based on height
  document.documentElement.style.setProperty('--scale-factor', scaleFactor); // Set CSS variable for scale factor
  document.body.style.overflow = 'hidden'; // Disable scrolling in portrait mode
  document.body.style.height = '100vh'; // Set body height to 100% of viewport height
  document.body.style.display = 'block'; // Use block layout
  document.body.style.justifyContent = 'unset'; // Reset horizontal alignment
//  document.body.style.alignItems = 'flex-start'; // Align items to the top
  document.querySelector('.container').classList.remove('scrollable'); // Remove scrolling class from container
  window.scrollTo(0, 0); // Scroll to the top
 } else { // If in landscape mode (width >= height)
  document.documentElement.style.setProperty('--scale-factor', 1); // Reset scale factor to 1 (no scaling)
  document.body.style.overflow = 'auto'; // Enable scrolling in landscape mode
  document.body.style.height = 'auto'; // Set body height to auto
  document.body.style.display = 'flex'; // Use flexbox layout
  document.body.style.justifyContent = 'center'; // Center content horizontally
  document.body.style.alignItems = 'center'; // Center content vertically
  document.querySelector('.container').classList.add('scrollable'); // Add scrolling class to container
 }
}
window.addEventListener('resize', updateScaleFactor); // Update scale factor on window resize
window.addEventListener('load', updateScaleFactor); // Update scale factor when page loads
</script>
</body>
</html>
)rawliteral";

// ====== Configuration Manager web page after save data

const char CfgMgr_Saved[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
<html lang="en">
<script>
function startCountdown() {
 var timeLeft = 25;
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
Wait <span id="countdown">25</span> seconds for reboot to finish.<br><br>
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