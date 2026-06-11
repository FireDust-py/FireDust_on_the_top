// 定义板载LED引脚，避免使用"魔法数字"
#define LED_PIN 4

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  // 初始化板载LED引脚为输出模式
  pinMode(LED_PIN, OUTPUT); 
}

void loop() {
  Serial.println("Hello ESP32!");
  digitalWrite(LED_PIN, HIGH);   // 点亮LED
  delay(500);                   // 作业要求改为等待0.5秒
  digitalWrite(LED_PIN, LOW);    // 熄灭LED
  delay(500);              // 等待0.5秒
}