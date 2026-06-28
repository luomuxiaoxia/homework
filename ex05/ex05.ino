const int touchPin = T0;
const int ledPin   = 2;

const int PWM_FREQ    = 5000;    // PWM 频率 5kHz
const int PWM_RES     = 8;       // 8 位分辨率 (0–255)

int speedLevel = 1;              // 当前速度档位 (1/2/3)
const int maxLevel = 3;

// 各档位对应的 delay 时长 (ms) —— 越小呼吸越快
const int speedDelay[4] = {0, 15, 7, 3};  // 索引 1/2/3 对应三档

// ---------- 触摸自锁相关变量 ----------
bool lastTouch    = false;
unsigned long lastToggleTime = 0;
const unsigned long debounceDelay = 200;

void setup() {
  Serial.begin(115200);

  // 配置 LEDC PWM
  ledcAttach(ledPin, PWM_FREQ, PWM_RES);
  ledcWrite(ledPin, 0);

  Serial.println("ex05: 多档位触摸调速呼吸灯");
  Serial.print("当前档位: "); Serial.println(speedLevel);
}

void loop() {
  // ---- 触摸检测 & 档位切换 ----
  bool curTouch = (touchRead(touchPin) < 30);

  if (!lastTouch && curTouch) {
    unsigned long now = millis();
    if (now - lastToggleTime > debounceDelay) {
      // 档位循环: 1→2→3→1
      speedLevel = (speedLevel % maxLevel) + 1;
      lastToggleTime = now;

      Serial.print("档位切换 → ");
      Serial.print(speedLevel);
      Serial.print(" (delay=");
      Serial.print(speedDelay[speedLevel]);
      Serial.println("ms)");
    }
  }
  lastTouch = curTouch;

  // ---- 呼吸灯 PWM 渐变 ----
  int step = 3;                           // 每次增减步长
  int dly  = speedDelay[speedLevel];      // 当前档位的延时

  // 渐亮：0 → 255
  for (int duty = 0; duty <= 255; duty += step) {
    ledcWrite(ledPin, duty);
    delay(dly);

    // 渐变过程中也检测触摸（避免卡顿感）
    curTouch = (touchRead(touchPin) < 30);
    if (!lastTouch && curTouch) {
      unsigned long now = millis();
      if (now - lastToggleTime > debounceDelay) {
        speedLevel = (speedLevel % maxLevel) + 1;
        lastToggleTime = now;
        dly = speedDelay[speedLevel];
        Serial.print("档位切换 → "); Serial.print(speedLevel);
        Serial.print(" (delay="); Serial.print(dly); Serial.println("ms)");
      }
    }
    lastTouch = curTouch;
  }

  // 渐暗：255 → 0
  for (int duty = 255; duty >= 0; duty -= step) {
    ledcWrite(ledPin, duty);
    delay(dly);

    curTouch = (touchRead(touchPin) < 30);
    if (!lastTouch && curTouch) {
      unsigned long now = millis();
      if (now - lastToggleTime > debounceDelay) {
        speedLevel = (speedLevel % maxLevel) + 1;
        lastToggleTime = now;
        dly = speedDelay[speedLevel];
        Serial.print("档位切换 → "); Serial.print(speedLevel);
        Serial.print(" (delay="); Serial.print(dly); Serial.println("ms)");
      }
    }
    lastTouch = curTouch;
  }
}
