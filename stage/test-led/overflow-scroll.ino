#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* lines[4] = {
  "Stopping by Woods",
  "on a Snowy Evening",
  "Whose woods these are",
  "I think I know."
};

byte scrollPos[4] = { 0, 0, 0, 0 };
const byte lineLength = 20;
const unsigned long scrollDelay = 300;
unsigned long previousMillis = 0;

void setup() {
  lcd.init();
  lcd.backlight();

  for (byte i = 0; i < 4; i++) {
    lcd.setCursor(0, i);
    lcd.print(lines[i]);
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= scrollDelay) {
    previousMillis = currentMillis;

    for (byte i = 0; i < 4; i++) {
      lcd.setCursor(0, i);
      byte lineLen = strlen(lines[i]);

      if (lineLen > lineLength) {
        for (byte j = 0; j < lineLength; j++) {
          byte charIndex = (scrollPos[i] + j) % lineLen;
          lcd.print(lines[i][charIndex]);
        }
        scrollPos[i] = (scrollPos[i] + 1) % lineLen;
      } else {
        lcd.print(lines[i]);
        for (byte j = lineLen; j < lineLength; j++) {
          lcd.print(" ");
        }
      }
    }
  }
}