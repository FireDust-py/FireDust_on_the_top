#define TOUCH_PIN 4
#define LED_PIN   2

// 阈值（建议先观察串口，取中间值）
int threshold = 500;

// LED状态（true=亮，false=灭）
bool ledState = false;

// ---------- 状态机定义 ----------
enum TouchState {
  IDLE,           // 空闲，等待触摸
  TOUCHED,        // 已检测到有效触摸，等待释放
  WAIT_RELEASE    // 触摸已处理，等待手指离开
};
TouchState state = IDLE;

// 采样计数器（用于去抖）
int stableCount = 0;
const int DEBOUNCE_SAMPLES = 5;   // 采样次数
const int DEBOUNCE_THRESH = 4;    // 至少4次有效才确认

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);     // 板载LED默认熄灭
}

void loop() {
  int touchValue = touchRead(TOUCH_PIN);
  Serial.print("Value: ");
  Serial.println(touchValue);

  bool isTouched = (touchValue < threshold);

  // ---------- 状态机处理 ----------
  switch (state) {
    case IDLE:
      if (isTouched) {
        stableCount++;
        if (stableCount >= DEBOUNCE_THRESH) {
          // 确认为有效触摸 → 翻转LED
          ledState = !ledState;
          digitalWrite(LED_PIN, ledState ? LOW : HIGH);
          Serial.println("LED toggled!");

          // 进入等待释放状态
          state = WAIT_RELEASE;
          stableCount = 0;  // 复位计数
        }
      } else {
        // 未触摸，清零计数
        stableCount = 0;
      }
      break;

    case WAIT_RELEASE:
      // 等待触摸释放（必须是连续未触摸）
      if (!isTouched) {
        stableCount++;
        if (stableCount >= DEBOUNCE_THRESH) {
          // 确认已释放，回到空闲状态
          state = IDLE;
          stableCount = 0;
          Serial.println("Released, ready for next touch.");
        }
      } else {
        // 仍然触摸，复位释放计数（防止瞬间噪声）
        stableCount = 0;
      }
      break;

    default:
      break;
  }

  // 小延时避免串口刷屏，不影响响应
  delay(20);
}