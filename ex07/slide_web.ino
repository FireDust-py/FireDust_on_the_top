#include <WiFi.h>
#include <WebServer.h>

// ---------- Wi-Fi 配置 ----------
const char* ssid = "FireDust";
const char* password = "88888888";

// ---------- LED 引脚 ----------
#define LED_PIN 2   // 板载 LED 或外接 LED

// ---------- PWM 配置（旧版 API） ----------
const int freq = 5000;        // 5kHz
const int resolution = 8;     // 8位，0~255

WebServer server(80);

// ---------- 根路径：返回带滑动条的 HTML 页面 ----------
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 无极调光器</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin-top: 50px;
      background-color: #f0f0f0;
    }
    .container {
      background: white;
      padding: 30px;
      border-radius: 12px;
      max-width: 400px;
      margin: 0 auto;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    h1 {
      color: #333;
    }
    .slider {
      width: 100%;
      height: 15px;
      border-radius: 10px;
      background: linear-gradient(to right, #000, #fff);
      outline: none;
      -webkit-appearance: none;
      appearance: none;
    }
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 25px;
      height: 25px;
      border-radius: 50%;
      background: #2196F3;
      cursor: pointer;
    }
    .slider::-moz-range-thumb {
      width: 25px;
      height: 25px;
      border-radius: 50%;
      background: #2196F3;
      cursor: pointer;
    }
    #brightnessValue {
      font-size: 24px;
      font-weight: bold;
      color: #2196F3;
      margin-top: 10px;
      display: inline-block;
      padding: 5px 20px;
      background: #e3f2fd;
      border-radius: 20px;
    }
    .info {
      margin-top: 20px;
      color: #666;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>💡 无极调光器</h1>
    <p>拖动滑块调节亮度</p>
    <input type="range" min="0" max="255" value="0" class="slider" id="brightnessSlider">
    <br>
    <span id="brightnessValue">0</span>
    <div class="info">ESP32 PWM 调光</div>
  </div>

  <script>
    const slider = document.getElementById('brightnessSlider');
    const display = document.getElementById('brightnessValue');

    // 当滑块值变化时（实时）
    slider.addEventListener('input', function() {
      const val = this.value;
      display.textContent = val;
      // 发送 GET 请求到 ESP32，带亮度值
      fetch(`/slider?value=${val}`)
        .then(response => {
          if (!response.ok) {
            console.warn('请求失败');
          }
        })
        .catch(err => console.warn('网络错误', err));
    });
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html; charset=UTF-8", html);
}

// ---------- 处理滑块请求：/slider?value=xxx ----------
void handleSlider() {
  if (server.hasArg("value")) {
    int brightness = server.arg("value").toInt();
    // 限制范围 0~255
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;
    // 设置 PWM 占空比
    ledcWrite(LED_PIN, brightness);
    // 可选：打印到串口以便调试
    Serial.print("Brightness set to: ");
    Serial.println(brightness);
    // 返回成功响应（可返回纯文本或JSON）
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing 'value' parameter");
  }
}

// ---------- setup ----------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n启动...");

  // 初始化 PWM
  ledcAttach(LED_PIN, freq, resolution);
  ledcWrite(LED_PIN, 0);   // 默认熄灭

  // 连接 Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("连接WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n连接成功");
  Serial.print("访问地址: http://");
  Serial.println(WiFi.localIP());

  // 注册路由
  server.on("/", handleRoot);
  server.on("/slider", handleSlider);
  server.begin();
  Serial.println("Web服务器已启动");
}

// ---------- loop ----------
void loop() {
  server.handleClient();
}