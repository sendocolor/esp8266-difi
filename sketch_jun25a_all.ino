#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// Wi-Fi 設定 / Wi-Fi setup
const char* ssid = "ESP-RGB";
const char* password = "12345678";

// 建立 Web 伺服器 / Web server on port 80
ESP8266WebServer server(80);

// RGB LED 腳位 / RGB LED pins
const int Red = 12;
const int Green = 13;
const int Blue = 14;

// DHT11 感測器設定 / DHT11 setup
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// 彩虹控制變數 / Rainbow effect control flag
bool rainbowActive = false;
#define FADESPEED 5

void setup() {
  Serial.begin(115200);
  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Blue, OUTPUT);
  dht.begin();

  // 啟動 Wi-Fi 熱點 / Start Wi-Fi access point
  WiFi.softAP(ssid, password);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // 網頁首頁 / Web root page
  server.on("/", []() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>DHT11 Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body {
      font-family: 'Segoe UI', sans-serif;
      margin: 0;
      padding: 20px;
      text-align: center; /* 對齊至中 / center the content */
      background-color: white;
      color: #222;
      transition: background-color 0.5s, color 0.5s;
    }

    @media (prefers-color-scheme: dark) {
      body {
        background-color: #111;
        color: #eee;
      }
      button {
        background-color: #444;
      }
    }

    h2 {
      font-size: 1.6em;
    }

    #values {
      font-size: 1.4em;
      margin: 10px 0;
    }

    canvas {
      width: 100%;
      max-width: 600px;
      margin: auto;
    }

    .btn {
      display: inline-block;
      margin: 12px;
      padding: 14px 28px;
      font-size: 1.1em;
      background-color: #2196F3;
      color: white;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      transition: 0.3s;
    }

    .btn:hover {
      background-color: #0b7dda;
    }
  </style>
</head>
<body>
  <h2>🌡️ DHT11 Temperature & Humidity</h2>
  <div id="values">
    🌡 Temp: <span id="temp">--</span> °C |
    💧 Humidity: <span id="humidity">--</span> %
  </div>
  <canvas id="myChart"></canvas>
  <div>
    <a href="/start"><button class="btn">🌈 Start Rainbow</button></a>
    <a href="/stop"><button class="btn">🛑 Stop</button></a>
  </div>

  <script>
    const labels = [];
    const tempData = [];
    const humData = [];

    const ctx = document.getElementById('myChart').getContext('2d');
    const myChart = new Chart(ctx, {
      type: 'line',
      data: {
        labels: labels,
        datasets: [{
          label: 'Temp (°C)',
          data: tempData,
          borderColor: 'red',
          fill: false
        }, {
          label: 'Humidity (%)',
          data: humData,
          borderColor: 'blue',
          fill: false
        }]
      },
      options: {
        responsive: true,
        scales: {
          x: { title: { display: true, text: 'Time' } },
          y: { title: { display: true, text: 'Value' }, suggestedMin: 0, suggestedMax: 100 }
        }
      }
    });

    // 每 3 秒向 /data 取得 JSON 溫濕度 / fetch DHT11 data every 3 seconds
    function fetchData() {
      fetch('/data')
        .then(res => res.json())
        .then(data => {
          const now = new Date();
          const t = data.temp.toFixed(1);
          const h = data.humidity.toFixed(1);
          document.getElementById('temp').textContent = t;
          document.getElementById('humidity').textContent = h;

          const timeStr = now.toLocaleTimeString();
          labels.push(timeStr);
          tempData.push(t);
          humData.push(h);

          if (labels.length > 20) {
            labels.shift(); tempData.shift(); humData.shift();
          }
          myChart.update();
        });
    }

    setInterval(fetchData, 3000); // 每三秒更新圖表 / update chart every 3s
  </script>
</body>
</html>
    )rawliteral";

    server.send(200, "text/html", html);
  });

  // 回傳 JSON 格式資料 / Serve JSON data from DHT11
  server.on("/data", []() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    String json = "{\"temp\":" + String(t, 1) + ",\"humidity\":" + String(h, 1) + "}";
    server.send(200, "application/json", json);
  });

  // 開啟彩虹模式 / Start rainbow
  server.on("/start", []() {
    rainbowActive = true;
    server.send(200, "text/html", "<h2>🌈 Rainbow started</h2><a href='/'>Back</a>");
  });

  // 停止彩虹模式 / Stop rainbow
  server.on("/stop", []() {
    rainbowActive = false;
    analogWrite(Red, 0);
    analogWrite(Green, 0);
    analogWrite(Blue, 0);
    server.send(200, "text/html", "<h2>🛑 Rainbow stopped</h2><a href='/'>Back</a>");
  });

  server.begin(); // 啟動網頁伺服器 / Start web server
  Serial.println("Web server started");
}

void loop() {
  server.handleClient(); // 處理客戶端請求 / Handle client requests

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
