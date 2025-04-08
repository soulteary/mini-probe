#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* headers[5] = {
  "UPTIME",
  "RAM",
  "NET(U/D)",
  "DISK(R/W)",
  "NOW"
};

String contents[5] = {
  "waiting...",
  "waiting...",
  "waiting...",
  "waiting...",
  "00:00:00"
};

byte scrollPos[5] = { 0, 0, 0, 0, 0 };
const byte lineLength = 20;
const unsigned long scrollDelay = 300;
const unsigned long displayInterval = 2000; // 2秒切换间隔
unsigned long previousMillis = 0;
unsigned long lastSwitchMillis = 0;
unsigned long lastTimeUpdateMillis = 0;
const unsigned long timeUpdateInterval = 1000;  // 1秒
byte currentDisplay = 0;

unsigned long internalTimeMillis = 0;
unsigned long lastInternalMillis = 0;

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("=[Mini Probe]= v0.1");
}

void updateInternalTime() {
  unsigned long currentMillis = millis();
  internalTimeMillis += currentMillis - lastInternalMillis;
  lastInternalMillis = currentMillis;

  unsigned long totalSeconds = internalTimeMillis / 1000;
  unsigned int hours = (totalSeconds / 3600) % 24;
  unsigned int minutes = (totalSeconds / 60) % 60;
  unsigned int seconds = totalSeconds % 60;

  char buf[9];
  sprintf(buf, "%02u:%02u:%02u", hours, minutes, seconds);
  contents[4] = String(buf);
}

void loop() {
  unsigned long currentMillis = millis();

  if (Serial.available() > 0) {
    String received = Serial.readStringUntil('\n');
    received.trim();
    int sepIndex = received.indexOf(':');

    if (sepIndex > 0) {
      String header = received.substring(0, sepIndex);
      String data = received.substring(sepIndex + 1);
      header.trim();
      data.trim();

      for (byte i = 0; i < 5; i++) {
        if (header.equalsIgnoreCase(headers[i])) {
          contents[i] = data;
          scrollPos[i] = 0;
          if (i == 4) { // NOW received, update internal time
            int h, m, s;
            if (sscanf(data.c_str(), "%d:%d:%d", &h, &m, &s) == 3) {
              internalTimeMillis = ((unsigned long)h * 3600UL + m * 60UL + s) * 1000UL;
              lastInternalMillis = currentMillis;
            }
          }
          break;
        }
      }
    }
  }

  updateInternalTime();

  // 每2秒切换显示内容，排除NOW（最后一项）
  if (currentMillis - lastSwitchMillis >= displayInterval) {
    lastSwitchMillis = currentMillis;
    currentDisplay = (currentDisplay + 1) % 4;

    lcd.setCursor(0, 1);
    lcd.print("                    "); // 清除上一标题
    lcd.setCursor(0, 1);
    lcd.print(headers[currentDisplay]);
    lcd.print(":");

    scrollPos[currentDisplay] = 0; // Reset scroll position each cycle

    lcd.setCursor(0, 2);
    lcd.print("                    "); // 清除上一内容
  }

  // 滚动显示内容
  if (currentMillis - previousMillis >= scrollDelay) {
    previousMillis = currentMillis;

    lcd.setCursor(0, 2);

    byte contentLen = contents[currentDisplay].length();
    if (contentLen > lineLength) {
      for (byte j = 0; j < lineLength; j++) {
        byte charIndex = (scrollPos[currentDisplay] + j) % contentLen;
        lcd.print(contents[currentDisplay][charIndex]);
      }
      scrollPos[currentDisplay] = (scrollPos[currentDisplay] + 1) % contentLen;
    } else {
      lcd.print(contents[currentDisplay]);
      for (byte j = contentLen; j < lineLength; j++) {
        lcd.print(" ");
      }
    }
  }

  // 每秒刷新NOW时间显示
  if (currentMillis - lastTimeUpdateMillis >= timeUpdateInterval) {
    lastTimeUpdateMillis = currentMillis;
    lcd.setCursor(0, 3);
    lcd.print("                    "); // 清除上一内容
    lcd.setCursor(20 - contents[4].length(), 3);
    lcd.print(contents[4]);
  }
}
