#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// Wi-Fi 設定 / Wi-Fi setup
const char* ssid = "ESP-RGB";
const char* password = "12345678";

ESP8266WebServer server(80);

// 腳位設定 / Pin definitions
const int Red = 12;
const int Green = 13;
const int Blue = 14;
const int DHTPIN = 5;
const int irLedPin = 2;
const int irRxPin = 4;
const int buttonPin = 0;
const int buzzerPin = 15;

DHT dht(DHTPIN, DHT11);
bool rainbowActive = false;
bool buzzerTrigger = false;
bool buzzerLog = false;
#define FADESPEED 5

void setup() {
  Serial.begin(115200);
  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Blue, OUTPUT);
  pinMode(irRxPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  dht.begin();
  tone(irLedPin, 38000);

  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP8266</title></head><body><h1>ESP8266 Sensor Monitor</h1></body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/data", []() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int ldr = analogRead(A0);
    int ir = digitalRead(irRxPin) == LOW ? 1 : 0;
    int buzzer = buzzerLog ? 1 : 0;
    buzzerLog = false;
    String json = "{";
    json += "\"temp\":" + String(t, 1) + ",";
    json += "\"humidity\":" + String(h, 1) + ",";
    json += "\"ldr\":" + String(ldr) + ",";
    json += "\"ir\":" + String(ir) + ",";
    json += "\"buzzer\":" + String(buzzer);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/buzzer", []() {
    buzzerTrigger = true;
    buzzerLog = true;
    server.send(200, "text/plain", "OK");
  });

  server.on("/start", []() {
    rainbowActive = true;
    server.send(200, "text/html", "<h2>🌈 Rainbow started</h2><a href='/'>Back</a>");
  });

  server.on("/stop", []() {
    rainbowActive = false;
    analogWrite(Red, 0);
    analogWrite(Green, 0);
    analogWrite(Blue, 0);
    server.send(200, "text/html", "<h2>🛑 Rainbow stopped</h2><a href='/'>Back</a>");
  });

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();

  if (buzzerTrigger) {
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
    buzzerTrigger = false;
  }

  static bool lastBtn = HIGH;
  bool currentBtn = digitalRead(buttonPin);
  if (lastBtn == HIGH && currentBtn == LOW) {
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
    buzzerLog = true;
  }
  lastBtn = currentBtn;

  if (rainbowActive) {
    int r, g, b;
    for (r = 0; r < 256 && rainbowActive; r++) {
      analogWrite(Red, r); delay(FADESPEED); server.handleClient();
    }
    for (b = 255; b > 0 && rainbowActive; b--) {
      analogWrite(Blue, b); delay(FADESPEED); server.handleClient();
    }
    for (g = 0; g < 256 && rainbowActive; g++) {
      analogWrite(Green, g); delay(FADESPEED); server.handleClient();
    }
    for (r = 255; r > 0 && rainbowActive; r--) {
      analogWrite(Red, r); delay(FADESPEED); server.handleClient();
    }
    for (b = 0; b < 256 && rainbowActive; b++) {
      analogWrite(Blue, b); delay(FADESPEED); server.handleClient();
    }
    for (g = 255; g > 0 && rainbowActive; g--) {
      analogWrite(Green, g); delay(FADESPEED); server.handleClient();
    }
  }
}
