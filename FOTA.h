/* ----------------------------------------------------------------------------

FOTA.h : 'Firmware On The Air' upload sketches remotely.

  FOTA.h holds 4 variables an 1 PROGMEM for FOTA.ino
    Nice buttons

---------------------------------------------------------------------------- */

// ----- Some global variables

String FOTA_user;
String FOTA_password;

unsigned long FOTA_bytes_written;  // only used to log end message
String FOTA_update_error;          // we may need to return an error code

// ====== Upload interface

const char FOTA_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
<head>
<meta name='viewport' content='width=device-width, height=device-height, initial-scale=1, user-scalable=no'>
<title>Firmware On The Air</title>
<style>
  :root {
   --scale-factor: 1;
  }
  body {
   background-color: #E6E6FA;
   margin: 0;
   padding: 0;
   height: 100vh;
   display: flex;
   justify-content: center;
//   align-items: center; // center page vertically or not
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
  .rim-only-button {
    width: 250px;
    height: 56px;
    background-color: lightgrey;
    border-width:3px;
    border-color:gold;
    border-radius: 50%;
    color: blue;
    text-align: center;
    font-size: 16px;
  }
  .file-input-wrapper {
    background-color: transparent;
    border-radius: 20px;
    width: 250px;
    height: 50px;
    top: -47px;
    position: relative;
    overflow: hidden;
    display: inline-block;
  }
  .file-input {
    font-size: 100px;
    position: absolute;
    left: 0;
    top: 0;
    opacity: 0;
  }
  .file-label {
    background-color: transparent;
    color: blue;
    font-size: 16px;
    cursor: pointer;
    display: inline-block;
    padding: 10px 20px;
  }
  .back_btn, .upload_btn, .reboot_btn {
    background-color: lightgrey;
    border-width:3px;
    border-color:gold;
    border-radius: 50%;
    color: blue;
    text-align: center;
    font-size: 16px;
  }
  .back_btn {
    width: 100px;
    height: 50px;
  }
  .upload_btn {
    width: 200px;
    height: 50px;
  }
  .reboot_btn {
    background-color: yellow;
    width: 100px;
    height: 50px;
    margin: 0 auto;
  }
</style>
</head>
<body>
<div class="container">
<div id="FOTA">
  <h1>Firmware On The Air</h1>
  <button type="button" onclick="history.back()" class="back_btn">Back</button><br><br><br>
  <form onsubmit="event.preventDefault(); uploadFile();">
    <div>
    <button id="rim-only-button" class="rim-only-button"></button>
    </div>
    <div id="file-input-wrapper" class="file-input-wrapper">
      <input type="file" id="firmwareFile" name="firmwareFile" class="file-input" required onchange="updateSelectedFileName();">
      <label for="firmwareFile" class="file-label">Select firmware file</label>
    </div>
    <div><span id="selectedfilename">Please select a firmware file</span></div>
    <br><br><button type="submit" class="upload_btn">Upload Firmware</button>
  </form>
  <br>
  <div id="progressText">No Firmware Loaded</div>
  <progress id="progressBar" value="0" max="100" style="width:300px;"></progress>
  <br><br>
  <button id="rebootButton" style="display: none;" onclick="reboot()" class="reboot_btn">Reboot</button>
</div>
<div id="REBOOT" style="display:none;">
<h1>
Firmware Upload Complete<br><br>
Wait <span id="countdown">25</span> seconds for reboot to finish...
</h1>
</div>
</div>
</center>
<script>

// ----- Short for document.getElementById function

function _(el) {
  return document.getElementById(el);
}

// ----- Function to update display on selected file

function updateSelectedFileName() {
  _('progressText').innerText = 'Ready for upload';
  const fileInput = _('firmwareFile');
  const fileName = fileInput.files[0].name;
  _('selectedfilename').innerText = 'Ready to upload: ' + fileName;
}

// ----- Upload function

function uploadFile() {
  const fileInput = _('firmwareFile');
  const file = fileInput.files[0];

  const xhr = new XMLHttpRequest();
  const formData = new FormData();
  formData.append('file', file);

  xhr.upload.addEventListener('progress', function(e) {
    if (e.lengthComputable) {
      const percentComplete = (e.loaded / e.total) * 100;
      _('progressBar').value = percentComplete;
      _('progressText').innerText = "Loaded "+percentComplete.toFixed(2) + '%; Upload in progress....';
    }
  });

  xhr.upload.addEventListener('load', function(e) {
    _('progressText').innerText = 'Upload complete';
    _("rebootButton").style.display = "block";
  });

  xhr.addEventListener('load', function(e) {
    if (xhr.status === 200) {
      alert('You may reboot now');
    } else {
      alert('Firmware update failed');
    }
  });

  xhr.addEventListener('error', function(e) {
    alert('An error occurred while uploading the file\n\nYou may retry');
  });

  xhr.open('POST', '/FOTA/upload');
  xhr.send(formData);
}

// ----- Reboot function

function reboot() {
  fetch("/FOTA/reboot")
  _('FOTA').style.display="none";
  _('REBOOT').style.display="block";
  var timeLeft = 25;
  var countdownElement = _("countdown");
  countdownElement.innerHTML = timeLeft;
  var countdownTimer = setInterval(function() {
    timeLeft--;
    countdownElement.innerHTML = timeLeft;
    if (timeLeft <= 0) {
      clearInterval(countdownTimer);
      window.location.href = '/';
    }
  }, 1000);


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
}
</script>
</body>
</html>
)rawliteral";

// ----------------------------------------------------------------------------