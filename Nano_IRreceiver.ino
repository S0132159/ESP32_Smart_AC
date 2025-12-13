#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// ---【關鍵修改】加大緩衝區 ---
// 必須在 include IRremote 之前定義
// Nano 記憶體有限，若編譯時提示記憶體不足，請將 600 改為 400
#if !defined(RAW_BUFFER_LENGTH)
#define RAW_BUFFER_LENGTH 600 
#endif

#include <IRremote.h>

#define IR_RECEIVE_PIN 2
LiquidCrystal_I2C lcd(0x27, 16, 2); 

String lcdLine1 = "Waiting...";
String lcdLine2 = "IR Signal";

void updateDisplay();
void translateSignalToLCD();

void setup() {
  Serial.begin(115200); 
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("AC IR Analyzer");
  lcd.setCursor(0, 1);
  lcd.print("Buffer: 600"); 

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  
  Serial.println(F("STARTING IR DECODE..."));
  Serial.println(F("Buffer size increased to 600 to handle AC signals."));
  Serial.println(F("Waiting for signals..."));
}

void loop() {
  if (IrReceiver.decode()) {
    
    Serial.println(F("------------------------------------------------"));
    
    // 檢查是否溢位 (Overflow)
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
       Serial.println(F("Error: Overflow! Signal too long for buffer."));
       Serial.println(F("Try increasing RAW_BUFFER_LENGTH if possible."));
       lcdLine1 = "Error:";
       lcdLine2 = "Overflow!";
    } else {
       // 這個函式會自動印出 Protocol, Command, Address 以及 Raw Data
       // 我們依賴它來顯示所有資訊，不需要自己去存取 rawDataPtr
       IrReceiver.printIRResultShort(&Serial); 
       
       if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
          Serial.println(F("Protocol is UNKNOWN. Check the Raw Data above."));
       }
       
       translateSignalToLCD();
    }

    updateDisplay();
    IrReceiver.resume(); 
  }
}

void translateSignalToLCD() {
  // 檢查是否溢位
  if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW) {
    return; 
  }

  unsigned long cmd = IrReceiver.decodedIRData.command;
  auto protocol = IrReceiver.decodedIRData.protocol;

  lcdLine1 = "Recv: ";
  
  if (protocol == UNKNOWN) {
     lcdLine1 += "Unk";
     // 用 numberOfBits 來顯示收到的訊號長度
     lcdLine2 = "Bits:" + String(IrReceiver.decodedIRData.numberOfBits);
     return;
  }

  lcdLine1 += String(getProtocolString(protocol));

  // --- 自定義翻譯區域 ---
  // 這裡填入你在 Serial Monitor 觀察到的 Command
  
  if (cmd == 0x1234ABCD) { 
      lcdLine2 = "COOL 26C";
  } 
  else {
      // 如果還沒定義，顯示 Hex 碼
      lcdLine2 = "Hex:" + String(cmd, HEX);
  }
}

void updateDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lcdLine1);
  lcd.setCursor(0, 1);
  lcd.print(lcdLine2);
}