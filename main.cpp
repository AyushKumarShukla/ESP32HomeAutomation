#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <DHT.h>

#define ON 1
#define OFF 0
#define RGBON 0
#define RGBOFF 1
#define GasSensor 32
#define Buzzer 23
DHT dht(33, DHT11);

String WebPageStart = R"(
<html>
<head>
  <title>GPIO CONTROL SERVER</title>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <style>
    /* General styles */
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
    }
    
    .container {
      width: 100%;
      max-width: 800px; /* Adjust max-width as needed */
      margin: 0 auto;
      padding: 20px;
    }

    .switch-board {
      width: 50%;
      margin: 20px auto;
      text-align: center;
    }

    .switch {
      display: block;
      width:70%;
      margin:15px auto;
      padding: 10px;
      text-align: center;
      color: #fcce03;
      background-color: #3d3c39;
      border-radius: 10px;
      text-decoration: none;
    }

    h2 {
      font-size: 25px;
      margin-top: 20px;
    }

    .slider {
      width: 100%;
      height: 15px;
      border-radius: 5px;
      background: #d3d3d3;
      -webkit-appearance: none;
      appearance: none;
      outline: none;
      margin-top: 10px;
    }

    .slider::-webkit-slider-thumb {
      width: 25px;
      height: 25px;
      border-radius: 50%;
      background: #4CAF50;
      -webkit-appearance: none;
      appearance: none;
      cursor: pointer;
    }

    /* Media queries for responsiveness */
    @media screen and (max-width: 600px) {
      .container {
        padding: 10px;
      }

      h2 {
        font-size: 20px;
      }

      .slider {
        margin-top: 5px;
      }
    }
  </style>
</head>
<body>
  <center><h2>GPIO CONTROL SERVER</h2></center>
  <div class='switch-board'>
    <p>Blue Intensity:</p>
    <input type="range" min="0" max="255" value="0" class="slider" id="blueSlider">
    <p>Green Intensity:</p>
    <input type="range" min="0" max="255" value="0" class="slider" id="greenSlider">
    <p>Red Intensity:</p>
    <input type="range" min="0" max="255" value="0" class="slider" id="redSlider">
    <h2>Temperature and Humidity</h2>
    <p>Temperature: <span id="temperature">N/A</span> &deg;C</p>
    <p>Humidity: <span id="humidity">N/A</span> %</p>
  </div>
)";

String WebPageEnd = R"(
  </div>
  <script>
    var blueSlider = document.getElementById("blueSlider");
    var greenSlider = document.getElementById("greenSlider");
    var redSlider = document.getElementById("redSlider");

    function updateColor() {
      var blueValue = blueSlider.value;
      var greenValue = greenSlider.value;
      var redValue = redSlider.value;
      var url = "/color?blue=" + blueValue + "&green=" + greenValue + "&red=" + redValue;
      fetch(url);
    }

    blueSlider.oninput = updateColor;
    greenSlider.oninput = updateColor;
    redSlider.oninput = updateColor;

    // WebSocket client setup
    var ws = new WebSocket('ws://' + window.location.hostname + ':81/');
    ws.onopen = function() {
      console.log('WebSocket client connected');
    };

    ws.onmessage = function(event) {
      // Handle messages received from the WebSocket server
      console.log('WebSocket message received: ' + event.data);
      // You can update the webpage or display an alert based on the message received
      alert(event.data);
    };

    // Function to update temperature and humidity
    function updateTempHumidity() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          document.getElementById('temperature').innerText = data.temperature;
          document.getElementById('humidity').innerText = data.humidity;
        })
        .catch(error => console.error('Error fetching data:', error));
    }

    // Periodically update temperature and humidity every 5 seconds
    setInterval(updateTempHumidity, 5000);
  </script>
 </body>
</html>
)";

const char* ssid = "MyAP";
const char* pass = "ESP-1234";

WebSocketsServer webSocket = WebSocketsServer(81); // WebSocket server port

WiFiServer server(80);

int StateLED1 = OFF;
int StateLED2 = OFF;
int LEDpin1 = 16; // GPIO 16 is LED 1
int LEDpin2 = 17; // GPIO 17 is LED 2 
// pins for controlling RGB led colors
int BLUEPIN = 27;
int GREENPIN = 25;
int REDPIN = 26;

int Motor = 19;

int STATEBLUE = OFF;
int STATEGREEN = OFF;
int STATERED = OFF;
int STATEMOTOR = OFF;

//store data from HTTP request header coming from the client
String header;

// for timing of request given by the client
unsigned long present = millis();
unsigned long past = 0;
unsigned long timeout = 2000;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  // Handle WebSocket events
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("[%u] WebSocket client connected\n", num);
      break;
    case WStype_TEXT:
      // Handle text messages received from WebSocket clients
      Serial.printf("[%u] Received text: %s\n", num, payload);
      // Process the message as needed
      break;
    case WStype_DISCONNECTED:
      Serial.printf("[%u] WebSocket client disconnected\n", num);
      break;
    // Other WebSocket event types can be handled here
  }
}

void setup() {
  Serial.begin(57600);
  dht.begin();
  delay(2000);
  pinMode(LEDpin1, OUTPUT);
  digitalWrite(LEDpin1, LOW);
  pinMode(LEDpin2, OUTPUT);
  digitalWrite(LEDpin2, LOW);

  pinMode(BLUEPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(REDPIN, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(Motor, OUTPUT);

  digitalWrite(BLUEPIN, HIGH);
  digitalWrite(GREENPIN, HIGH);
  digitalWrite(REDPIN, HIGH);

  WiFi.softAP(ssid, pass);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  bool GAS_ALERT;
  int Gas_Value = analogRead(GasSensor);
  Gas_Value = map(Gas_Value, 0, 4095, 0, 100);
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();
  Serial.print("Gas Level:");
  Serial.print(Gas_Value);
  Serial.print("\n");
  Serial.print("Temperature:");
  Serial.print(temp);
  Serial.print("\n");

  Serial.print("Humidity:");
  Serial.print(humid);
  Serial.print("\n");
  if (Gas_Value > 28) {
    digitalWrite(Buzzer, HIGH);
    GAS_ALERT = true;
    webSocket.broadcastTXT("Gas alert triggered!");
    while (Gas_Value > 28) {
      Gas_Value = analogRead(GasSensor);
      Gas_Value = map(Gas_Value, 0, 4095, 0, 100);
      sleep(2);
    }
  } else {
    digitalWrite(Buzzer, LOW);
    GAS_ALERT = false;
  }

  WiFiClient client = server.available();
  if (client) {
    present = millis();
    past = present;
    String currentLine = "";

    while (client.connected() && present - past < timeout) {
      present = millis();
      //when client sends a request
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // respond OK to client
            if (header.indexOf("GET /data") >= 0) {
              // Serve temperature and humidity as JSON
              String jsonData = "{";
              jsonData += "\"temperature\": " + String(temp, 1) + ",";
              jsonData += "\"humidity\": " + String(humid, 1);
              jsonData += "}";
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type: application/json");
              client.println("Connection: close");
              client.println();
              client.print(jsonData);
            } else {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("connection:close");
              client.println();
              //display gas alert webpage
              if (header.indexOf("GET /1/ON") >= 0) {
                StateLED1 = ON;
                digitalWrite(LEDpin1, HIGH);
              } else if (header.indexOf("GET /1/OFF") >= 0) {
                StateLED1 = OFF;
                digitalWrite(LEDpin1, LOW);
              }

              if (header.indexOf("GET /2/ON") >= 0) {
                StateLED2 = ON;
                digitalWrite(LEDpin2, HIGH);
              } else if (header.indexOf("GET /2/OFF") >= 0) {
                StateLED2 = OFF;
                digitalWrite(LEDpin2, LOW);
              }

              if (header.indexOf("GET /MOTOR/ON") >= 0) {
                STATEMOTOR = ON;
                digitalWrite(Motor, HIGH);
              } else if (header.indexOf("GET /MOTOR/OFF") >= 0) {
                STATEMOTOR = OFF;
                digitalWrite(Motor, LOW);
              }
              if (header.indexOf("GET /color") >= 0) {
                int blueValue = header.indexOf("blue=");
                int greenValue = header.indexOf("&green=");
                int redValue = header.indexOf("&red=");
                int blueInt = header.substring(blueValue + 5, greenValue).toInt();
                int greenInt = header.substring(greenValue + 7, redValue).toInt();
                int redInt = header.substring(redValue + 5).toInt();

                analogWrite(BLUEPIN, blueInt);
                analogWrite(GREENPIN, greenInt);
                analogWrite(REDPIN, redInt);
              }

              client.print(WebPageStart);
              client.print("<div class='switch-board'>");
              if (StateLED1) {
                client.println("<a href=\"/1/OFF\" class='switch'>TURN LED 1 OFF</a>");
              } else {
                client.println("<a href=\"/1/ON\" class='switch'>TURN LED 1 ON</a>");
              }
              if (StateLED2) {
                client.println("<a href=\"/2/OFF\" class='switch'>TURN LED 2 OFF</a>");
              } else {
                client.println("<a href=\"/2/ON\" class='switch'>TURN LED 2 ON</a>");
              }
              if (STATEMOTOR == 1) {
                client.println("<a href=\"/MOTOR/OFF\" class='switch'>TURN MOTOR OFF</a>");
              } else {
                client.println("<a href=\"/MOTOR/ON\" class='switch'>TURN MOTOR ON</a>");
              }

              client.print("</div>");

              if (GAS_ALERT) {
                client.println("<html><body><h2>SMOKE/GAS ALERT !!!!</h2></body></html>");
              }
              
              client.print(WebPageEnd);
              client.println();
            }
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
  }
  webSocket.loop(); // Handle WebSocket events
}
