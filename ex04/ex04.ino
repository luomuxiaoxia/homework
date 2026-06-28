const int touchPin = T0;       // 触摸引脚 T0 = GPIO4
const int ledPin   = 2;        // LED 引脚（内置LED通常为 GPIO2）

bool ledState     = false;     // LED 当前状态（false=灭，true=亮）
bool lastTouch    = false;     // 上一次循环的触摸状态
unsigned long lastToggleTime = 0;           // 上次翻转的时刻 (ms)
const unsigned long debounceDelay = 200;    // 防抖延时 (ms)

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.println("ex04: 触摸自锁开关已就绪");
  Serial.println("触摸 T0 引脚可翻转 LED 状态（防抖 200ms）");
}

void loop() {
  // 读取当前触摸状态（低于阈值=被触摸）
  bool curTouch = (touchRead(touchPin) < 30);

  // 边缘检测：上一次未触摸 && 本次触摸 → 上升沿（按下瞬间）
  if (!lastTouch && curTouch) {
    // 软件防抖：距离上次翻转必须超过 debounceDelay
    unsigned long now = millis();
    if (now - lastToggleTime > debounceDelay) {
      ledState = !ledState;                 // 翻转状态
      digitalWrite(ledPin, ledState ? HIGH : LOW);
      lastToggleTime = now;

      Serial.print("触摸触发！LED → ");
      Serial.println(ledState ? "ON" : "OFF");
    }
  }

  // 保存本次触摸状态供下一次循环比较
  lastTouch = curTouch;

  delay(10);  // 小延时，稳定读取
}
