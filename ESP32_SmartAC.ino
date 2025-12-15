#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// --- 硬體與網路設定 ---
const char* ssid = "Smart_AC_Host";
const char* password = "12345678";

//定義dht11,IR腳位
#define DHTPIN 4
#define DHTTYPE DHT11
#define IR_LED_PIN 14 

DHT dht(DHTPIN, DHTTYPE);
IRsend irsend(IR_LED_PIN);
AsyncWebServer server(80);

// --- 系統狀態結構 ---
struct ACState {
  bool power;           
  int targetTemp;       
  float currentTemp;    
  float humidity;       
  String fanSpeed;      // low, medium, high, auto
  String mode;          // cool, heat, fan, dry
  
  // 進階功能狀態
  bool sleepMode;       
  unsigned long sleepStartTime; 
  int sleepStartTemp;   
  bool timerActive;     
  unsigned long timerEndTime; 
  String timerEndAction; 
  bool autoTempMode;    
  unsigned long lastTempCheck; 
};

// 初始化狀態
ACState ac = {false, 26, NAN, NAN, "auto", "cool", false, 0, 26, false, 0, "off", false, 0};

// --- 統一的關機重置函式 ---
// 這個函式專門用來處理「關機 + 清除排程」
void executePowerOff() {
  ac.power = false;
  
  // *** 核心修改：關機時，將所有排程歸零 ***
  ac.sleepMode = false;    // 取消舒眠
  ac.timerActive = false;  // 取消定時
  ac.autoTempMode = false; // 取消恆溫監控
  
  // 發送紅外線關機訊號
  // irsend.sendPanasonic(address, code, bits); // 請換成你的紅外線碼
  Serial.println("[系統動作] 執行強制關機，並重置所有排程。");
}

// --- 狀態回報函式 ---
void printStatus(String commandReceived) {
  Serial.println("\n========== [收到指令: " + commandReceived + "] ==========");
  Serial.printf("電源狀態: %s\n", ac.power ? "開機 (ON)" : "關機 (OFF)");
  Serial.printf("運作模式: %s\n", ac.mode.c_str());
  Serial.printf("風速設定: %s\n", ac.fanSpeed.c_str());
  Serial.printf("目標溫度: %d °C\n", ac.targetTemp);
  
  // 處理感測器數據可能是 NaN 的情況
  String curTempStr = isnan(ac.currentTemp) ? "讀取中..." : String(ac.currentTemp, 1);
  String curHumStr = isnan(ac.humidity) ? "讀取中..." : String(ac.humidity, 0);
  Serial.println("環境數值: 室溫 " + curTempStr + "°C / 濕度 " + curHumStr + "%");

  Serial.println("--- 進階功能 ---");
  Serial.printf("舒眠模式: %s\n", ac.sleepMode ? "啟動中" : "關閉");
  Serial.printf("恆溫監控: %s\n", ac.autoTempMode ? "啟動中" : "關閉");
  
  if (ac.timerActive) {
    long remaining = (long)(ac.timerEndTime - millis()) / 60000;
    Serial.printf("定時設定: 啟動 (剩餘 %ld 分鐘後執行 %s)\n", remaining, ac.timerEndAction.c_str());
  } else {
    Serial.println("定時設定: 無");
  }
  Serial.println("================================================\n");
}

// --- 紅外線發射 (模擬) ---
void sendACCommand() {
  // 這裡之後要填入真實的紅外線編碼
  // 目前僅作 Serial 輸出證明邏輯有跑到這裡
  // Serial.println("[IR] 發射紅外線訊號..."); 
}

// --- 邏輯更新 ---
void updateACLogic() {
  unsigned long now = millis();
  
  // 讀取感測器 (每2秒)
  static unsigned long lastSensorRead = 0;
  if (now - lastSensorRead > 2000) {
    lastSensorRead = now;
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t)) ac.currentTemp = t;
    if (!isnan(h)) ac.humidity = h;
  }

// 如果已經關機，就不執行後面的控制邏輯
  if (!ac.power) return;

  // 舒眠邏輯
  if (ac.sleepMode) {
    long elapsed = now - ac.sleepStartTime;
    int newTarget = ac.sleepStartTemp;
    if (elapsed > 7200000) newTarget = 29;
    else if (elapsed > 3600000) newTarget = ac.sleepStartTemp + 1;

    if (newTarget != ac.targetTemp && newTarget <= 29) {
      ac.targetTemp = newTarget;
      sendACCommand();
      printStatus("系統自動調整(舒眠)"); // 自動調整時也印出狀態
    }
  }

  // 定時邏輯
  if (ac.timerActive && now >= ac.timerEndTime) {
    // 時間到，先關閉定時器標記
    ac.timerActive = false;

    if (ac.timerEndAction == "off") {
      executePowerOff();
      printStatus("系統自動執行(定時關機)");
    } 
    else if (ac.timerEndAction == "sleep") {
      ac.sleepMode = true;
      ac.sleepStartTime = millis();
      ac.sleepStartTemp = ac.targetTemp;
      printStatus("系統自動執行(定時轉舒眠)");
      sendACCommand();
    }
    
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  irsend.begin();
  if(!LittleFS.begin()) Serial.println("FS Error");
  
  WiFi.softAP(ssid, password);
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  // --- API 1: 獲取狀態  ---
  server.on("/api/state", HTTP_GET, [](AsyncWebServerRequest *request){
    JsonDocument doc;
    doc["power"] = ac.power;
    doc["currentTemp"] = isnan(ac.currentTemp) ? "NaN" : String(ac.currentTemp, 1);
    doc["targetTemp"] = ac.targetTemp;
    doc["humidity"] = isnan(ac.humidity) ? "NaN" : String(ac.humidity, 0);
    doc["sleepMode"] = ac.sleepMode;
    doc["timerActive"] = ac.timerActive;
    doc["autoTemp"] = ac.autoTempMode;
    if (ac.timerActive) {
      doc["timerRemaining"] = (long)(ac.timerEndTime - millis()) / 60000; 
      doc["timerAction"] = ac.timerEndAction;
    } else {
      doc["timerRemaining"] = 0;
    }
    
    String res;
    serializeJson(doc, res);
    request->send(200, "application/json", res);
  });

  // --- API 2: 設定溫度 ---
  // 網址參數改為 val 以配合你的 JavaScript
  server.on("/api/setTemp", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("val")) {
      ac.targetTemp = request->getParam("val")->value().toInt();
      // 如果關機中設定溫度，是否要自動開機？通常不用，這裡只更新變數
      if(ac.power) sendACCommand(); 
      printStatus("設定溫度");
    }
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });

  // --- API 3: 電源開關 ---
  server.on("/api/power", HTTP_GET, [](AsyncWebServerRequest *request){
    // 這裡實作 Toggle (切換) 功能
    if (ac.power) {
      // 如果原本是開的 -> 現在要關機
      // *** 呼叫統一關機函式，清除所有排程 ***
      executePowerOff();
      printStatus("手動關機 (清除排程)");
    } else {
      // 如果原本是關的 -> 現在要開機
      ac.power = true;
      sendACCommand();
      printStatus("手動開機"); 
    }
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });

  // --- API 4: 定時器 ---
  server.on("/api/timer", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("mins") && request->hasParam("action")) {
      int mins = request->getParam("mins")->value().toInt();
      String action = request->getParam("action")->value();
      
      if (mins > 0) {
        ac.timerActive = true;
        ac.timerEndTime = millis() + (mins * 60000);
        ac.timerEndAction = action;
        printStatus("設定定時器");
      } else {
        ac.timerActive = false;
        printStatus("取消定時器");
      }
    }
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });

  // --- API 5: 舒眠模式 ---
  server.on("/api/sleep", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("set")) {
      String val = request->getParam("set")->value();
      if (val == "true") {
        ac.sleepMode = true;
        ac.sleepStartTime = millis();
        ac.sleepStartTemp = ac.targetTemp;
      } else {
        ac.sleepMode = false;
      }
      printStatus("切換舒眠模式");
    }
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });

  // --- API 6: 恆溫模式 ---
  server.on("/api/autoTemp", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("set")) {
      ac.autoTempMode = (request->getParam("set")->value() == "true");
      printStatus("切換恆溫監控");
    }
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });

  server.begin();
  Serial.println("Web Server Started! 請打開 Serial Monitor 查看狀態。");
}

void loop() {
  updateACLogic();
  delay(100);
}