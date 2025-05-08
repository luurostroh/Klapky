#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

char idx[] PROGMEM = R"=====(

"<!doctype html>

<html>

<head>
  <meta charset="utf-8">

  <title>Blank template</title>

  <!-- Load external CSS styles -->
  <link rel="stylesheet" href="style.css">

</head>
<script>
var gateway = 'ws://' + window.location.hostname + ':81/';
var Socket;
window.addEventListener('load', onLoad);

function init() {
  console.log('Trying to open a WebSocket connection...');
  Socket = new WebSocket(((window.location.protocol === "https:") ? "wss://" : "ws://") + window.location.host + "/ws");
  //Socket = new WebSocket(gateway);
  //Socket.onmessage =  function onOpen(event) {console.log('Connection opened');}
  Socket.onclose =  function onClose(event) {console.log('Connection closed');}
  Socket.onmessage = function (event) {
  if(event.data[0]=='i'){ 
      let _id = event.data.substring(0,2);
      if(event.data[2] == '1'){
          document.getElementById(_id).style.backgroundColor = 'lightgreen';
      }
      else{
          document.getElementById(_id).style.backgroundColor = 'green';
      }
  }
  if(event.data[0]=='o'){ 
      let _id = event.data.substring(0,2);
       if(event.data[2] == '1'){
          document.getElementById(_id).style.backgroundColor = 'yellow';
      }
      else{
          document.getElementById(_id).style.backgroundColor = 'darkgoldenrod';
      }
  }
    
}
}

function onLoad(event) {
  init();
  //Socket.send("start");
}



function Click(element) {
  var state;
  if (element.className == 'btn2') {
      if (element.style.backgroundColor == 'lightgreen') element.style.backgroundColor = 'green';
      else element.style.backgroundColor = 'lightgreen';
  }
  else {
      if (element.style.backgroundColor == 'yellow'){state = "0"; element.style.backgroundColor = 'darkgoldenrod';}
      else {state = "1"; element.style.backgroundColor = 'yellow';}
  }
  var msg = "O";
 
  msg += element.id;
  msg += state;
  Socket.send(msg);
}

</script>

<style>
body {
  background-color: hwb(246 21% 49% / 0.452);
}

h1 {
  color: #09262b;
  text-align: center;
}

p {
  font-family: verdana;
  font-size: 20px;
}

.btn2 {
  height: 30px;
  width: 30px;
  background-color:green;
}

.btn1 {
  height: 30px;
  width: 30px;
  background-color:darkgoldenrod;
}

table {
  vertical-align: middle;
}

tr,td {
  padding: 10px;
}
</style>

<body>
  <table>
      <tr >
          <td><button id="o0" class="btn1" onclick="Click(this)">1</button></td>
          <td><button id="o1" class="btn1" onclick="Click(this)">2</button></td>
          <td><button id="o2" class="btn1" onclick="Click(this)">3</button></td>
          <td><button id="o3" class="btn1" onclick="Click(this)">4</button></td>
          <td><button id="o4" class="btn1" onclick="Click(this)">5</button></td>
          <td><button id="o5" class="btn1" onclick="Click(this)">6</button></td>
          <td><button id="o6" class="btn1" onclick="Click(this)">7</button></td>
          <td><button id="o7" class="btn1" onclick="Click(this)">8</button></td>
      </tr>
      <tr style="height:100px">
      </tr>
      <tr>
          <td><button id="i0" class="btn2" onclick="Click(this)">1</button></td>
          <td><button id="i1" class="btn2" onclick="Click(this)">2</button></td>
          <td><button id="i2" class="btn2" onclick="Click(this)">3</button></td>
          <td><button id="i3" class="btn2" onclick="Click(this)">4</button></td>
          <td><button id="i4" class="btn2" onclick="Click(this)">5</button></td>
          <td><button id="i5" class="btn2" onclick="Click(this)">6</button></td>
          <td><button id="i6" class="btn2" onclick="Click(this)">7</button></td>
          <td><button id="i7" class="btn2" onclick="Click(this)">8</button></td>
      </tr>
  </table>

  <!-- Load external JavaScript -->
  <script src="scripts.js"></script>

</body>

</html>"
)=====";
// Replace with your network credentials
const char* ssid     = "ESP32-Access-Point";
const char* password = "heslo12345";
// WiFiServer server(80);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
String header;
int ADC_Max = 4096;     // This is the default ADC max value on the ESP32 (12 bit ADC width);
                        // this width can be set (in low-level oode) from 9-12 bits, for a
                        // a range of max values of 512-4096
// create four servo objects 
Servo servo[3];



// Published values for SG90 servos; adjust if needed
int minUs = 1000;
int maxUs = 2000;

// These are all GPIO pins on the ESP32
// Recommended pins include 2,4,12-19,21-23,25-27,32-33
// for the ESP32-S2 the GPIO pins are 1-21,26,33-42
// for the ESP32-S3 the GPIO pins are 1-21,35-45,47-48
// for the ESP32-C3 the GPIO pins are 1-10,18-21
int servoPin[3] = {16,17,18};
int potPin[3]   = {33,32,35};

// ESP32PWM pwm;
int val;    // variable to read the value from the analog pin
int tmp_val[3] = {0};
// put function declarations here:

void setupWebServer() {

  // Serve the root page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", idx);
    // printf("start page\n");
  });

  // Start the server
  server.begin();
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
 {
  String debug_data = "";
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  String payload = "";
  char msg[5];
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
  {
    data[len] = 0;
    String rec_data = "";
    payload = (char *)data;
   
    debug_data += "ok\n";
    debug_data += payload;
    Serial.println(debug_data);

    if (payload[0] == 'O') 
    {
      int _idx = payload[2] - 0x30;
      // Serial.print("index");Serial.println(_idx);
        if(payload[3] == '1')
        {
          // Serial.println("on"); 
          // i2c_outputs.data_arr[0] |= 1<<_idx;
          // outs[_idx].setBackground(TFT_YELLOW,TFT_BLACK);
        }
        else
        {
          // Serial.println("off");  
          // i2c_outputs.data_arr[0] &= ~(1<<_idx);
          // outs[_idx].setBackground(TFT_DARK_YELLOW,TFT_BLACK);
        }

    }
  }

 }


void DataToPage()
{
    char msg[4];
    for(int i = 0;i<8;i++)
    {
      // if(toggle[i]){sprintf(msg,"o%u1",i);ws.textAll(msg);}
      // if(i2c_inputs.data_arr[0] & 1<<i){sprintf(msg,"i%u1",i);ws.textAll(msg);}
    }

}


 bool client_connected;
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) 
{
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      client_connected = true;
      // DataToPage();
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      client_connected = false;
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}


void setup() {

	Serial.begin(115200);
  servo[0].attach(16, minUs, maxUs);
  servo[1].attach(17, minUs, maxUs);
  servo[2].attach(18, minUs, maxUs);
    // Connect to Wi-Fi network with SSID and password
    Serial.print("Setting AP (Access Point)…");
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP(ssid, password);
  
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    
    setupWebServer();
    initWebSocket();
    ws.cleanupClients(); 

}

void loop()
{

    for (int i = 0; i < 3; i++) 
    {
        val = analogRead(potPin[i]); // read the value of the potentiometer (value between 0 and 1023)
        val = map(val, 0, ADC_Max, 0, 360) / 4; // scale it to use it with the servo (value between 0 and 180)
        if (tmp_val[i] != val) 
        {
            tmp_val[i] = val;
            //  Serial.print(val);Serial.print("   ");
            Serial.print(i);
            Serial.print(" -- ");

            Serial.println(val);
            servo[i].write(val);
        }
        // set the servo position according to the scaled value
    }

    delay(200);
}



