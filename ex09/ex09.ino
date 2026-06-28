#include <WiFi.h>
#include <WebServer.h>

// ========== WiFi AP 配置 (ESP32 自己开热点) ==========
const char* ap_ssid     = "ESP32_Dashboard";
const char* ap_password = "12345678";   // 至少8位，手机连接时输入

WebServer server(80);

const int touchPin = T0;

// ========== HTML 仪表盘页面 ==========
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 传感器仪表盘</title>
  <style>
    body {
      font-family: 'Segoe UI', Arial, sans-serif;
      text-align: center;
      margin: 0;
      padding-top: 40px;
      background: #0d1117;
      color: #c9d1d9;
    }
    h1 { color: #58a6ff; font-size: 28px; margin-bottom: 30px; }
    .dashboard {
      background: #161b22;
      border: 1px solid #30363d;
      border-radius: 20px;
      padding: 40px 20px;
      max-width: 420px;
      margin: 0 auto;
      box-shadow: 0 8px 32px rgba(0,0,0,0.5);
    }
    .label {
      font-size: 16px;
      color: #8b949e;
      text-transform: uppercase;
      letter-spacing: 2px;
      margin-bottom: 10px;
    }
    .value {
      font-size: 96px;
      font-weight: bold;
      font-family: 'Courier New', monospace;
      margin: 10px 0;
      transition: color 0.3s;
    }
    .bar-bg {
      background: #21262d;
      border-radius: 10px;
      height: 24px;
      width: 100%;
      margin: 20px 0;
      overflow: hidden;
    }
    .bar-fill {
      height: 100%;
      border-radius: 10px;
      background: linear-gradient(90deg, #238636, #58a6ff);
      transition: width 0.2s ease;
    }
    .status-text {
      font-size: 18px;
      margin-top: 8px;
      color: #8b949e;
    }
    .footer {
      margin-top: 24px;
      font-size: 13px;
      color: #484f58;
    }
  </style>
</head>
<body>
  <div class="dashboard">
    <h1>&#128300; 触摸传感器仪表盘</h1>
    <div class="label">Touch Raw Value (T0)</div>
    <div class="value" id="sensorValue">--</div>
    <div class="bar-bg">
      <div class="bar-fill" id="bar" style="width: 0%;"></div>
    </div>
    <div class="status-text" id="statusText">等待数据...</div>
    <div class="footer">数据每 200ms 自动刷新</div>
  </div>

  <script>
    const MAX_RAW = 120;  // 触摸传感器典型无触摸值上限（可据实际调整）

    function fetchData() {
      fetch('/data')
        .then(r => r.text())
        .then(rawStr => {
          const raw = parseInt(rawStr, 10);
          document.getElementById('sensorValue').textContent = raw;

          // 进度条：raw 越小 = 触摸越近 = 进度条越满
          const pct = Math.max(0, Math.min(100, ((MAX_RAW - raw) / MAX_RAW) * 100));
          document.getElementById('bar').style.width = pct.toFixed(1) + '%';

          // 根据数值变色
          const valEl = document.getElementById('sensorValue');
          const stEl   = document.getElementById('statusText');
          if (raw < 20) {
            valEl.style.color = '#f85149';  // 红色：被触摸
            stEl.textContent = '⚠ 检测到触摸！';
          } else if (raw < 40) {
            valEl.style.color = '#d2991d';  // 橙色：手很近
            stEl.textContent = '手非常接近...';
          } else if (raw < 70) {
            valEl.style.color = '#58a6ff';  // 蓝色：手靠近
            stEl.textContent = '检测到接近';
          } else {
            valEl.style.color = '#3fb950';  // 绿色：无触摸
            stEl.textContent = '正常（无触摸）';
          }
        })
        .catch(e => {
          document.getElementById('statusText').textContent = '连接失败...';
        });
    }

    // 初始获取
    fetchData();
    // 每 200ms 刷新
    setInterval(fetchData, 200);
  </script>
</body>
</html>
)rawliteral";

// ========== 路由处理 ==========
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", htmlPage);
}

void handleData() {
  // 返回触摸传感器的原始模拟量数值
  int raw = touchRead(touchPin);
  server.send(200, "text/plain", String(raw));
}

void setup() {
  Serial.begin(115200);
  while (!Serial);     // 等待USB串口就绪 (C3/S3必需)
  delay(300);

  // 开启 AP 热点
  Serial.print("正在开启热点: ");
  Serial.println(ap_ssid);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.print("热点已开启！IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/",     handleRoot);
  server.on("/data", handleData);
  server.begin();

  Serial.println("传感器仪表盘 Web Server 已启动");
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
