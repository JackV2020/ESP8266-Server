/* ----------------------------------------------------------------------------

HTMLClientExampleGPIOs.h

  An example created by wsServerConfigureWebSocketServer when it does not exist.

---------------------------------------------------------------------------- */

const char HTMLClientExampleGPIOs[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
<head>
<title>Wemos GPIO Example</title>
<script>
// force favicon refresh
document.addEventListener("DOMContentLoaded", function() {
 document.querySelector("link[rel='icon']").href =
  `/favicon.ico?${Math.floor(100000 + Math.random() * 900000)}`;
});
// Define the list of GPIO pins and D numbers
const GPIOs =  [ 2, 4, 5, 12, 13, 14];
const GPIODs = [ 4, 2, 1,  6,  7,  5];

// ---------------------------------- WebSocket Connection

var socket;

// Function to initialize the WebSocket
function initWebSocket() {

 socket = new WebSocket('ws://' + window.location.hostname + '/wsServer/ws');

 socket.onmessage = function(event) { handleMessage(event.data);};

 socket.onclose = function(event) {
  console.log("Attempting to reconnect in 1 second...");
  setTimeout(initWebSocket, 1000);
 };

 socket.onerror = function(error) {
  console.error("WebSocket error:", error);
 };

// Request initial data from the server
 setTimeout(function() { connect(); }, 500);
}

function connect() { socket.send("10");}

// ------------------------- WebSocket Receive From Server

function wsServerGetPart(string,part) {
 let SEPERATOR ="||";
 let SEPERATORSIZE = 2;
 let startIndex = 0;
 let endIndex = - SEPERATORSIZE ; // '-length' of seperator
 for (var i = 0; i < part; i++) {
  if (endIndex == -1) { // part is missing so we return NULL
   return "";
  }
  startIndex = endIndex + SEPERATORSIZE;
  endIndex = string.indexOf(SEPERATOR, startIndex);
 }
 if (endIndex == -1) {
  return string.substring(startIndex);
 } else {
  return string.substring(startIndex, endIndex);
 }
}

function handleMessage(data) {
 console.log("Receive: "+data);
 let type = wsServerGetPart(data, 1);
 let GPIOrow = "row"+wsServerGetPart(data, 2);
 let Value = wsServerGetPart(data, 3);
 let GPIO="x";
 let button;

 switch (type) {

 case "11": // INPUT
  document.getElementById(GPIOrow).querySelector('.mode').value = 0;
  document.getElementById(GPIOrow).querySelector('.slider').value = Value;
  document.getElementById(GPIOrow).querySelector('.valueInput').value = Value;
 break;
 case "12": // OUTPUT
  document.getElementById(GPIOrow).querySelector('.mode').value = 1;
  document.getElementById(GPIOrow).querySelector('.slider').value = Value;
  document.getElementById(GPIOrow).querySelector('.valueInput').value = Value;
 break;
 case "13": // INPUT_PULLUP
  document.getElementById(GPIOrow).querySelector('.mode').value = 2;
  document.getElementById(GPIOrow).querySelector('.slider').value = Value;
  document.getElementById(GPIOrow).querySelector('.valueInput').value = Value;
 break;
 case "14": // PWM
  document.getElementById(GPIOrow).querySelector('.mode').value = 3;
  document.getElementById(GPIOrow).querySelector('.slider').value = Value;
  document.getElementById(GPIOrow).querySelector('.valueInput').value = Value;
 break;
 case "15": // BLINK
  document.getElementById(GPIOrow).querySelector('.mode').value = 4;
  document.getElementById(GPIOrow).querySelector('.slider').value = Value;
  document.getElementById(GPIOrow).querySelector('.valueInput').value = Value;
 break;

 case "16": // Analog Ticker
  console.log("Analog ticker "+Value);
  button = document.getElementById("toggleA0button");
  if (Value == 0) { button.innerHTML = "A0 Stopped"}
  else {button.innerHTML = "A0 Active"}
 break;

 case "17": // DIGITAL
  Value = Value * 255;
  document.getElementById(GPIOrow).querySelector('.slider').value = Value;
  document.getElementById(GPIOrow).querySelector('.valueInput').value = Value;
 break;

 case "18": // PWM
  document.getElementById(GPIOrow).querySelector('.slider').value = Value;
  document.getElementById(GPIOrow).querySelector('.valueInput').value = Value;
 break;

 case "101": // DIGITAL
  Value = Value * 255;
  document.getElementById(GPIOrow).querySelector('.slider').value = Value;
  document.getElementById(GPIOrow).querySelector('.valueInput').value = Value;
 break;

 case "102": // Ticker data
  document.getElementById('A0reading').innerText = ("0000" + Value).substr(-4);
  document.getElementById('A0voltage').innerText =
   ((Math.round( Value / 1024 * 3.3 * 1000) / 1000) + "0000").substr(0,5);
 break;

 case "100":
  let mode = 0; let pwm = 0;
  for ( var i = 1 ; i < 7; i++ ) {
   GPIO = wsServerGetPart(data, i * 3 - 1)
   mode = wsServerGetPart(data, i * 3)
   Value = wsServerGetPart(data, i * 3 + 1)
   console.log("Received: GPIO: "+GPIO+" Mode: "+mode+" Value: "+Value);

   var row = document.getElementById('row' + GPIO);
   if (row) {
    row.querySelector('.mode').value = mode;
    row.querySelector('.slider').value = Value;
    row.querySelector('.valueInput').value = Value;
   }

  }
  Value=wsServerGetPart(data,20);
  document.getElementById('A0reading').innerText = Value;
  document.getElementById('A0voltage').innerText =
   ((Math.round( Value / 1024 * 3.3 * 1000) / 1000) + "0000").substr(0,5);

  button = document.getElementById("toggleA0button");
  Value=wsServerGetPart(data,21);
  if (Value == 0) { button.innerHTML = "A0 Stopped"}
  else {button.innerHTML = "A0 Active"}
 break;
 }
}

// ---------------------------------------- Send To Server

function sendData(what, pin, value) {
 console.log("Send: "+what+"||"+pin+"||"+value);
 switch (what) {
 case "10": // ask update
  socket.send(what);
  break;
 case "16": // attach / detach ticker
 case "17": // digital
 case "18": // pwm
  socket.send(what+"||"+pin+"||"+value);
 break;
 default:  // all other
  socket.send(what+"||"+pin);
 }
}

function handleModeChange(pin) {
// Modes : INPUT = 0; OUTPUT = 1; INPUT_PULLUP = 2; PWM = 3; BLINK = 4
// Transactions: "1" + (mode value + 1)
 var mode = parseInt(document.querySelector('#row' + pin + ' .mode').value) + 1;
 sendData("1"+mode,pin);
}

function handleSliderChange(pin) {
 var value= document.querySelector('#row' + pin + ' .slider').value;
 var mode= document.querySelector('#row' + pin + ' .mode').value;
 if (mode == 3) { // pwm
   sendData( "18", pin , value);
 } else if ((mode == 1) || (mode == 4)) { // digital output || blink
   sendData( "17", pin , Math.floor(value/128) ); // 0 < value < 255 --> 0 or 1
 } else {
   sendData( "10"); // Force correction when mode is not an output mode
 }
}

function handleInputChange(pin) {
 var value = document.querySelector('#row' + pin + ' .valueInput').value;
 var mode= document.querySelector('#row' + pin + ' .mode').value;
 if (mode == 3) { // pwm
  sendData( "18", pin , value);
 } else if ((mode == 1) || (mode == 4)) {
  sendData( "17", pin , value);
 } else {
  sendData( "10");
 }
}

// ------------------------------------- Create table rows

// Function to create the table rows based on GPIO list

function createGPIOTable() {
 const tableBody = document.getElementById('gpioTableBody');
 for (let i = 0; i < GPIOs.length; i++) {
  const pin = GPIOs[i];
  const gpioD = GPIODs[i];

//----- New row

  const row = document.createElement('tr');
  row.id = 'row' + pin;

//----- GPIO

  const pinCell = document.createElement('td');
  pinCell.style.textAlign = "center";
  pinCell.innerText = pin;
  row.appendChild(pinCell);

//----- D

  const DCell = document.createElement('td');
  DCell.style.textAlign = "center";
  DCell.innerText = 'D'+gpioD;
  row.appendChild(DCell);

//----- mode

  // Create cell for mode select and mode display
  const modeCell = document.createElement('td');
  modeCell.style.textAlign = "center";

  const modeSelect = document.createElement('select');
  modeSelect.className = 'mode';
  modeSelect.onchange = function() { handleModeChange(pin); };
  if (pin == 2) { 
  // OUTPUT = 1 ,PWM = 4 , BLINK = 5 ( pin 2 (== LED_BUILTIN) has no input)
   modeSelect.innerHTML = `
    <option selected value=1>Output</option>
    <option value=3><center>Pwm</center></option>
    <option value=4>Blink</option>
   `;
  } else {
   modeSelect.innerHTML = `
   <option value=0>Input</option>
   <option selected value=1>Output</option>
   <option value=2>Input_Pullup</option>
   <option value=3><center>Pwm</center></option>
   <option value=4>Blink</option>
   `;
  }
  modeSelect.style.textAlign = "center";
  modeCell.appendChild(modeSelect);

  const modeDisplay = document.createElement('span');
  modeDisplay.className = 'modeDisplay';
  modeCell.appendChild(modeDisplay);

  row.appendChild(modeCell);

// ----- slider

  const sliderCell = document.createElement('td');
  const sliderInput = document.createElement('input');
  sliderInput.type = 'range';
  sliderInput.min= '0';
  sliderInput.max = '255';
  sliderInput.value = '0';
  sliderInput.className = 'slider';
  sliderInput.onchange = function() { handleSliderChange(pin); };

  sliderCell.appendChild(sliderInput);
  row.appendChild(sliderCell);

// ----- input

  const valueCell = document.createElement('td');

  const valueInput = document.createElement('input');
  valueInput.type = 'number';
  valueInput.style.width = '50px';
  valueInput.className = 'valueInput';
  valueInput.onchange = function() { handleInputChange(pin); };
  valueCell.appendChild(valueInput);

  const valueDisplay = document.createElement('span');
  valueDisplay.className = 'value';
  valueCell.appendChild(valueDisplay);

  row.appendChild(valueCell);

// Append the row to the table body
  tableBody.appendChild(row);
 }
}

// Initialize WebSocket and create the GPIO table when the page loads
window.onload = function() {
 createGPIOTable();
 initWebSocket();
};

// ----------------- Some functions for pwm and blink demo

// handle pwm move

var to = ["up","up","up","up","up","up"];
var onthemove=false;

function move(){
 if (onthemove) {
  for (let i = 0; i <= 5 ; i++ ) {
   // if pwm mode
   if (document.querySelector('#row' + GPIOs[i] + ' .mode').value == 3) {
    var pos = document.querySelector('#row' + GPIOs[i] +' .slider').value * 1.0;
    if (pos == 0) { to[i] = "up"}
    if (pos == 255) { to[i] = "down"}
    if (to[i] == "up" ) { pos += 5 ; pos=Math.min(255,pos) } 
      else { pos -= 5 ; pos=Math.max(0,pos) }
    sendData("18", GPIOs[i], pos);
   }
  }
 }
}

setInterval(function() { move(); }, 1000);

// handle blink

function blink(){
 for (let i = 0; i <= 5 ; i++ ) {
  // if blink mode
  if (document.querySelector('#row' + GPIOs[i] + ' .mode').value == 4) {
   // if low make high
   if (document.querySelector('#row' + GPIOs[i] +' .slider').value == 0 ) {
    sendData( "17", GPIOs[i], 255);
   // else make low
   } else {
    sendData( "17", GPIOs[i], 0);
   }
  }
 }
}

setInterval(function() { blink(); }, 5000);

// handle A0

function toggleA0() {
 let button = document.getElementById("toggleA0button");
 console.log("button.innerHTML "+button.innerHTML)
 if ( button.innerHTML == "A0 Stopped") { sendData( "16", "A0", 200); }
 else { sendData( "16", "A0", 0); }
}

// ------------------------------ And finally the web page
</script>
</head>
<body>
<center>
 <h1>Wemos GPIO Example</h1>
 <table style="background-color:lightgrey;" border="1">
 <thead>
  <tr><th>GPIO</th>
  <th>Label</th>
  <th>Mode</th>
  <th>Pwm<br>&#8592; Low &#9830; High &#8594;</th>
  <th>Value</th></tr>
 </thead>
 <tbody id="gpioTableBody">
  <!-- Table rows for each GPIO pin will be added here -->
 </tbody>
 </table>
<br><button title="Put some Modes on Pwm and click me" 
     onclick="onthemove = ! onthemove">Toggle Pwm AutoMove</button>
<h2>Analog Input (A0)<br>
digital: <span id="A0reading">0</span><br>
value: <span id="A0voltage">0</span> volt</h2>
<br><button id="toggleA0button" 
    title="Click to toggle A0 reading"
    onclick="toggleA0()">A0 Stopped</button>
</center>
</body>
</html>
)rawliteral";

// ----------------------------------------------------------------------------