#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Replace with your network credentials
const char* ssid = "C0nVeRgE_NeT";
const char* password = "h1ND1K0@L@m";

#define DHTPIN 4     // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT22     // DHT 22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);

// current temperature, humidity, and heat index, updated in loop()
float t = 0.0;
float h = 0.0;
float n = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 5 seconds
const long interval = 5000;  

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    .sensor-box {
      display: inline-block;
      width: 90%;
      margin: 0 auto;
    }
    .sensor-box p {
      padding: 10px 0;
      border-bottom: 1px solid #ddd;
    }
  </style>
</head>
<body>
  <h2>Asynchronous Server</h2>
  <div class="sensor-box">
    <p>
      <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
      <span class="dht-labels">Temperature</span> 
      <span id="temperature">%TEMPERATURE%</span>
      <sup class="units">&deg;C</sup>
    </p>
    <p>
      <i class="fas fa-tint" style="color:#00add6;"></i> 
      <span class="dht-labels">Humidity</span>
      <span id="humidity">%HUMIDITY%</span>
      <sup class="units">&percnt;</sup>
    </p>
    <p>
      <i class="fas fa-thermometer-three-quarters" style="color:#ff5733;"></i> 
      <span class="dht-labels">Heat Index</span> 
      <span id="heatindex">%HEATINDEX%</span>
      <sup class="units">&deg;C</sup>
    </p>
  </div>
</body>
<script>
// Refresh temperature every 5 seconds
setInterval(function() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 5000);

// Refresh humidity every 5 seconds
setInterval(function() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 5000);

// Refresh heat index every 5 seconds
setInterval(function() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("heatindex").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/heatindex", true);
  xhttp.send();
}, 5000);
</script>
</html>)rawliteral";


String processor(const String& var){
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  else if(var == "HEATINDEX"){  // Check for the heat index placeholder
    return String(n);    // Ensure heatIndex variable is returned
  }
  return String();
}


void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  dht.begin();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });
  server.on("/heatindex", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(n).c_str());
  });

  // Start server
  server.begin();
}
 
void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Save the last time you updated the DHT values
    previousMillis = currentMillis;
    
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      t = newT;
      Serial.println(t);
    }

    // Read humidity
    float newH = dht.readHumidity();
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      h = newH;
      Serial.println(h);
    }

    // Calculate Heat Index
    float newHI = dht.computeHeatIndex(t, h, false);
    if (!isnan(t) && !isnan(h)) {
      n = newHI;
      Serial.println(n);
    }
  }
}
