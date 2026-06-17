#include <ESP8266WiFi.h>
#include <SPI.h>
#include <LoRa.h>

// ---------------- LoRa Pin Config for ESP8266 ----------------
#define LORA_SS    D8   // GPIO15
#define LORA_RST   D0   // GPIO16
#define LORA_DIO0  D2   // GPIO4

// Node IDs
byte MasterNode = 0xFF;
byte Node1 = 0xBB;

// --------- WiFi Credentials ---------
const char* ssid     = "realme X";
const char* password = "sart@2004";

// --------- LoRa Data Variables ---------
float soilPercent = 0;
float temperature = 0;
float waterPercent  = 0;  // Smoothed percentage
int motorStatus   = 0;
bool dataReceived = false;  

// --------- Moving Average Setup ---------
#define WATER_BUFFER_SIZE 5
float waterBuffer[WATER_BUFFER_SIZE];
int bufferIndex = 0;

// --------- Web Server ---------
WiFiServer server(80);

// Function to compute average of water buffer
float averageWater() {
  float sum = 0;
  for(int i=0; i<WATER_BUFFER_SIZE; i++) sum += waterBuffer[i];
  return sum / WATER_BUFFER_SIZE;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Receiver with Live Web Server");

  // ----- WiFi Setup -----
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());

  server.begin();

  // ----- LoRa Setup -----
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed! Check wiring.");
    while (1);
  }
  Serial.println("LoRa init successful!");

  // Initialize water buffer
  for(int i=0; i<WATER_BUFFER_SIZE; i++) waterBuffer[i] = 0;
}

void loop() {
  // ----- Receive LoRa Data -----
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    byte recipient = LoRa.read();
    byte sender    = LoRa.read();
    byte msgCount  = LoRa.read();
    byte length    = LoRa.read();

    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    if (recipient != MasterNode && recipient != 0xFF) return;

    Serial.println("LoRa Packet: " + incoming);

    // Parse values
    float values[5];
    int i = 0;
    char incomingArray[60];
    incoming.toCharArray(incomingArray, 60);
    char *token = strtok(incomingArray, ",");
    while (token != NULL && i < 5) {
      values[i++] = atof(token);
      token = strtok(NULL, ",");
    }

    if (i == 5) {
      soilPercent = values[1];
      temperature = values[2];
      float newWater = values[3];  // Raw percentage from node
      motorStatus = (int)values[4];
      dataReceived = true;

      // Update moving average buffer
      waterBuffer[bufferIndex] = newWater;
      bufferIndex = (bufferIndex + 1) % WATER_BUFFER_SIZE;
      waterPercent = averageWater();
    }
  }

  // ----- Serve Web Page -----
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("GET /data") != -1) {
      // Serve JSON data for AJAX
      String json = "{";
      json += "\"soil\":" + String(soilPercent) + ",";
      json += "\"temp\":" + String(temperature) + ",";
      json += "\"water\":" + String(waterPercent) + ",";
      json += "\"motor\":" + String(motorStatus);
      json += "}";
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.println(json);
    } else {
      // Serve main HTML page
      String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
      html += "<title>Smart Field Monitor</title>";
      html += "<style>";
      html += "body { font-family: Arial; text-align: center; background: #f0f8ff; }";
      html += "h1 { color: #003366; }";
      html += ".card { display:inline-block; background:#fff; padding:20px; margin:10px; ";
      html += "border-radius:15px; box-shadow:0px 4px 8px rgba(0,0,0,0.2); min-width:200px; }";
      html += "</style>";
      html += "<script>";
      html += "function updateData() {";
      html += "fetch('/data').then(response => response.json()).then(data => {";
      html += "document.getElementById('soil').innerText = data.soil + ' %';";
      html += "document.getElementById('temp').innerText = data.temp + ' °C';";
      html += "document.getElementById('water').innerText = data.water + ' %';";
      html += "document.getElementById('motor').innerText = data.motor == 1 ? 'ON' : 'OFF';";
      html += "document.getElementById('motor').style.color = data.motor == 1 ? 'green' : 'red';";
      html += "}); setTimeout(updateData, 1000); }"; // update every 1 sec
      html += "window.onload = updateData;";
      html += "</script></head><body>";
      html += "<h1>🌾 Smart Field Monitoring</h1>";
      html += "<div class='card'><h2>🌱 Soil Moisture</h2><p id='soil'>--</p></div>";
      html += "<div class='card'><h2>🌡️ Temperature</h2><p id='temp'>--</p></div>";
      html += "<div class='card'><h2>💧 Water Level</h2><p id='water'>--</p></div>";
      html += "<div class='card'><h2>⚡ Motor</h2><p id='motor' style='color:red'>--</p></div>";
      html += "</body></html>";

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();
      client.println(html);
    }
  }
}