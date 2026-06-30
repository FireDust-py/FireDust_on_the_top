#include <WiFi.h>
#include <WebServer.h>

// ---------- Wi-Fi 配置 ----------
const char* ssid = "FireDust";
const char* password = "88888888";

// ---------- 引脚定义 ----------
#define TOUCH_PIN 4   // 触摸引脚 T0

WebServer server(80);

// ---------- 根路径：返回仪表盘页面 ----------
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 触摸仪表盘</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      background: #0b0e14;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      color: #fff;
    }
    .container {
      background: #1a1f2b;
      padding: 40px 50px;
      border-radius: 30px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.5);
      text-align: center;
      width: 90%;
      max-width: 500px;
    }
    h1 {
      font-weight: 300;
      font-size: 1.8rem;
      color: #8a9bb5;
      margin-bottom: 20px;
      letter-spacing: 2px;
    }
    .value-display {
      font-size: 6rem;
      font-weight: 700;
      color: #4fc3f7;
      margin: 15px 0;
      line-height: 1.2;
      transition: color 0.2s;
    }
    .value-display.low {
      color: #ff6b6b;
    }
    .value-display.medium {
      color: #ffd93d;
    }
    .value-display.high {
      color: #4fc3f7;
    }
    .progress-container {
      width: 100%;
      height: 12px;
      background: #2a3140;
      border-radius: 10px;
      overflow: hidden;
      margin: 20px 0 10px;
    }
    .progress-bar {
      height: 100%;
      width: 0%;
      border-radius: 10px;
      background: #4fc3f7;
      transition: width 0.15s ease, background 0.3s;
    }
    .label {
      font-size: 0.9rem;
      color: #6b7b93;
      margin-top: 5px;
      letter-spacing: 1px;
    }
    .status-text {
      margin-top: 15px;
      font-size: 1.1rem;
      color: #8a9bb5;
    }
    .unit {
      font-size: 2rem;
      color: #6b7b93;
    }
    .footer {
      margin-top: 25px;
      font-size: 0.8rem;
      color: #3a4458;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>📊 触摸传感器</h1>
    <div class="value-display" id="valueDisplay">--</div>
    <div class="progress-container">
      <div class="progress-bar" id="progressBar"></div>
    </div>
    <div class="label">触摸强度</div>
    <div class="status-text" id="statusText">等待数据...</div>
    <div class="footer">数据实时更新 · 手指靠近数值减小</div>
  </div>

  <script>
    const display = document.getElementById('valueDisplay');
    const progress = document.getElementById('progressBar');
    const statusText = document.getElementById('statusText');

    // 最大参考值（可根据实际情况调整，或动态学习）
    const MAX_REF = 600;   // 未触摸时典型值（如500左右），可根据你的环境修改

    // 更新界面
    function updateUI(value) {
      // 显示数值
      display.textContent = value;

      // 颜色分级：值越小（触摸越强）越红
      const ratio = Math.min(value / MAX_REF, 1); // 0~1
      if (ratio < 0.3) {
        display.className = 'value-display low';
        statusText.textContent = '⚠️ 强触摸';
      } else if (ratio < 0.6) {
        display.className = 'value-display medium';
        statusText.textContent = '🔶 中等感应';
      } else {
        display.className = 'value-display high';
        statusText.textContent = '✅ 空闲';
      }

      // 进度条：反向显示（值越小，进度条越长，表示触发程度）
      const progressPercent = (1 - ratio) * 100;
      progress.style.width = progressPercent + '%';
      // 进度条颜色渐变
      if (progressPercent > 70) {
        progress.style.background = '#ff6b6b';
      } else if (progressPercent > 40) {
        progress.style.background = '#ffd93d';
      } else {
        progress.style.background = '#4fc3f7';
      }
    }

    // 从服务器获取触摸值
    function fetchTouch() {
      fetch('/touch')
        .then(response => {
          if (!response.ok) throw new Error('Network error');
          return response.json();
        })
        .then(data => {
          if (data.hasOwnProperty('value')) {
            updateUI(data.value);
          }
        })
        .catch(err => {
          statusText.textContent = '❌ 连接失败';
          console.warn(err);
        });
    }

    // 每200ms更新一次
    setInterval(fetchTouch, 200);
    // 立即加载一次
    fetchTouch();
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html; charset=UTF-8", html);
}

// ---------- API：返回触摸值 JSON ----------
void handleTouch() {
  int value = touchRead(TOUCH_PIN);
  String json = "{\"value\":" + String(value) + "}";
  server.send(200, "application/json", json);
}

// ---------- setup ----------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n启动触摸仪表盘...");

  // 连接 Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("连接WiFi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n连接成功");
    Serial.print("访问地址: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n连接超时！请检查热点名称和密码。");
  }

  // 注册路由
  server.on("/", handleRoot);
  server.on("/touch", handleTouch);
  server.begin();
  Serial.println("Web服务器已启动");
}

// ---------- loop ----------
void loop() {
  server.handleClient();
}
