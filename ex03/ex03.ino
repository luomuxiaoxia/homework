const int ledPin = 2;

// 定义每个闪灯步骤：{持续时间(毫秒), LED电平}
struct Step {
  unsigned long duration;
  int level;          // HIGH 或 LOW
};

// SOS 模式数组（按顺序执行）
Step sosPattern[] = {
  // S: 短闪3次 (亮200ms, 灭200ms) × 3
  {200, HIGH}, {200, LOW},
  {200, HIGH}, {200, LOW},
  {200, HIGH}, {200, LOW},
  // 字母间隔
  {500, LOW},

  // O: 长闪3次 (亮600ms, 灭200ms) × 3
  {600, HIGH}, {200, LOW},
  {600, HIGH}, {200, LOW},
  {600, HIGH}, {200, LOW},
  // 字母间隔
  {500, LOW},

  // S: 短闪3次 (亮200ms, 灭200ms) × 3
  {200, HIGH}, {200, LOW},
  {200, HIGH}, {200, LOW},
  {200, HIGH}, {200, LOW},
  // 单词间隔（长停顿）
  {2000, LOW}
};

// 计算步骤总数
const int stepCount = sizeof(sosPattern) / sizeof(sosPattern[0]);
int currentStep = 0;                // 当前执行到的步骤索引
unsigned long stepStartTime = 0;    // 当前步骤的开始时间

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  // 设置初始LED状态（第一步）
  digitalWrite(ledPin, sosPattern[0].level);
  stepStartTime = millis();
  Serial.println("SOS Signal Started (non-blocking)");
}

void loop() {
  unsigned long now = millis();
  unsigned long elapsed = now - stepStartTime;

  // 检查当前步骤是否已经完成
  if (elapsed >= sosPattern[currentStep].duration) {
    // 移动到下一步（循环）
    currentStep = (currentStep + 1) % stepCount;
    stepStartTime = now;   // 重置计时器

    // 执行新步骤：设置LED电平
    digitalWrite(ledPin, sosPattern[currentStep].level);

  }

}