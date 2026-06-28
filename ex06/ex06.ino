const int ledPinA = 2;    // LED A
const int ledPinB = 4;    // LED B

const int PWM_FREQ  = 5000;
const int PWM_RES   = 8;  // 0–255

void setup() {
  Serial.begin(115200);

  // 初始化两个 PWM 通道
  ledcAttach(ledPinA, PWM_FREQ, PWM_RES);
  ledcAttach(ledPinB, PWM_FREQ, PWM_RES);

  // 起始状态：A 全暗，B 全亮（反相）
  ledcWrite(ledPinA, 0);
  ledcWrite(ledPinB, 255);

  Serial.println("ex06: 警车双闪灯效（双通道反相PWM）");
}

void loop() {
  int step = 2;   // 步长，越小越平滑
  int dly  = 10;  // 每步延时 (ms)

  // 灯A 渐亮 (0→255)，灯B 同步渐暗 (255→0)
  for (int dutyA = 0; dutyA <= 255; dutyA += step) {
    int dutyB = 255 - dutyA;   // 反相
    ledcWrite(ledPinA, dutyA);
    ledcWrite(ledPinB, dutyB);
    delay(dly);
  }

  // 灯A 渐暗 (255→0)，灯B 同步渐亮 (0→255)
  for (int dutyA = 255; dutyA >= 0; dutyA -= step) {
    int dutyB = 255 - dutyA;   // 反相
    ledcWrite(ledPinA, dutyA);
    ledcWrite(ledPinB, dutyB);
    delay(dly);
  }
}
