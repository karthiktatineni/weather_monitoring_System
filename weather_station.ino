#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

#define DHTPIN 2       // DHT11 data pin
#define DHTTYPE DHT11  // DHT sensor type
DHT dht(DHTPIN, DHTTYPE);

#define MQ7PIN A0      // MQ-7 air quality sensor analog pin

// Wi-Fi Credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Create Web Server on port 80
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Define web server route
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  // Print headers for the Serial Plotter
  Serial.println("Temperature (°C) \t Humidity (%) \t Air Quality (Raw Value)");
}

void loop() {
  server.handleClient();  // Handle web server requests

  // Read sensor data
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int airQuality = analogRead(MQ7PIN);

  // Check for valid sensor readings
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Sensor error. Check connections!");
  } else {
    // Print data for Serial Plotter
    Serial.print(temperature);
    Serial.print("\t");
    Serial.print(humidity);
    Serial.print("\t");
    Serial.println(airQuality);
  }

  delay(1000);  // Update every second
}

void handleRoot() {
  // Read temperature and humidity from DHT11
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int mq7Value = analogRead(MQ7PIN);

  // Handle NaN readings
  String tempDisplay = isnan(temp) ? "N/A" : String(temp);
  String humDisplay = isnan(hum) ? "N/A" : String(hum);

  // Determine air quality
  String airQuality;
  if (mq7Value < 300) {
    airQuality = "Good";
  } else if (mq7Value < 600) {
    airQuality = "Moderate";
  } else {
    airQuality = "Poor";
  }

  // HTML page content
  String html = R"=====( 
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Environment Monitor</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        text-align: center;
        background-color: #f4f4f9;
        margin: 0;
        padding: 0;
      }
      .container {
        margin: 50px auto;
        padding: 20px;
        width: 80%;
        max-width: 600px;
        background: #ffffff;
        box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
        border-radius: 10px;
      }
      h1 {
        color: #333333;
      }
      .data {
        margin: 20px 0;
        font-size: 1.2em;
      }
      .value {
        font-size: 2em;
        color: #4caf50;
      }
    </style>
    <meta http-equiv="refresh" content="5"> <!-- Auto-refresh every 5 seconds -->
  </head>
  <body>
    <div class="container">
      <h1>Environment Monitor</h1>
      <div class="data">
        <p>Temperature: <span class="value">__TEMP__ °C</span></p>
        <p>Humidity: <span class="value">__HUM__ %</span></p>
        <p>Air Quality: <span class="value">__AIR_QUALITY__</span></p>
      </div>
    </div>
  </body>
  </html>
  )=====";

  // Replace placeholders with actual values
  html.replace("__TEMP__", tempDisplay);
  html.replace("__HUM__", humDisplay);
  html.replace("__AIR_QUALITY__", airQuality);

  // Send HTML response
  server.send(200, "text/html", html);
}
