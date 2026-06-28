#include <WiFi.h>
#include <WebServer.h>

// ========== WiFi AP 配置 (ESP32 自己开热点) ==========
const char* ap_ssid     = "ESP32_Alarm";
const char* ap_password = "12345678";   // 至少8位，手机连接时输入

WebServer server(80);

const int touchPin = T0;
const int ledPin   = 2;

// ========== 系统状态 ==========
enum State {
  DISARMED,   // 撤防
  ARMED,      // 布防（等待触发）
  ALARM       // 报警中
};
State sysState = DISARMED;

// 报警闪烁控制
unsigned long lastBlink = 0;
bool blinkOn = false;
const int blinkInterval = 100;   // 报警闪烁间隔 100ms（高频）

// ========== HTML 页面 ==========
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 安防报警器</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin-top: 60px;
      background: #1a1a2e;
      color: #eee;
    }
    h2 { color: #e94560; }
    .container {
      background: #16213e;
      border-radius: 16px;
      padding: 30px;
      max-width: 400px;
      margin: 0 auto;
      box-shadow: 0 4px 20px rgba(0,0,0,0.4);
    }
    .status {
      font-size: 24px;
      font-weight: bold;
      padding: 16px;
      border-radius: 8px;
      margin: 16px 0;
    }
    .disarmed { background: #2d6a4f; color: #95d5b2; }
    .armed    { background: #b8860b; color: #ffe066; }
    .alarm    { background: #c0392b; color: #ffcccc; animation: pulse 0.5s infinite; }
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50%      { opacity: 0.5; }
    }
    button {
      font-size: 20px;
      padding: 14px 40px;
      margin: 10px;
      border: none;
      border-radius: 10px;
      cursor: pointer;
      font-weight: bold;
    }
    .btn-arm    { background: #e76f51; color: white; }
    .btn-disarm { background: #2a9d8f; color: white; }
    button:active { transform: scale(0.95); }
  </style>
</head>
<body>
  <div class="container">
    <h2>&#128737; ESP32 安防报警器</h2>
    <div class="status disarmed" id="statusBox">状态：撤防</div>
    <button class="btn-arm"    onclick="sendCmd('arm')">    &#128737; 布防 (Arm)</button>
    <button class="btn-disarm" onclick="sendCmd('disarm')">&#128275; 撤防 (Disarm)</button>
  </div>

  <script>
    function sendCmd(cmd) {
      fetch('/' + cmd)
        .then(r => r.text())
        .then(state => updateUI(state));
    }

    function updateUI(state) {
      const box = document.getElementById('statusBox');
      if (state === 'DISARMED') {
        box.textContent = '状态：撤防';
        box.className = 'status disarmed';
      } else if (state === 'ARMED') {
        box.textContent = '状态：布防中';
        box.className = 'status armed';
      } else if (state === 'ALARM') {
        box.textContent = '状态：⚠ 报警！';
        box.className = 'status alarm';
      }
    }

    // 定时轮询当前状态
    setInterval(() => {
      fetch('/status')
        .then(r => r.text())
        .then(state => updateUI(state));
    }, 1000);

    // 页面加载时获取初始状态
    fetch('/status')
      .then(r => r.text())
      .then(state => updateUI(state));
  </script>
</body>
</html>
)rawliteral";

// ========== 路由处理 ==========
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", htmlPage);
}

void handleArm() {
  sysState = ARMED;
  digitalWrite(ledPin, LOW);  // 确保LED初始熄灭
  server.send(200, "text/plain", "ARMED");
  Serial.println(">> 系统已布防");
}

void handleDisarm() {
  sysState = DISARMED;
  digitalWrite(ledPin, LOW);  // 熄灭LED
  server.send(200, "text/plain", "DISARMED");
  Serial.println(">> 系统已撤防");
}

void handleStatus() {
  switch (sysState) {
    case DISARMED: server.send(200, "text/plain", "DISARMED"); break;
    case ARMED:    server.send(200, "text/plain", "ARMED");    break;
    case ALARM:    server.send(200, "text/plain", "ALARM");    break;
  }
}

// ========== 状态名称辅助函数 ==========
const char* stateName(State s) {
  switch (s) {
    case DISARMED: return "撤防";
    case ARMED:    return "布防";
    case ALARM:    return "报警!";
  }
  return "?";
}

void setup() {
  Serial.begin(115200);
  while (!Serial);     // 等待USB串口就绪 (C3/S3必需)
  delay(300);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // 开启 AP 热点
  Serial.print("正在开启热点: ");
  Serial.println(ap_ssid);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("热点已开启！IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/",       handleRoot);
  server.on("/arm",    handleArm);
  server.on("/disarm", handleDisarm);
  server.on("/status", handleStatus);
  server.begin();

  Serial.println("安防系统就绪，默认撤防");
  Serial.println("================================");
  Serial.print("  WiFi名称: ");
  Serial.println(ap_ssid);
  Serial.print("  WiFi密码: ");
  Serial.println(ap_password);
  Serial.print("  访问地址: http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("================================");
  Serial.println(">>> 用手机连接上方WiFi，再打开浏览器输入地址 <<<");
}

void loop() {
  server.handleClient();

  // ---- 触摸检测 ----
  bool touched = (touchRead(touchPin) < 30);

  if (sysState == ARMED && touched) {
    sysState = ALARM;
    Serial.println("!!! 报警触发 !!!");
  }

  // ---- 报警闪烁 ----
  if (sysState == ALARM) {
    unsigned long now = millis();
    if (now - lastBlink >= blinkInterval) {
      lastBlink = now;
      blinkOn = !blinkOn;
      digitalWrite(ledPin, blinkOn ? HIGH : LOW);
    }
  }

  delay(5);
}
