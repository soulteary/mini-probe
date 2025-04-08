#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* headers[4] = {
  "UPDATE:",
  "RAM:",
  "NET(U/D):",
  "DISK(R/W):"
};

const char* contents[4] = {
  "428h8m0s",
  "83.76% (20.1GB/24.0GB)",
  "101.4GB / 173.6GB",
  "3.1TB / 965.3GB"
};

byte scrollPos[4] = { 0, 0, 0, 0 };
const byte lineLength = 20;
const unsigned long scrollDelay = 500;
unsigned long previousMillis = 0;

void setup() {
  lcd.init();
  lcd.backlight();

  for (byte i = 0; i < 4; i++) {
    lcd.setCursor(0, i);
    lcd.print(headers[i]);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= scrollDelay) {
    previousMillis = currentMillis;

    for (byte i = 0; i < 4; i++) {
      lcd.setCursor(strlen(headers[i]) + 1, i);
      byte contentLen = strlen(contents[i]);
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
