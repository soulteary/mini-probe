#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* headers[4] = {
  "UPTIME",
  "RAM",
  "NET(U/D)",
  "DISK(R/W)"
};

String contents[4] = {
  "waiting...",
  "waiting...",
  "waiting...",
  "waiting..."
};

byte scrollPos[4] = { 0, 0, 0, 0 };
const byte lineLength = 20;
const unsigned long scrollDelay = 300;
unsigned long previousMillis = 0;

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  for (byte i = 0; i < 4; i++) {
    lcd.setCursor(0, i);
    lcd.print(headers[i]);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (Serial.available() > 0) {
    String received = Serial.readStringUntil('\n');
    received.trim();  // 移除首尾空格及换行符
    int sepIndex = received.indexOf(':');

    if (sepIndex > 0) {
      String header = received.substring(0, sepIndex);
      String data = received.substring(sepIndex + 1);
      header.trim();
      data.trim();

      for (byte i = 0; i < 4; i++) {
        if (header.equalsIgnoreCase(headers[i])) {
          contents[i] = data;
          scrollPos[i] = 0;
        }
      }
    }
  }

  if (currentMillis - previousMillis >= scrollDelay) {
    previousMillis = currentMillis;

    for (byte i = 0; i < 4; i++) {
      lcd.setCursor(strlen(headers[i]) + 1, i);
      byte contentLen = contents[i].length();
      byte availableSpace = lineLength - (strlen(headers[i]) + 1);

      if (contentLen > availableSpace) {
        for (byte j = 0; j < availableSpace; j++) {
          byte charIndex = (scrollPos[i] + j) % contentLen;
          lcd.print(contents[i][charIndex]);
        }
        scrollPos[i] = (scrollPos[i] + 1) % contentLen;
      } else {
        lcd.print(contents[i]);
        for (byte j = contentLen; j < availableSpace; j++) {
          lcd.print(" ");
        }
      }
    }
  }
}
