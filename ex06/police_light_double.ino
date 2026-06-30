// ---------- 引脚定义 ----------
#define LED_A_PIN  2   // 红灯（或任意 LED）
#define LED_B_PIN  5   // 蓝灯（另一路 LED）

// ---------- PWM 配置（使用旧版 API） ----------
const int freq = 5000;        // 5kHz 频率
const int resolution = 8;     // 8位分辨率，亮度 0~255

// ---------- 呼吸参数 ----------
int brightnessA = 0;          // LED_A 当前亮度 0~255
int brightnessB = 255;        // LED_B 初始与 A 反相
int step = 1;                 // 亮度步进（每次增加或减少的值）
unsigned long lastUpdate = 0;
int updateInterval = 10;      // 每 10ms 更新一次，控制速度（值越小闪烁越快）

// ---------- setup ----------
void setup() {
  Serial.begin(115200);
  delay(1000);

  // 初始化两个 PWM 通道（旧版 API）
  ledcAttach(LED_A_PIN, freq, resolution);
  ledcAttach(LED_B_PIN, freq, resolution);

  // 设置初始亮度
  ledcWrite(LED_A_PIN, brightnessA);
  ledcWrite(LED_B_PIN, brightnessB);
}

// ---------- loop ----------
void loop() {
  // 定时更新亮度
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    // 1. 更新 LED_A 亮度（线性渐变）
    brightnessA += step;
    // 边界检查：当达到 0 或 255 时反转方向
    if (brightnessA >= 255) {
      brightnessA = 255;
      step = -1;          // 开始下降
    } else if (brightnessA <= 0) {
      brightnessA = 0;
      step = 1;           // 开始上升
    }

    // 2. LED_B 亮度与 LED_A 严格反相
    brightnessB = 255 - brightnessA;

    // 3. 输出 PWM
    ledcWrite(LED_A_PIN, brightnessA);
    ledcWrite(LED_B_PIN, brightnessB);
  }

  // 小延时释放 CPU
  delay(1);
}