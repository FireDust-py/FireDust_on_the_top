// 定义三个 LED 引脚（可根据实际接线修改）
const int ledRed = 2;    // 红色 LED 接 GPIO 2
const int ledYellow = 4; // 黄色 LED 接 GPIO 4
const int ledGreen = 5;  // 绿色 LED 接 GPIO 5（如果只有红黄绿，可将绿色接在此）

// PWM 属性
const int freq = 5000;
const int resolution = 8; // 0~255

void setup() {
  Serial.begin(115200);
  // 为每个引脚绑定 PWM 通道（ESP32 会自动分配通道）
  ledcAttach(ledRed, freq, resolution);
  ledcAttach(ledYellow, freq, resolution);
  ledcAttach(ledGreen, freq, resolution);
}

void loop() {
  // 彩虹循环：相位从 0 到 2*PI，每步相位增量决定渐变速度
  const int totalSteps = 300;       // 总步数（越大渐变越平滑，但周期变长）
  const float phaseStep = 2 * PI / totalSteps; // 每步相位增量

  for (int i = 0; i <= totalSteps; i++) {
    float phase = i * phaseStep;    // 当前相位（0 ~ 2*PI）

    // 使用正弦波，三个通道相位差 120° (2*PI/3)
    // 亮度映射到 0~255，并取绝对值或偏移以保证始终有颜色变化
    // 方法1：用 (sin + 1)/2 映射到 0~1，再乘255
    float r = (sin(phase) + 1.0) / 2.0;
    float y = (sin(phase + 2*PI/3) + 1.0) / 2.0;
    float g = (sin(phase + 4*PI/3) + 1.0) / 2.0;

    float r_val = (cos(phase) + 1.0) * 127.5;      // 范围 0~255
    float y_val = (cos(phase + 2*PI/3) + 1.0) * 127.5;
    float g_val = (cos(phase + 4*PI/3) + 1.0) * 127.5;

    // 写入PWM
    ledcWrite(ledRed, (int)r_val);
    ledcWrite(ledYellow, (int)y_val);
    ledcWrite(ledGreen, (int)g_val);

    delay(10);  // 每步延时，总周期 = totalSteps * 10ms ≈ 3秒
  }
}
