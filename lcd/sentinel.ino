#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* headers[5] = { "UPTIME", "RAM", "NET(U/D)", "DISK(R/W)", "NOW" };
char contents[5][64] = { "waiting...", "waiting...", "waiting...", "waiting...", "00:00:00" };
char tempContents[5][64];  // 临时缓冲区
bool contentUpdated[5] = { false };

byte scrollPos[4] = { 0 };
const byte lineLength = 20;
const unsigned long scrollDelay = 500;
const unsigned long displayInterval = 4000;
unsigned long previousMillis = 0;
unsigned long lastSwitchMillis = 0;
unsigned long lastTimeUpdateMillis = 0;
unsigned long lastSyncRequestMillis = 0;
const unsigned long timeUpdateInterval = 1000;
const unsigned long timeSyncInterval = 15000;
byte currentDisplay = 0;

unsigned long internalTimeMillis = 0;
unsigned long lastInternalMillis = 0;

void safeLCDPrint(byte col, byte row, const char* msg) {
  lcd.setCursor(col, row);
  char buffer[21];
  snprintf(buffer, 21, "%-20s", msg);
  lcd.print(buffer);
}

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  delay(300);
  Serial.begin(115200);
  safeLCDPrint(0, 0, "=[Mini Probe]= v0.1");
  lastInternalMillis = millis();
  memcpy(tempContents, contents, sizeof(contents));
}

void updateInternalTime() {
  unsigned long currentMillis = millis();
  internalTimeMillis += currentMillis - lastInternalMillis;
  lastInternalMillis = currentMillis;

  unsigned long totalSeconds = internalTimeMillis / 1000;
  snprintf(contents[4], sizeof(contents[4]), "%02lu:%02lu:%02lu",
           (totalSeconds / 3600) % 24, (totalSeconds / 60) % 60, totalSeconds % 60);
}

void requestTimeSync() {
  Serial.println("SYNC_REQUEST");
}

void processSerial() {
  if (Serial.available()) {
    char buffer[80] = { 0 };
    size_t len = Serial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
    buffer[len] = '\0';

    char* sep = strchr(buffer, ':');
    if (sep != nullptr) {
      *sep = '\0';
      char* header = buffer;
      char* data = sep + 1;

      while (*header == ' ') header++;
      while (*data == ' ') data++;

      char* end;
      end = header + strlen(header) - 1;
      while (end > header && isspace(*end)) *(end--) = '\0';

      end = data + strlen(data) - 1;
      while (end > data && isspace(*end)) *(end--) = '\0';

      for (byte i = 0; i < 5; i++) {
        if (strcasecmp(header, headers[i]) == 0) {
          strncpy(tempContents[i], data, sizeof(tempContents[i]) - 1);
          tempContents[i][sizeof(tempContents[i]) - 1] = '\0';
          contentUpdated[i] = true;

          if (i == 4) {  // 立即更新时间
            int h, m, s;
            if (sscanf(data, "%d:%d:%d", &h, &m, &s) == 3) {
              internalTimeMillis = ((unsigned long)h * 3600UL + m * 60UL + s) * 1000UL;
              lastInternalMillis = millis();
              strncpy(contents[4], tempContents[4], sizeof(contents[4]));
              contentUpdated[4] = false;
            }
          }
          break;
        }
      }
    }
  }
}


void loop() {
  unsigned long currentMillis = millis();

  processSerial();
  updateInternalTime();

  if (currentMillis - lastSyncRequestMillis >= timeSyncInterval) {
    lastSyncRequestMillis = currentMillis;
    requestTimeSync();
  }

  if (currentMillis - lastSwitchMillis >= displayInterval) {
    lastSwitchMillis = currentMillis;
    currentDisplay = (currentDisplay + 1) % 4;

    if (currentDisplay == 0) {  // 完整显示一轮后才更新内容
      for (byte i = 0; i < 4; i++) {
        if (contentUpdated[i]) {
          strncpy(contents[i], tempContents[i], sizeof(contents[i]));
          contentUpdated[i] = false;
        }
      }
    }

    char headerLine[21];
    snprintf(headerLine, 21, "%s:", headers[currentDisplay]);
    safeLCDPrint(0, 1, headerLine);

    scrollPos[currentDisplay] = 0;
  }

  if (currentMillis - previousMillis >= scrollDelay) {
    previousMillis = currentMillis;

    char* content = contents[currentDisplay];
    byte contentLen = strlen(content);
    char displayBuffer[21];

    if (contentLen <= lineLength) {
      snprintf(displayBuffer, 21, "%-20s", content);
    } else {
      for (byte i = 0; i < lineLength; i++) {
        displayBuffer[i] = content[(scrollPos[currentDisplay] + i) % contentLen];
      }
      displayBuffer[lineLength] = '\0';
      scrollPos[currentDisplay] = (scrollPos[currentDisplay] + 1) % contentLen;
    }

    safeLCDPrint(0, 2, displayBuffer);
  }

  if (currentMillis - lastTimeUpdateMillis >= timeUpdateInterval) {
    lastTimeUpdateMillis = currentMillis;
    char timeBuffer[21];
    snprintf(timeBuffer, 21, "%20s", contents[4]);
    safeLCDPrint(0, 3, timeBuffer);
  }
}