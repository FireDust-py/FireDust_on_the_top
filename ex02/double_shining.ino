// 定义两个 LED 引脚
const int ledPin1 = 2;   // 第一个 LED 接 GPIO 2
const int ledPin2 = 4;   // 第二个 LED 接 GPIO 4（需串联限流电阻）

void setup() {
  Serial.begin(115200);
  // 将两个引脚都设置为输出模式
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
}

void loop() {
  // 状态1：LED1 亮，LED2 灭
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, LOW);
  Serial.println("LED1 ON, LED2 OFF");
  delay(1000);   // 持续 1 秒

  // 状态2：LED1 灭，LED2 亮
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, HIGH);
  Serial.println("LED1 OFF, LED2 ON");
  delay(1000);   // 持续 1 秒
}