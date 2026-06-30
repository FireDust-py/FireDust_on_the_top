#include <WiFi.h>
#include <WebServer.h>

// ---------- Wi-Fi 配置 ----------
const char* ssid = "FireDust";
const char* password = "88888888";

// ---------- 引脚定义 ----------
#define TOUCH_PIN 4      // 触摸引脚（T0）
#define LED_PIN   2      // 板载 LED 或外接 LED

// ---------- 触摸阈值（提示设为 500） ----------
// 注意：ESP32 touchRead() 在触摸时数值减小，阈值需根据实际串口数值调整
// 若未触摸值 > 500，触摸后 < 500，则 500 可作为中间值
int threshold = 500;

// ---------- 状态变量 ----------
bool armed = false;          // 布防状态
bool alarmActive = false;    // 报警触发状态

// ---------- 闪烁控制 ----------
unsigned long lastBlink = 0;
const int blinkInterval = 100;   // 100ms 闪烁周期
bool ledState = false;

WebServer server(80);

// ---------- 辅助：更新 LED 状态 ----------
void updateLED() {
  if (alarmActive) {
    // 报警时 LED 由闪烁控制，此处不做操作
    return;
  }
  if (armed) {
    digitalWrite(LED_PIN, HIGH);   // 布防常亮
  } else {
    digitalWrite(LED_PIN, LOW);    // 撤防熄灭
  }
}

// ---------- 根路径：显示控制页面 ----------
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>物联网安防报警器</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin-top: 50px;
      background: #f0f0f0;
    }
    .container {
      background: white;
      padding: 30px;
      border-radius: 12px;
      max-width: 400px;
      margin: 0 auto;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    }
    h1 { color: #333; }
    .status {
      font-size: 20px;
      margin: 20px 0;
      padding: 10px;
      border-radius: 8px;
    }
    .status.armed { background: #ffeb3b; color: #333; }
    .status.disarmed { background: #4caf50; color: white; }
    .status.alarm { background: #f44336; color: white; animation: blink 0.5s infinite; }
    @keyframes blink {
      0% { opacity: 1; }
      50% { opacity: 0.3; }
    }
    button {
      padding: 12px 30px;
      font-size: 18px;
      margin: 10px;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      color: white;
      transition: 0.3s;
    }
    button.arm { background: #2196F3; }
    button.arm:hover { background: #1976D2; }
    button.disarm { background: #f44336; }
    button.disarm:hover { background: #d32f2f; }
    button:disabled { opacity: 0.5; cursor: not-allowed; }
    .info { margin-top: 20px; color: #666; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔐 安防报警器</h1>
    <div id="statusDisplay" class="status disarmed">撤防中</div>
    <div>
      <button class="arm" id="armBtn" onclick="setArm(true)">布防</button>
      <button class="disarm" id="disarmBtn" onclick="setArm(false)">撤防</button>
    </div>
    <div class="info">布防后触碰触摸引脚将触发报警</div>
  </div>

  <script>
    // 更新状态显示
    function updateStatus(data) {
      const display = document.getElementById('statusDisplay');
      if (data.alarmActive) {
        display.className = 'status alarm';
        display.textContent = '🚨 报警中！';
      } else if (data.armed) {
        display.className = 'status armed';
        display.textContent = '🔒 已布防';
      } else {
        display.className = 'status disarmed';
        display.textContent = '🔓 已撤防';
      }
      // 控制按钮可用性（布防后禁用布防，撤防后禁用撤防）
      document.getElementById('armBtn').disabled = data.armed;
      document.getElementById('disarmBtn').disabled = !data.armed;
    }

    // 获取最新状态（轮询）
    function fetchStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => updateStatus(data))
        .catch(err => console.warn('状态获取失败', err));
    }

    // 发送布防/撤防请求
    function setArm(arm) {
      const action = arm ? 'arm' : 'disarm';
      fetch('/' + action, { method: 'POST' })
        .then(response => response.json())
        .then(data => {
          updateStatus(data);
        })
        .catch(err => console.warn('请求失败', err));
    }

    // 首次加载获取状态，并每1秒轮询
    window.onload = function() {
      fetchStatus();
      setInterval(fetchStatus, 1000);
    };
  </script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html; charset=UTF-8", html);
}

// ---------- API：布防 ----------
void handleArm() {
  armed = true;
  alarmActive = false;   // 布防时清除报警状态
  updateLED();
  // 返回 JSON 状态
  String json = "{\"armed\":true,\"alarmActive\":false}";
  server.send(200, "application/json", json);
}

// ---------- API：撤防 ----------
void handleDisarm() {
  armed = false;
  alarmActive = false;
  updateLED();
  String json = "{\"armed\":false,\"alarmActive\":false}";
  server.send(200, "application/json", json);
}

// ---------- API：获取状态 ----------
void handleStatus() {
  String json = "{\"armed\":" + String(armed ? "true" : "false") +
                ",\"alarmActive\":" + String(alarmActive ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

// ---------- setup ----------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n启动安防报警器...");   // 这行应该在WiFi.begin前

  // 初始化 LED 引脚
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // ---- 连接 Wi-Fi 并增加超时 ----
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
    // 即使连不上，也继续启动 Web 服务器（但可能无法访问）
  }

  // 注册路由和启动服务器
  server.on("/", handleRoot);
  server.on("/arm", HTTP_POST, handleArm);
  server.on("/disarm", HTTP_POST, handleDisarm);
  server.on("/status", HTTP_GET, handleStatus);
  server.begin();
  Serial.println("Web服务器已启动（若WiFi未连接，请检查）");
}

// ---------- loop ----------
void loop() {
  // 1. 处理 Web 请求
  server.handleClient();

  // 2. 触摸检测（仅在布防且未报警时检测）
  if (armed && !alarmActive) {
    int touchValue = touchRead(TOUCH_PIN);
    if (touchValue < threshold) {
      alarmActive = true;
      updateLED();   // 闪烁控制将在后续执行
      Serial.println("⚠️ 触发报警！");
    }
  }

  // 3. 报警闪烁控制（非阻塞）
  if (alarmActive) {
    if (millis() - lastBlink >= blinkInterval) {
      lastBlink = millis();
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    }
  } else {
    // 非报警状态，由 updateLED 控制
    // 但 updateLED 只在状态变化时调用，这里保证常亮/常灭
    // 因为非报警时，updateLED 已经在 arm/disarm 时调用，但以防万一，我们在这里也做一次？
    // 但为了避免干扰，可再次调用，但会频繁写，无妨。
    // 我们可以用标志，但简单处理：不在报警时，直接调用 updateLED
    // 但 updateLED 会根据 armed 设置固定电平，因此覆盖掉闪烁。
    // 但是我们希望闪烁时不受干扰，而 alarmActive=false 时，闪烁停止。
    // 所以我们可以在此检查，如果 alarmActive 变为 false，则调用 updateLED 恢复到正常状态。
    // 由于 alarmActive 只有 arm/disarm 和触摸触发改变，触摸触发时设置 alarmActive=true，
    // 而 arm/disarm 会清除 alarmActive 并调用 updateLED，所以这里我们不需要额外操作。
    // 但为了安全，如果 alarmActive 为 false 且之前可能有闪烁残留，我们调用 updateLED 确保状态正确。
    // 由于我们每次 arm/disarm 都会调用 updateLED，所以没问题。
    // 但这里为了让闪烁停止时立即恢复，我们可以在 alarmActive 变为 false 时调用。
    // 因为我们检测到 alarmActive 为 false，但可能之前刚被清除，我们直接调用 updateLED。
    // 不过为了简化，我们可以加一个标志记录上次的 alarmActive，如果从 true 变 false，则调用 updateLED。
    // 这里采用一个静态变量记录上次值。
    static bool lastAlarm = false;
    if (lastAlarm != alarmActive) {
      lastAlarm = alarmActive;
      if (!alarmActive) {
        updateLED();   // 停止闪烁，恢复布防/撤防状态
      }
    }
  }

  // 小延时释放CPU
  delay(10);
}