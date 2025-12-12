#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h> // 請在 Arduino IDE 庫管理員安裝 ArduinoJson

// --- Wi-Fi 設定 ---
const char* ssid = "Smart_AC";
const char* password = "11111111";

// 建立 Web Server 物件，使用 Port 80
AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  // 1. 初始化 LittleFS 檔案系統
  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed");
    return;
  }
  Serial.println("LittleFS Mounted Successfully");

  // *** 改成 AP 模式 (建立熱點) ***
  WiFi.softAP(ssid, password); // 使用 softAP 建立熱點
  
  Serial.println("\nAP Started!");
  Serial.print("IP Address: http://");
  Serial.println(WiFi.softAPIP()); // 預設通常是 192.168.4.1

  // 3. 配置靜態檔案路由
  // 這行代碼非常強大，它會自動處理 index.html, .css, .js 的請求
  // 只要瀏覽器請求 "/" 或 "/index-Cl9UTSTF.css" 等，它會自動從 LittleFS 尋找
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  // 處理 404
  server.onNotFound([](AsyncWebServerRequest *request){
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404, "text/plain", "Not found");
    }
  });

  // 啟動伺服器
  server.begin();
}

void loop() {
  Serial.print("WebUI is Running......");
  delay(5000);
  
}