/* ----------------------------------------------------------------------------

HTMLClientExampleDataStore.h

  An example created by wsServerConfigureWebSocketServer when it does not exist.

---------------------------------------------------------------------------- */ 

const char HTMLClientExampleDataStore[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
<head>
<title>Wemos DataStore Example</title>
</head>
<body>
<center>
<h1>Wemos DataStore Example</h1>
<table style="background-color:lightgrey;" border="1">
<thead>
<tr><th>Variable</th><th>Value</th><th>Select</th></tr>
</thead>
<tbody>
<tr>
<td>Name: <input id="name"></td>
<td>Value: <input id="value"><button onclick="saveInput()">Save</button></td>
<td>
 Choose a name:
 <select name="names" id="names" onChange="getValue()">
 </select>
 <button onclick="WipeData()">Wipe</button>
</td>
</tr>
</tbody>
</table>
<h3>Open another browser window or...</h3>
<h3>combine me with wsClientDataStore.ino</h3>
</center>

<script>

// ---------------------------------- WebSocket Connection

var socket

// Function to initialize the WebSocket
function initWebSocket() {
 // Connect to the WebSocket server
 socket = new WebSocket('ws://' + window.location.hostname + '/wsServer/ws')

 // Handle received messages
 socket.onmessage = function(event) {handleMessage(event.data)}

 // Handle WebSocket close event
 socket.onclose = function(event) {
  console.log("Attempting to reconnect in 1 second...")
  setTimeout(initWebSocket, 1000)
 }

 socket.onerror = function(error) {
  console.error("WebSocket error:", error)
 }

 // Request initial data from the server
 setTimeout(function() {connect()}, 500)
}

function connect() {socket.send("20")} // List keys

// Initialize WebSocket when the page loads
window.onload = function() {initWebSocket()}

// ---------------------------------------- Send To Server

var myupdate = false;

function saveInput() {
 const name = document.getElementById("name").value
 const value = document.getElementById("value").value
 DataStore('21', name, value)
 myupdate = true;
}

function getValue() {
 name = document.getElementById("names").value
 console.log("Get value for>"+name+"<")
 if (name !="") { DataStore('22', name) }
}

function WipeData() {
 name = document.getElementById("names").value
 if (name !="") { DataStore('23', name) }
}

function DataStore(type, name, value) {
 console.log("---------------------")
 console.log("Send: "+type+"||"+name+"||"+value)
 socket.send(type+"||"+name+"||"+value)
}

// ------------------------- WebSocket Receive From Server

function handleMessage(data) {
 console.log("Received: "+data)
 let type = data.substring(0,2)
 if ( (type > 19) && ( type < 30 )) { // 10..19 reserved for DataStore
  data = data.substring(4)
  let name = data.substring(0,data.indexOf("||"))
  let value = data.substring(data.indexOf("||")+2)

  if (type == 29) {  // 10 is error, DataStore may be full or data not found
   alert(data)
  }

  if ( (type == 21) && (value != "" ) ) { 
  // Someone saved a new variable or updated an existing variable
   if ( (document.getElementById("name").value == name) || 
        (document.getElementById("name").value == "") ) {
   // update value when same name is in variable name input field.
    document.getElementById("name").value = name
    document.getElementById("value").value = value
   }
   editOptions("add",name) // optionally add name to select box
   if (myupdate) { 
    document.getElementById("names").value = name
    myupdate = false
   }
  }

  if (type == 22) { // We found a value for the selected name 
   document.getElementById("name").value = name
   document.getElementById("value").value = value
  }

  if (type == 23) {  // name may be deleted by someone else
   document.getElementById("value").value = ""
   editOptions("del",name)
   getValue()
  }

  if (type == 20) {  // there may be a list with names in DataStore
   let entries = 0;
   while (data != "") {
    entries = entries + 1;
    index = data.indexOf("||")
    if (index > -1 ) { name=data.substring(0,index) }
    else { name=data }
    data = data.substring(name.length+2)
    editOptions("add",name) 
    if (entries == 1 ) { DataStore('22', name) }
   }
  }
 }
}

// --------------------------- Add / Delete Select Options

function editOptions(action,item) {

 let foundindex = -1
 let newindex = 0
 for (i = 0; i < document.getElementById("names").length; ++i){
  if (item == document.getElementById("names").options[i].value){foundindex = i}
  if (item  > document.getElementById("names").options[i].value){newindex = i+1}
 }
 if ((action == "add") && (foundindex == -1)) {
  var x = document.getElementById("names")
  var option = document.createElement("option")
  option.text = item
  option.value = item
  x.add(option, x[newindex])
 }
 if ((action == "del") && (foundindex != -1)) {
  var x = document.getElementById("names")
  x.remove(foundindex)
 }
}
</script>
</body>
</html>
)rawliteral";

// ----------------------------------------------------------------------------