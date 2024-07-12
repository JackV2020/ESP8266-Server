/* ----------------------------------------------------------------------------

LittleFSWeb.h holds the web page to manage LittleFSWeb

  This is used to create the actual html /LittleFSWeb/LittleFSWeb.html.

---------------------------------------------------------------------------- */

const char LittleFSWeb[] PROGMEM=R"rawliteral(<!DOCTYPE HTML>
<html lang="en">
<!--
  You can edit /LittleFSWeb/LittleFSWeb.html to make changes to the page.
  Safer is :
  - make a copy to for example /LittleFSWeb/MyTest.html
  - edit and test that file until you are happy with it
  - edit and save your result in /LittleFSWeb/LittleFSWeb.html
  hint when you have an editor with a search summary pane, search for 'function'
-->
<head>
 <meta name='viewport' content='width=device-width, height=device-height, initial-scale=1, user-scalable=no'>
 <link rel="icon" type="image/x-icon" href="/favicon.ico">
 <meta charset="UTF-8">
 <title>LittleFSWeb</title>
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
//    box-shadow: 0px 0px 15px 0px blue;
//    box-shadow: 3px 3px 5px 0px rgba(0,0,0,0.75);
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

 </style>

</head>
<body id='body' onload='listFiles()'>
 <div class="container">

<!-- LittleFSWeb -->

 <div style="display:block;" id="LittleFSWeb">
 <h1>LittleFSWeb</h1>

  <a href="/"><button class="rnd_btn">Home</button></a>
  <button class="rnd_btn" onclick="refresh()">Refresh</button>
  <button class="rnd_btn" onclick="littlefswebInfoToggle()">Info</button>
  <!-- button class="rnd_btn" onclick="logout()">Logout</button -->


 <div style="display:none;" id='info'>
<br><hr>
<h3>With LittleFSWeb you can :</h3>
Manage (sub)directories and files.<br>
(use rename to move between directories.)<br><br>

Build a web site with your own html files.<br>
- Upload/create files on your web site.<br>
- Default in any (sub)directory is index.html.<br>
- Edit .html*, .css*, .js*, .svg, .json and .txt files.<br>
*allow extra .proc extension to use proc_processor().<br>
(see example 'proc_processor.html.proc').<br>
*or extra .gz extension for gzipped files.<br>
(gz takes precedence over uncompressed files)<br>
- Change /index.html for your own needs<br>
and if you want /LittleFSWeb/LittleFSWeb.html.<br>
- Maybe create /LittleFSWeb/LittleFSWeb.html.gz.<br><br>

- And when you want a clean start...Format<br>
<h5><font color=red>This is a 'quantum' computer so...</font></h5>
- When you upload/copy/save a 'huge' file...<br>
your server is very, very busy for you<br>
...just be patient and wait...<br>
<br><hr><br>
 </div>
 <p id="statusLittleFSWeb"></p>

  <div id="detailsLittleFSWeb" class="centered-content"></div><br>

  <div id="bottombuttons" style="display:none">

   <button class="rnd_btn" onclick="doFunction('New Directory')">New Dir</button>
   <button class="rnd_btn" onclick="doFunction('New File')">New File</button>
   <button class="rnd_btn" onclick="doFunction('Edit File')">Edit</button>
   <br>

   <button class="rnd_btn" onclick="doFunction('Copy')">Copy</button>
   <button class="rnd_btn" onclick="doFunction('Upload')">Upload</button>
   <button class="rnd_btn" onclick="doFunction('Rename')">Rename</button>

   <br>

   <button class="rnd_btn" style='background-color:pink;'
    onclick="doFunction('Delete')">Delete</button>
   <button class="rnd_btn" onclick="doFunction('Download')">Download</button>
   <button class="rnd_btn" style='background-color:pink;'
    onclick="doFunction('Format')">Format</button>
  </div>
 </div>

<!-- LittleFSWeb Editor -->

 <div style="display:none;"  id="LittleFSWebEditor">
 <h1>LittleFSWeb Editor</h1>
  <button class="rnd_btn" onclick="listFiles()">Exit</button>
  <button class="rnd_btn" onclick="saveFile()">Save</button>
  <button class="rnd_btn" onclick="getFile()">Reload</button>
 <p id="statusEditor"></p>

  <input type="text" id="filename" name="filename" size="40"><br>
  <button class="rnd_btn" style="width:30px;height:30px;margin-top:5px;
    margin-bottom:5px;margin-right:20px;font-size:20px;font-weight:bold;"
    onclick="setFontSize('-')">-</button>
  <button class="rnd_btn" style="width:30px;height:30px;margin-left:20px;
    font-size:20px;font-weight:bold;"
    onclick="setFontSize('+')">+</button><br>
  <textarea id="fileContent" name="fileContent" style =
    "height:300px;width:300px;white-space:pre;overflow:auto;font-size:100%;"
<!-- onclick="_('statusEditor').innerHTML='';"  -->
      >Editor</textarea><br><br>

 </div>

<!-- LittleFSWeb Upload -->

 <div style="display:none;"  id="LittleFSWebUpload">
   <h1>LittleFSWeb Upload File(s)</h1>
   <div>
    <button id="rim-only-button" class="rim-only-button"></button>
   </div>
   <div id="file-input-wrapper" class="file-input-wrapper">
    <input multiple type="file" name="filesToUpload" id="filesToUpload" class="file-input" required onchange="uploadFiles()">
    <label for="filesToUpload" class="file-label">Select & Upload File(s)</label>
   </div>
   <br>
   <progress id="progressTotal" value="0" max="100" style="width:300px;"></progress><br>
   <p id="statusUpload"></p>
   <progress id="progressFile" value="0" max="100" style="width:300px;"></progress>
   <p id="loaded_n_total"></p>
   <br>
   <button class="rnd_btn" onclick="listFiles();">LittleFSWeb</button>
 </div>

 </div>

<script>

// ------------------------------------------------------ Some general functions

// ----- Short for document.getElementById function
function _(el) {
 return document.getElementById(el);
}

// ----- Function to switch layout

function switchLayout(layout) {
  _("LittleFSWeb").style.display = "none";
  _("LittleFSWebEditor").style.display = "none";
  _("LittleFSWebUpload").style.display = "none";
  _(layout).style.display = "block";
}

// ----- Function to pause some milliseconds

function pausemillis(millis)
{
 var date=new Date();
 var curDate=null;
 do { curDate=new Date(); }
 while(curDate-date < millis);
}

// ------------------------------------------------------------ Editor functions

// ----- Function to increase / decrease fontsize

function setFontSize(plusmin){
 var x=_('fileContent').style.fontSize.slice(0, -1) * 1;
 if (plusmin == "+"){
  x=x + 10;
 } else {
  x=x - 10;
 }
 _('fileContent').style.fontSize=x +"%";
}

// ---------- Functions to resize edit window on mobile Start

const textarea=_('fileContent');

// ----- Variables to store the initial distance and dimensions

let textareaInitialDistance=null;
let textareaInitialWidth=null;
let textareaInitialHeight=null;
let textareaInitialTouches=[];

// ----- Define the minimum and maximum dimensions

const textareaMinWidth=200; // Minimum width in pixels
var textareaMaxWidth=600; // Maximum width in pixels
const textareaMinHeight=300; // Minimum height in pixels
const textareaMaxHeight=800; // Maximum height in pixels

// ----- Calculate the distance between two touch points

function calculateDistance(touch1, touch2) {
 const dx=touch2.pageX - touch1.pageX;
 const dy=touch2.pageY - touch1.pageY;
 return {
  distance: Math.sqrt(dx * dx + dy * dy),
  dx: dx,
  dy: dy
 };
}

// ----- Event listener touchstart function

textarea.addEventListener('touchstart', function(event) {
 _("statusEditor").innerHTML="";
 if (event.touches.length === 2) {
  // Store the initial touch points
  textareaInitialTouches=Array.from(event.touches);

  // Calculate and store the initial distance between the two touch points
  const initialCalculation=calculateDistance(textareaInitialTouches[0],
    textareaInitialTouches[1]);
  textareaInitialDistance=initialCalculation.distance;

  // Store the initial dimensions of the textarea
  textareaInitialWidth=textarea.clientWidth;
  textareaInitialHeight=textarea.clientHeight;

  // Prevent default behavior to allow custom gesture handling
  event.preventDefault();
 }
});

// ----- Event listener touchmove function

textarea.addEventListener('touchmove', function(event) {
 if (event.touches.length === 2) {
  // Calculate the new distance and direction between the two touch points
  const newTouches=Array.from(event.touches);
  const newCalculation=calculateDistance(newTouches[0], newTouches[1]);
  // Calculate the scale factor
  const scale=newCalculation.distance / textareaInitialDistance;

  // Determine whether the movement is primarily horizontal or vertical
  const dx=Math.abs(newCalculation.dx);
  const dy=Math.abs(newCalculation.dy);
  const isHorizontal=dx > dy;

  if (isHorizontal) {
   // Calculate the new width
   let newWidth=textareaInitialWidth * scale;
   // Limit the new width to the minimum and maximum width
   newWidth=Math.min(Math.max(newWidth, textareaMinWidth), textareaMaxWidth);

   // Update the width of the textarea
   textarea.style.width=newWidth + 'px';
  } else {
   // Calculate the new height
   let newHeight=textareaInitialHeight * scale;
   // Limit the new height to the minimum and maximum height
   newHeight=Math.min(Math.max(newHeight, textareaMinHeight), textareaMaxHeight);
   // Update the height of the textarea
   textarea.style.height=newHeight + 'px';
  }

  // Prevent default behavior to allow custom gesture handling
  event.preventDefault();
 }
});

// ----- Event listener touchend function

textarea.addEventListener('touchend', function(event) {
 // Reset initial values when the gesture ends
 textareaInitialDistance=null;
 textareaInitialWidth=null;
 textareaInitialHeight=null;
});

// ---------- Functions to resize edit window on mobile End

// ----- Function to make tab and Ctrl-s work in textarea

textarea.addEventListener('keydown', function(event) {

 if (event.key === 'Tab') {
// Prevent the default behavior (shifting focus)
  event.preventDefault();
// Get the current cursor position in the textarea
  const start=textarea.selectionStart;
  const end=textarea.selectionEnd;
// Get the contents
  const value=textarea.value;
// Insert tab (which is 2 spaces '  ') on current cursor position
  textarea.value=value.substring(0, start)+'  '+value.substring(end);
// Cursor to new position
// tab is 2 spaces so move + 2 characters
  textarea.selectionStart=textarea.selectionEnd=start+2;

 } else if (event.ctrlKey && event.key === 's') {
   event.preventDefault();
   saveFile();
 }
});


// ----- Function to load the file to edit

function getFile() {
 _('fileContent').value="Loading...";
 _("statusEditor").innerHTML="<font color=red>Loading...</font>";
 var d=new Date();
 var h=d.getHours();
 var m=d.getMinutes();
 var s=d.getSeconds();
 var ms=d.getMilliseconds();
 var key="?" + h + m + s + ms;
/*
 NOTE the extra "/LittleFSWebFetch" before the path
 this is handled by LittleFSWeb so the file is not processed by
 the proc_processor in the web server of the main app.
 This would process strings beween % signs and we could not edit .proc files.
*/
 const filename="/LittleFSWebFetch"+_('filename').value+key;
 const url=`${filename}`; // Construct the URL for the file

 fetch(url)
  .then(response => response.text())
  .then(data => {
   console.log("Data length received:", data.length);
   _("statusEditor").innerHTML="<font color=green>Loaded</font>";
   _('fileContent').value=data;
  })
  .catch(err => {
   console.error(err);
   _("statusEditor").innerHTML="<font color=red>Error loading file.</font>";
  });
}

// ----- Function to save the file ( the file is sent in chunks )

function saveFile() {

  // Get the filename from the input field
 const filename=_('filename').value;
 // Get the file content from the textarea
 const fileContent=_('fileContent').value;
 // URL of the server-side endpoint
 const url='/LittleFSWeb/saveFile';
 // Create a FormData object and append the filename and file content to it
 const formData=new FormData();
 formData.append('filename', filename);
 formData.append('isLastChunk', false);
// formData.append('fileSize', "12345");
 const d = new Date();
 formData.append('sessionID', d.getTime());
 formData.append('fileSize', _('fileContent').value.length);
 // Convert file content into a Blob and split it into chunks

 // Define the chunk size in bytes (you can adjust it as needed)
 const chunkSize=1024;
 const contentBlob=new Blob([fileContent], { type: 'text/plain' });

 // Use fetch with a POST request to send data in chunks

 const sendChunk=(start, end) => {
  const chunkBlob=contentBlob.slice(start, end);
  formData.set('fileContent', chunkBlob);

  // Determine if this is the last chunk
  const isLastChunk=end >= contentBlob.size;
  formData.set('isLastChunk', isLastChunk ? 'true' : 'false');

  fetch(url, {
    method: 'POST',
    body: formData
  })

  .then(response => {

    if (!response.ok) {
      throw new Error(`HTTP error! Status: ${response.status}`);
    }

    // Check if there is more data to send and proceed accordingly

    if (end < contentBlob.size) {
     _("statusEditor").innerHTML="<font color=red>Saving...</font> "
      + end
      + " / "
      + contentBlob.size;
      sendChunk(end, end + chunkSize);

    } else {

      _("statusEditor").innerHTML="<font color=red>Closing...</font>";

      setTimeout(() => {
      // Wait 3 seconds to allow the ESP to receive last chunk and close file
        var startDate=new Date();
        var endDate=new Date();
        while ((endDate.getTime() - startDate.getTime()) / 1000 < 3) {
          endDate=new Date();
        }
// get last status
        xmlhttp=new XMLHttpRequest();
        xmlhttp.open("GET", "/LittleFSWeb/lastSaveStatus", true);
        xmlhttp.onreadystatechange = () => {
          if (xmlhttp.readyState === XMLHttpRequest.DONE) {
            if (xmlhttp.responseText == "oke") {
              _("statusEditor").innerHTML="<font color=green>File save oke.</font>";
            } else {
              _("statusEditor").innerHTML="<font color=red>File save failed.</font>";
            }
          }
        }
        xmlhttp.send();
      }, 200); // Start this function after 200 milliseconds

    }
  })

  .catch(err => {
    console.error(err);
    _("statusEditor").innerHTML="<font color=red>Error saving file.</font>";
  });

 };
 // Start the chunk upload process
 sendChunk(0, chunkSize);
}

// ------------------------------------------------------- Main screen functions

// ----- Info toggle function

var littlefswebInfo=false;
function littlefswebInfoToggle() {
 littlefswebInfo=!littlefswebInfo;
 if (littlefswebInfo) {
  _('info').style.display="block";
 } else {
  _('info').style.display="none";
 }
}

// ----- Logout function

function logout() {
 // Supply invalid credentials to logout and redirect to root
 var xhr = new XMLHttpRequest();
 xhr.open("GET", "/LittleFSWeb/logout", true, "invaliduser", "invalidpassword");
 xhr.onreadystatechange = function () {
  if (xhr.readyState == 4) {
   window.location.href = "/";
  }
 }
 xhr.send();
}

// ----- List files functions

function listFiles() {
 switchLayout("LittleFSWeb");
 xmlhttp=new XMLHttpRequest();
 xmlhttp.open("GET", "/LittleFSWeb/listfiles", true);
 xmlhttp.onreadystatechange = () => {
  // In local files, status is 0 upon success in Mozilla Firefox
  if (xmlhttp.readyState === XMLHttpRequest.DONE) {
   const status = xmlhttp.status;
   if (status === 0 || (status >= 200 && status < 400)) {
    // The request has been completed successfully
    console.log(xmlhttp.responseText);
   if (xmlhttp.responseText.indexOf("Counts") > 0) {
    // first row for root folder
    var table="<div id='filesdiv' style='height: 400px; overflow-y: auto;'>"
    +"<table id='filestable' style='border: 1px solid black; "
    + "border-collapse: collapse; background-color: #F6F6FA'>"
    +"<tr align=\'left\'>"
    +'<td nowrap align=\'center\' width=40>&nbsp;&nbsp;<input type="radio"'
    + 'name="rowSelect" value="row-1">&nbsp;&nbsp;</td>'
    +"<td>/</td>"
    +"<td align='right'>[Dir]</td>"
    +"<td width=20>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>"
    +"</tr>";
    var mainArray=xmlhttp.responseText.split("|");
    var sA;
    for (var i=0; i < mainArray.length; i++) {
     sA=mainArray[i].split(",");
     if (sA[0] == 'LittleFSCopyStatus') {
      if (sA[1] == "Copying") {
       _("statusLittleFSWeb").innerHTML="Copying";
      }
     } else if (sA[0] == 'Counts') {table=table
// last rows with totals
      +'<tr align=\'right\' style=\'border: 1px solid black;\'>'
      +"<td></td>"
      +'<td>'+sA[1]+' Dirs; '+sA[2]+' Files \>&nbsp;</td>'
      +'<td>'+sA[3]+'</td>'
      +'</tr>'
      +'<tr align=\'right\' style=\'border: 1px solid black;\'>'
      +'<td></td><td>Available \>&nbsp;</td>'
      +'<td>'+sA[4]+'</td>'
      +'</tr>'
//      +'<tr align=\'right\' style=\'border: 1px solid black;\'>'
//      +'<td></td><td>Total \>&nbsp;</td>'
//      +'<td nowrap>'+sA[5]+'</td>'
//      +'</tr>';
     } else if (sA[1] == '[Dir]') {table=table
// directory row
      +'<tr align=\'left\' style=\'border: 1px solid black;\'>'
      +'<td align=\'center\' width=40>'
      +'<input type="radio" name="rowSelect" value="row-1"></td>'
      +'<td>'+sA[0]+'</td>'
      +'<td align=\'right\'>'+sA[1]+'</td>'
      +'</tr>';
     }
     else {table=table
// file row
      +'<tr align=\'left\' style=\'border: 1px solid black;\'>'
      +'<td align=\'center\' width=40>'
      +'<input type="radio" name="rowSelect" value="row-1"></td>'
// NOTE for below the '?number' forces a browser which cached a copy to
//  redownload the file so it shows the last updated file
      +'<td nowrap><a href="' + sA[0]
        + '?' + Math.floor(Math.random() * 1000000)
        + '">' + sA[0] + '</a></td>'
      +'<td nowrap align=\'right\'>'+sA[1]+'</td>'
      +'</tr>';
     }
    }
    table += "</table></div>";
    _("detailsLittleFSWeb").innerHTML=table;
    _("filesdiv").style.width=(_("filestable").offsetWidth) +"px";
    _("bottombuttons").style.display='block';

   }
  } else {
   alert("listFiles request error!");
  }
 }
};
 xmlhttp.send();
}

// ----- Refresh list function

function refresh() {
  _("statusLittleFSWeb").innerHTML="";
  listFiles();
}

// ------------------------------------------------------------ Upload functions

var uploadFolder="*";

var nr_files;
var nr_files_ready;
let fileQueue=[]; // Queue to store files
let isUploading=false; // Flag to indicate if an upload is in progress
let total_bytes;
let total_bytes_sent;

function uploadFiles() {

 _("statusUpload").innerHTML="";

 // Set the upload directory to get free file system space
 const xmlhttp=new XMLHttpRequest();
 xmlhttp.open("GET",
  "/LittleFSWeb/setUploadDirectory?name=" + uploadFolder, false);
 xmlhttp.send();

 var free_bytes=xmlhttp.responseText;
 total_bytes=0
 total_bytes_sent=0
 nr_files_ready=0;
 // Get the number of files to be uploaded
 nr_files=_("filesToUpload").files.length;
 // Add files to the queue
 fileQueue=[];
 var oke=true;
 for (let i=0; i < nr_files; i++) {
  const file=_("filesToUpload").files[i];
  total_bytes=total_bytes + file.size;
console.log(total_bytes+" free: "+free_bytes);
  if (file.name.length > 31) {
   alert("Error: Uploads canceled! Filename too long: "
     + file.name.length + " (max 31)\n\nfilename: " + file.name);
   _("statusUpload").innerHTML="<font color=red>Upload Canceled</font>";
   oke=false;
   break;
  } else if (total_bytes > free_bytes) {
   alert("Error: Uploads canceled! Not enough disk space");
   _("statusUpload").innerHTML="<font color=red>Upload Canceled</font>";
   oke=false;
   break;
  } else {
   fileQueue.push(file); // Add the file to the queue
  }
 }

 // Start processing the queue
 if (oke) { processQueue(); } else { listFiles();}
}

function processQueue() {
 // Check if there is an upload in progress or the queue is empty
 if (isUploading || fileQueue.length === 0) {
  return;
 }

 // Set the upload directory to set folder for this file
 const xmlhttp=new XMLHttpRequest();
 xmlhttp.open("GET",
  "/LittleFSWeb/setUploadDirectory?name=" + uploadFolder, false);
 xmlhttp.send();

 // Indicate that an upload is in progress
 isUploading=true;
 // Retrieve the next file from the queue
 const file=fileQueue.shift();
 const fileName=file.name; // Get the file name

 // Create a FormData object and add the file
 const formdata=new FormData();
 formdata.append("filesToUpload", file);

 // Create a new XMLHttpRequest
 const ajax=new XMLHttpRequest();
 // Attach event listeners and pass the file name to the progressHandler
 ajax.upload.addEventListener("progress", function(event) {
  progressHandler(event, fileName);
 }, false);
 ajax.addEventListener("load", completeHandler, false);
 ajax.addEventListener("error", errorHandler, false);
 ajax.addEventListener("abort", abortHandler, false);

 ajax.open("POST", "/", true);

 // Add a callback for when the upload ends
 ajax.onloadend=function () {
  isUploading=false; // Reset the upload flag
  processQueue(); // Continue processing the queue
 };

 // Send the form data
 ajax.send(formdata);
 pausemillis(1000);
}

let previousprogress = 0;

function progressHandler(event, fileName) {

 var percentFile=(event.loaded / event.total) * 100;

 total_bytes_sent = total_bytes_sent + (event.loaded - previousprogress);
 var percentTotal=(total_bytes_sent / total_bytes) * 100;

 // Display progress information

// _("loaded_n_total").innerHTML=nr_files_ready + " file(s) ready, Uploaded " +
//    event.loaded + " bytes of " + event.total + " for file: " + fileName;

 _("loaded_n_total").innerHTML="Uploading " + fileName + "<br>" +
    event.loaded + " / " + event.total;

 _("progressTotal").value=Math.round(percentTotal);
 _("progressFile").value=Math.round(percentFile);

 if (percentFile >= 100) {
  _("statusUpload").innerHTML="Please wait, writing file to filesystem";
  previousprogress = 0;
 } else {
 _("statusUpload").innerHTML=Math.round(percentTotal) + "% uploaded... please wait";
  previousprogress = event.loaded
 }

}

function completeHandler(event) {
 nr_files_ready=nr_files_ready + 1;
 if(nr_files_ready == nr_files) {
  _("loaded_n_total").innerHTML="";
//  _("progressFile").value=0;
//  _("progressTotal").value=0;
  _("statusUpload").innerHTML="Upload "+nr_files+" file(s) complete";
//  listFiles();
 }
}

function errorHandler(event) {
 _("statusUpload").innerHTML="Upload Failed";
}
function abortHandler(event) {
 _("statusUpload").innerHTML="Upload Aborted";
}

// ---------------------------------------------------- Bottom buttons functions

function doFunction(func) {

 if (func == "Format") {
  ValidateAndAct(func,"*"); }
 else {

  var selectedInput = document.querySelector('input[name="rowSelect"]:checked');
  var selectedRow = selectedInput.parentNode.parentNode;
  var selectedRowValue = selectedRow.children[1].textContent;
  var selectedRowType = selectedRow.children[2].textContent;

  switch (func) {
   case "New Directory":
    if (selectedRowType != "[Dir]" ) {
     alert ("Can not create directory in file")
    } else {
     if (selectedRowValue =="/") {selectedRowValue="" }
     ValidateAndAct(func,selectedRowValue);
    }
   break;
   case "New File":
    if (selectedRowType != "[Dir]" ) {
     alert ("Can not create file in file")
    } else {
     if (selectedRowValue =="/") {selectedRowValue="" }
     ValidateAndAct(func,selectedRowValue);
    }
   break;
   case "Edit File":
    if (selectedRowType == "[Dir]" ) {
     alert ("Can not edit directory")
    } else {
     switchLayout("LittleFSWebEditor");
     var filetypes=['txt','html','css','js','svg','json','proc'];
     var ftype=selectedRowValue.substring(selectedRowValue.lastIndexOf(".")+1);
     if (filetypes.includes(ftype)) {
      _("statusEditor").innerHTML="Loading...";
      _("filename").value=selectedRowValue;
      getFile();
     } else {
      alert ("Valid types :\n\n ['txt','html','css','js','svg','json','proc']")
     }
    }
   break;
   case "Save File":
    _("statusEditor").innerHTML="Saving...";
    saveFile();
   break;
   case "Upload":
    if (selectedRowType != "[Dir]" ) {
     alert ("Can not upload in file")
    } else {
     uploadFolder=selectedRowValue + "/";
     _("statusUpload").innerHTML="";
     _("progressFile").value=0;
     _("progressTotal").value=0;
     _("statusUpload").innerHTML="Select file(s) to upload";
     switchLayout("LittleFSWebUpload");
//     showUploadButtonFancy(selectedRowValue);
    }
   break;
   case "Copy":
    if (selectedRowType == "[Dir]" ) {
     alert ("Can not copy directories")
    } else {
     ValidateAndAct(func,selectedRowValue);
    }
   break;
   case "Rename":
    ValidateAndAct(func,selectedRowValue);
   break;
   case "Download":
    if (selectedRowType == "[Dir]" ) {
     alert ("Can not download directories")
    } else {
     // Create a dummy anchor element
     var a=document.createElement('a');
     // Set the href attribute to the path of the file to be downloaded
/*
 NOTE the extra "/LittleFSWebFetch" before the path
 this is handled by LittleFSWeb so the file is not processed by
 the proc_processor in the web server of the main app.
 This would process strings beween % signs and change downloading .proc files.
*/
     a.href="/LittleFSWebFetch"+selectedRowValue;
     // Download attribute = the filename to be saved, just a dummy value
     a.download=selectedRowValue.substring(selectedRowValue.lastIndexOf("/")+1);
     // Append the anchor element to the document body
     document.body.appendChild(a);
     // Programmatically trigger a click event on the anchor element
     a.click();
     // Remove the anchor element from the document body
     document.body.removeChild(a);
    }
   break;
   case "Delete":
    if (selectedRowType == "[Dir]" ) {
     if (selectedRowValue == "/") {
      alert("Use Format to erase disk")
     } else {
      ValidateAndAct("Delete Directory",selectedRowValue);
     }
    } else {
     ValidateAndAct(func,selectedRowValue);
    }
   break;
   case "Format":
    ValidateAndAct(func,"*");
   break;
   default:
    alert("Not implemented : "+func);
  }
 }
}

function ValidateAndAct(func, parm1) {
 // Clear prompt
 var prompt="";

 // Create a modal overlay
 var overlay=document.createElement("div");
 overlay.style.position="fixed";
 overlay.style.top="0";
 overlay.style.left="0";
 overlay.style.width="100%"; // full width
 overlay.style.height="100%"; // full height
 overlay.style.backgroundColor="rgba(0, 0, 0, 0.5)";
 overlay.style.display="flex";
 overlay.style.alignItems="center";
 overlay.style.justifyContent="center";
 document.body.appendChild(overlay);

 // Create a modal container
 var modal=document.createElement("div");
 modal.style.align="center";
 if ((func == "Delete")
  || (func == "Delete Directory")
  || (func == "Format") ) {
  modal.style.backgroundColor="#faa";
 } else {
  modal.style.backgroundColor="#aaf";
 }
 modal.style.padding="20px";
 modal.style.borderRadius="8px";
 modal.style.width="300px"; // Fixed width
 overlay.appendChild(modal);

 // Set the custom title
 var title=document.createElement("div");
 if (parm1 == ""){
  title.innerText="LittleFS\n\n" + func + "\n\nroot\n\n";
 } else if (func == "Copy") {
  title.innerText="LittleFS\n\n" + func + " (be patient)\n\n" + parm1 + "\n\n";
 } else {
  title.innerText="LittleFS\n\n" + func + "\n\n" + parm1 + "\n\n";
 }
 title.style.fontWeight="bold";
 title.style.marginBottom="10px";
 // Center the title
 title.style.textAlign="center";
 // Center element horizontally within its container
 title.style.margin="0 auto";

 modal.appendChild(title);

 // Create an input field for prompt
 var promptInput=document.createElement("input");
 promptInput.id="promptInput";
 if ((func == "Delete") || (func == "Delete Directory")) {
  promptInput.type="hidden";
 } else {
  promptInput.type="text";
 }
 if ( (func == "Copy") || (func == "Rename") ) {
  promptInput.placeholder="Type the new name";
 } else if ((func == "New Directory") || (func == "New File") ){
  promptInput.value=parm1 +"/";
 } else if (func == "Format") {
  promptInput.placeholder="Type Format to confirm";
 } else { // Delete Directory || Delete
  promptInput.value="dummy input";
 }
 promptInput.style.width="280px"; // Fixed width
 promptInput.style.marginBottom="10px"; // Adjust the spacing
 promptInput.style.textAlign="center"; // Center text horizontally
 modal.appendChild(promptInput);

 // Create a button container
 var buttonContainer=document.createElement("div");
 buttonContainer.style.display="flex";
 buttonContainer.style.justifyContent="space-between";
 modal.appendChild(buttonContainer);

 // Create a button to enter new value
 var eButton=document.createElement("button");
 eButton.id="eButton";
 if ((func == "Delete") || (func == "Delete Directory")) {
  eButton.innerText="Confirm";
 } else {
  eButton.innerText="Enter";
 }
 eButton.style.width="144px"; // Fixed width
 eButton.style.fontSize="14px"; // Adjust the font size

 eButton.onclick=function() {
  prompt=promptInput.value;
  overlay.parentNode.removeChild(overlay); // Remove the overlay/modal
  if (prompt != "") {
   var urltocall;
   var ok=false;
   if (func == "Format") {
    ok=( "Format" == prompt );
    urltocall="/LittleFSWeb/format?name=" + prompt + "&action=format";
   }

   if (func == "Copy") {
    ok=true;
    urltocall="/LittleFSWeb/file?name=" + parm1 + "&action=copy@" + prompt;
   }
   if (func == "Rename") {
    ok=true;
    urltocall="/LittleFSWeb/file?name=" + parm1 + "&action=rename@" + prompt;
   }
   if (func == "Delete") {
    ok=( parm1 == prompt );
    ok=true;
    urltocall="/LittleFSWeb/file?name=" + parm1 + "&action=delete";
   }

   if (func == "New Directory") {
    ok=true;
    urltocall="/LittleFSWeb/dir?name=" + prompt + "&action=newdir";
   }
   if (func == "New File") {
    ok=true;
    urltocall="/LittleFSWeb/file?name=" + prompt + "&action=newfile";
   }
   if (func == "Delete Directory") {
    ok=( parm1 == prompt );
    ok=true;
    urltocall="/LittleFSWeb/dir?name=" + parm1 + "&action=deldir";
   }
   if (ok) {
    _("statusLittleFSWeb").innerHTML="Please wait...";
    xmlhttp=new XMLHttpRequest();
    xmlhttp.open("GET", urltocall, false);
    xmlhttp.send();
    _("statusLittleFSWeb").innerHTML=xmlhttp.responseText;
    listFiles();
   } else {
    alert ("Wrong value: " + prompt + " \<\> " + parm1 );
   }
  }
 };
 buttonContainer.appendChild(eButton);

 // Create a button to cancel
 var cButton=document.createElement("button");
 cButton.innerText="Cancel";
 cButton.style.width="144px";
 cButton.style.fontSize="14px";
 cButton.onclick=function() {
  overlay.parentNode.removeChild(overlay);
 };
 buttonContainer.appendChild(cButton);

 // Check if the Enter key was pressed (keyCode 13) or the Go button on a phone
 _("promptInput").addEventListener("keyup", function(event) {
   if (event.keyCode === 13 || event.key === "Enter") {
     _("eButton").click();
   }
 });

}

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

// ----------------------------------------------------------------------------