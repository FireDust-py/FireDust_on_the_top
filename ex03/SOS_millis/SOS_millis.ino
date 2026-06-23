// 定义LED引脚（ESP32 开发板常见 GPIO2）
const int ledPin = 2;

// 动作结构：定义每个步骤的持续时间（毫秒）和 LED 状态
struct Action {
  int duration;   // 持续时间，单位 ms
  int ledState;   // HIGH 或 LOW
};

// SOS 序列：S（短闪3次） + O（长闪3次） + S（短闪3次）
// 每个“亮/灭”为一个动作，字母间隔和单词间隔也作为灭灯动作
Action sosActions[] = {
  // ---- 字母 S：短闪3次 (亮200ms，灭200ms) ----
  {200, HIGH}, {200, LOW},
  {200, HIGH}, {200, LOW},
  {200, HIGH}, {200, LOW},
  // 字母间隔（灭500ms）
  {500, LOW},

  // ---- 字母 O：长闪3次 (亮600ms，灭200ms) ----
  {600, HIGH}, {200, LOW},
  {600, HIGH}, {200, LOW},
  {600, HIGH}, {200, LOW},
  // 字母间隔（灭500ms）
  {500, LOW},

  // ---- 字母 S：短闪3次 (亮200ms，灭200ms) ----
  {200, HIGH}, {200, LOW},
  {200, HIGH}, {200, LOW},
  {200, HIGH}, {200, LOW},
  // 单词间隔（灭2000ms）
  {2000, LOW}
};

// 计算动作总数
const int totalActions = sizeof(sosActions) / sizeof(sosActions[0]);

// 状态变量
unsigned long lastTime = 0;   // 上一次动作切换的时间
int currentAction = 0;        // 当前执行的动作索引

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  // 初始化：将 LED 设为第一个动作的状态（第一个动作是 HIGH，即亮）
  digitalWrite(ledPin, sosActions[0].ledState);
  lastTime = millis();        // 记录起始时间
}

void loop() {
  unsigned long now = millis();

  // 检查当前动作是否已执行完毕
  if (now - lastTime >= sosActions[currentAction].duration) {
    // 切换到下一个动作
    currentAction++;
    // 如果到达序列末尾，则重新开始
    if (currentAction >= totalActions) {
      currentAction = 0;
    }
    // 执行新动作（改变 LED 状态）
    digitalWrite(ledPin, sosActions[currentAction].ledState);
    
    // 可选：在串口输出当前动作信息（调试用）
    Serial.print("Action ");
    Serial.print(currentAction);
    Serial.print(": LED ");
    Serial.println(sosActions[currentAction].ledState == HIGH ? "ON" : "OFF");
    
    // 更新时间戳
    lastTime = now;
  }

  // ---------- 在这里可以添加其他需要同时执行的任务 ----------
  // 例如：读取传感器、处理按键、发送数据等，它们不会受到 delay 阻塞。
}
