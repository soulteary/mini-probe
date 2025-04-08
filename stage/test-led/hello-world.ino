#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Initialize LCD at address 0x27, size 20x4
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  lcd.init();       // Initialize LCD
  lcd.backlight();  // Turn on backlight

  // Display A Poem
  lcd.setCursor(2, 0);
  lcd.print("The Road Not Taken");

  lcd.setCursor(0, 1);
  lcd.print("Two roads diverged");

  lcd.setCursor(0, 2);
  lcd.print("in a yellow wood,");

  lcd.setCursor(0, 3);
  lcd.print("- Robert Frost -");
}

void loop() {
  // Empty loop
}
