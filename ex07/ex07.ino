#include <WiFi.h>
#include <WebServer.h>

// ========== WiFi AP 配置 (ESP32 自己开热点) ==========
const char* ap_ssid     = "ESP32_Dimmer";
const char* ap_password = "12345678";   // 至少8位，手机连接时输入

WebServer server(80);

const int ledPin     = 2;
const int PWM_FREQ   = 5000;
const int PWM_RES    = 8;

// ========== HTML 页面 ==========
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 无极调光器</title>
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
    input[type=range] {
      width: 90%;
      height: 30px;
      -webkit-appearance: none;
      background: #0f3460;
      border-radius: 15px;
      outline: none;
      margin: 20px 0;
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 36px;
      height: 36px;
      background: #e94560;
      border-radius: 50%;
      cursor: pointer;
    }
    .value {
      font-size: 48px;
      font-weight: bold;
      color: #f5c518;
    }
  </style>
</head>
<body>
  <div class="container">
    <h2>&#128161; ESP32 无极调光器</h2>
    <p>拖动滑动条调节LED亮度</p>
    <input type="range" id="slider" min="0" max="255" value="0"
           oninput="updateSlider(this.value)">
    <br>
    <span class="value" id="val">0</span>
  </div>

  <script>
    function updateSlider(val) {
      document.getElementById('val').textContent = val;
      // 通过 GET 请求将数值发送给 ESP32
      fetch('/set?duty=' + val)
        .then(r => r.text())
        .then(t => console.log('OK:', t))
        .catch(e => console.error('Error:', e));
    }

    // 页面加载后获取一次当前值
    fetch('/get')
      .then(r => r.text())
      .then(v => {
        document.getElementById('slider').value = v;
        document.getElementById('val').textContent = v;
      });
  </script>
</body>
</html>
)rawliteral";

// 当前占空比
int currentDuty = 0;

// ========== 路由处理 ==========
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", htmlPage);
}

void handleSet() {
  if (server.hasArg("duty")) {
    int duty = server.arg("duty").toInt();
    duty = constrain(duty, 0, 255);
    currentDuty = duty;
    ledcWrite(ledPin, duty);
    server.send(200, "text/plain", String(duty));
    Serial.print("PWM → "); Serial.println(duty);
  } else {
    server.send(400, "text/plain", "Missing 'duty' parameter");
  }
}

void handleGet() {
  server.send(200, "text/plain", String(currentDuty));
}

void setup() {
  Serial.begin(115200);
  while (!Serial);     // 等待USB串口就绪 (C3/S3必需)
  delay(300);

  // PWM 初始化
  ledcAttach(ledPin, PWM_FREQ, PWM_RES);
  ledcWrite(ledPin, 0);

  // 开启 AP 热点
  Serial.print("正在开启热点: ");
  Serial.println(ap_ssid);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("热点已开启！IP: ");
  Serial.println(WiFi.softAPIP());

  // Web Server 路由
  server.on("/",      handleRoot);
  server.on("/set",   handleSet);
  server.on("/get",   handleGet);
  server.begin();

  Serial.println("Web Server 已启动");
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
  delay(2);
}
