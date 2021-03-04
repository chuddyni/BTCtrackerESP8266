#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// WiFi settings
const char* ssid     = "ssid";
const char* password = "pass";

// API server
const char* host = "api.coindesk.com";
float lastValue = 0;
void setup() {

  // Serial
  Serial.begin(115200);
  delay(10);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Loguje sie do wifi");
  display.display();
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname("ESP-host");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  display.clearDisplay();
  display.println("Polaczono z wifi");
}

void loop() {

  // Connect to API
  //  Serial.print("connecting to ");
  //  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/v1/bpi/currentprice/PLN.json";

  //  Serial.print("Requesting URL: ");
  //  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(100);

  // Read all the lines of the reply from server and print them to Serial
  String answer;
  while (client.available()) {
    String line = client.readStringUntil('\r');
    answer += line;
  }

  client.stop();
  //  Serial.println();
  //  Serial.println("closing connection");

  // Process answer
  //  Serial.println();
  //  Serial.println("Answer: ");
  //  Serial.println(answer);

  // Convert to JSON
  String jsonAnswer;
  int jsonIndex;

  for (int i = 0; i < answer.length(); i++) {
    if (answer[i] == '{') {
      jsonIndex = i;
      break;
    }
  }

  // Get JSON data
  jsonAnswer = answer.substring(jsonIndex);
  //  Serial.println("--------------------");
  //  Serial.println("JSON answer: ");
  //  Serial.println(jsonAnswer);
  //  //  jsonAnswer.trim();
  //  Serial.println("--------------------");


  // Stream& input;

  StaticJsonDocument<768> doc;
  deserializeJson(doc, jsonAnswer);

  JsonObject time = doc["time"];
  const char* time_updated = time["updated"]; // "Feb 4, 2021 16:07:00 UTC"
  const char* time_updatedISO = time["updatedISO"]; // "2021-02-04T16:07:00+00:00"
  const char* time_updateduk = time["updateduk"]; // "Feb 4, 2021 at 16:07 GMT"

  const char* disclaimer = doc["disclaimer"]; // "This data was produced from the CoinDesk Bitcoin Price ...

  JsonObject bpi_USD = doc["bpi"]["USD"];
  const char* bpi_USD_code = bpi_USD["code"]; // "USD"
  const char* bpi_USD_rate = bpi_USD["rate"]; // "36,588.9517"
  const char* bpi_USD_description = bpi_USD["description"]; // "United States Dollar"
  float bpi_USD_rate_float = bpi_USD["rate_float"]; // 36588.9517

  JsonObject bpi_PLN = doc["bpi"]["PLN"];
  const char* bpi_PLN_code = bpi_PLN["code"]; // "PLN"
  const char* bpi_PLN_rate = bpi_PLN["rate"]; // "137,518.4003"
  const char* bpi_PLN_description = bpi_PLN["description"]; // "Polish Zloty"
  float bpi_PLN_rate_float = bpi_PLN["rate_float"]; // 137518.4003

  if (bpi_PLN_rate_float > 0) {
    lastValue = bpi_PLN_rate_float;
    Serial.println(lastValue);
    display.clearDisplay();
      display.setCursor(0, 10);

    display.println("Kurs BTC: ");
    display.println(lastValue);
    display.display();
  }


  // Wait 5 seconds
  delay(5000);
}
