// ---------- 引脚定义 ----------
#define TOUCH_PIN   4
#define LED_PIN     2

// ---------- PWM 配置（旧版 API） ----------
const int freq = 5000;          // 频率 5000Hz
const int resolution = 8;       // 分辨率 8位 (0-255)

// ---------- 触摸阈值 ----------
int threshold = 500;

// ---------- 档位变量 ----------
int speedLevel = 1;
int updateInterval = 30;        // 亮度更新间隔 (ms)

// ---------- 呼吸变量 ----------
int brightness = 0;
int fadeStep = 1;
unsigned long lastUpdate = 0;

// ---------- 触摸状态机 ----------
enum TouchState { IDLE, WAIT_RELEASE };
TouchState state = IDLE;
int stableCount = 0;
const int DEBOUNCE_SAMPLES = 5;
const int DEBOUNCE_THRESH = 4;

// ---------- 辅助函数：根据档位设置间隔 ----------
void setSpeedByLevel(int level) {
  switch (level) {
    case 1: updateInterval = 30; break;   // 缓慢呼吸
    case 2: updateInterval = 15; break;   // 中等速度
    case 3: updateInterval = 6;  break;   // 快速呼吸
    default: updateInterval = 30;
  }
  Serial.print("Speed Level: ");
  Serial.print(level);
  Serial.print(", Interval: ");
  Serial.println(updateInterval);
}

// ---------- setup ----------
void setup() {
  Serial.begin(115200);
  delay(1000);

  // 使用旧版 API 初始化 PWM（与你的呼吸灯代码一致）
  ledcAttach(LED_PIN, freq, resolution);
  ledcWrite(LED_PIN, 0);   // 初始熄灭

  // 设置初始档位
  setSpeedByLevel(speedLevel);
}

// ---------- loop ----------
void loop() {
  // ===== 1. 触摸检测（状态机） =====
  int touchValue = touchRead(TOUCH_PIN);
  bool isTouched = (touchValue < threshold);

  switch (state) {
    case IDLE:
      if (isTouched) {
        stableCount++;
        if (stableCount >= DEBOUNCE_THRESH) {
          // 有效触摸：切换档位
          speedLevel = (speedLevel % 3) + 1;   // 1→2→3→1
          setSpeedByLevel(speedLevel);
          state = WAIT_RELEASE;
          stableCount = 0;
        }
      } else {
        stableCount = 0;
      }
      break;

    case WAIT_RELEASE:
      if (!isTouched) {
        stableCount++;
        if (stableCount >= DEBOUNCE_THRESH) {
          state = IDLE;
          stableCount = 0;
        }
      } else {
        stableCount = 0;
      }
      break;
  }

  // ===== 2. 呼吸灯更新（非阻塞） =====
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    // 亮度步进（线性渐变，也可改为正弦，但线性简单）
    brightness += fadeStep;
    if (brightness >= 255) {
      brightness = 255;
      fadeStep = -1;
    } else if (brightness <= 0) {
      brightness = 0;
      fadeStep = 1;
    }
    ledcWrite(LED_PIN, brightness);   // 使用引脚号，与旧版 API 一致
  }

  delay(5);   // 小延时，不影响触摸采样
}