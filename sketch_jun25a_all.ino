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
const int DHTPIN = 5;           // DHT11 感測腳位 / DHT11 data pin
const int irLedPin = 2;        // IR LED 腳位 / IR LED pin (2)
const int irRxPin = 4;          // 紅外線接收腳位 / IR receiver pin (D2)
const int buttonPin = 0;        // 按鍵腳位 / Button pin
const int buzzerPin = 15;       // 蜂鳴器腳位 / Buzzer pin

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
  tone(irLedPin, 38000); // 啟動 IR LED 輸出 38kHz / Start 38kHz IR LED output

  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // 首頁 / Main page
  server.on("/", []() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang='en'>
<head>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <title>ESP Sensor Monitor</title>
  <script src='https://cdn.jsdelivr.net/npm/chart.js'></script>
  <style>
    body {
      font-family: sans-serif;
      text-align: center;
      background-color: #fff;
      color: #000;
      transition: background-color 0.5s, color 0.5s;
    }
    @media (prefers-color-scheme: dark) {
      body { background-color: #111; color: #eee; }
      button { background-color: #333; color: #fff; }
    }
    canvas { max-width: 600px; margin: 20px auto; display: block; }
    .btn { padding: 10px 20px; font-size: 1em; margin: 10px; }
  </style>
</head>
<body>
  <h2>📊 ESP8266 Sensor Monitor</h2>
  <div id="values">
    🌡 Temp: <span id="temp">--</span> °C |<br>
    💧 Humidity: <span id="humidity">--</span> %<br>
    🌞 LDR: <span id="ldr">--</span> |<br>
    🚦 IR Sensor: <span id="ir">--</span>
  </div>
  <p>
    <button class="btn" onclick="triggerBuzzer()">🔔 Trigger Buzzer</button><br>
    <button class="btn" onclick="toggleChart()">📈 Toggle Chart</button><br>
    <button class="btn" onclick="location.href='/start'">🌈 Start Rainbow</button><br>
    <button class="btn" onclick="location.href='/stop'">🛑 Stop Rainbow</button>
  </p>
  <div id="buzzerLog">🔕 Buzzer not triggered</div>

  <canvas id="chartTemp" height="150"></canvas>
  <canvas id="chartLDR" height="150" style="display:none"></canvas>
  <canvas id="chartIR" height="150" style="display:none"></canvas>

  <script>
    const labels = [];
    const tempData = [], humData = [], ldrData = [], irData = [];
    const ctxTemp = document.getElementById('chartTemp').getContext('2d');
    const ctxLDR = document.getElementById('chartLDR').getContext('2d');
    const ctxIR  = document.getElementById('chartIR').getContext('2d');
    const chartTemp = new Chart(ctxTemp, {
      type: 'line', data: { labels, datasets: [
        { label: 'Temp (°C)', data: tempData, borderColor: 'red', fill: false },
        { label: 'Humidity (%)', data: humData, borderColor: 'blue', fill: false }
      ] },
      options: { responsive: true, scales: { y: { min: 0, max: 100 } } }
    });
    const chartLDR = new Chart(ctxLDR, {
      type: 'line', data: { labels, datasets: [
        { label: 'LDR', data: ldrData, borderColor: 'orange', fill: false }
      ] },
      options: { responsive: true, scales: { y: { min: 0, max: 1024 } } }
    });
    const chartIR = new Chart(ctxIR, {
      type: 'line', data: { labels, datasets: [
        { label: 'IR Sensor (1=detect)', data: irData, borderColor: 'green', fill: false }
      ] },
      options: { responsive: true, scales: { y: { min: 0, max: 1 } } }
    });

    function fetchData() {
      fetch('/data').then(res => res.json()).then(data => {
        const time = new Date().toLocaleTimeString();
        labels.push(time);
        if (labels.length > 30) labels.shift();
        tempData.push(data.temp); if (tempData.length > 30) tempData.shift();
        humData.push(data.humidity); if (humData.length > 30) humData.shift();
        ldrData.push(data.ldr); if (ldrData.length > 30) ldrData.shift();
        irData.push(data.ir); if (irData.length > 30) irData.shift();

        document.getElementById('temp').textContent = data.temp;
        document.getElementById('humidity').textContent = data.humidity;
        document.getElementById('ldr').textContent = data.ldr;
        document.getElementById('ir').textContent = data.ir;

        chartTemp.update(); chartLDR.update(); chartIR.update();
        if (data.buzzer) {
          document.getElementById('buzzerLog').textContent = '🔔 Buzzer was triggered';
        }
      });
    }

    function triggerBuzzer() {
      fetch('/buzzer').then(() => {
        document.getElementById('buzzerLog').textContent = '🔔 Buzzer triggered manually';
      });
    }

    function toggleChart() {
      const l = document.getElementById('chartLDR');
      const i = document.getElementById('chartIR');
      l.style.display = l.style.display === 'none' ? 'block' : 'none';
      i.style.display = i.style.display === 'none' ? 'block' : 'none';
    }

    setInterval(fetchData, 3000);
  </script>
</body>
</html>
    )rawliteral";
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
