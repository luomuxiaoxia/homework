#define ledPin 2
unsigned long previousMillis = 0;
const long interval = 1000;   // 1秒
int ledState = LOW;
void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState;          // 翻转状态
    digitalWrite(ledPin, ledState);
    Serial.println(ledState ? "ON" : "OFF");
  }
}