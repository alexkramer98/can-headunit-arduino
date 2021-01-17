#include <SPI.h>
#include "mcp2515_can.h"
#include <math.h>

const int SPI_CS_PIN = 10;
mcp2515_can CAN(SPI_CS_PIN);

int lastCanActive = 0;
bool isCanDead = true;

void setup() {
  Serial.begin(115200);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  initCan();
}

void initCan() {
  while (CAN_OK != CAN.begin(CAN_50KBPS)) {}
  delay(2000);
}

void parseCan() {
  unsigned char len = 0;
  unsigned char buf[8];

  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    lastCanActive = millis();
    if (isCanDead) {
      digitalWrite(3, HIGH);
      digitalWrite(4, HIGH);
      Serial.println("BOOT RPI!");
    }
    isCanDead = false;

    CAN.readMsgBuf(&len, buf);
    unsigned long canId = CAN.getCanId();

    switch(canId) {
      case 0x3C3:
        Serial.print("brightness:");
        Serial.println(round(buf[2] / 112 * 255));
        Serial.print("dayTime:");
        Serial.println(buf[5] == 16);
        break;
      case 0x380:
        Serial.print("inReverse:");
        Serial.println(buf[2] == 76);
        break;
      case 0x3C4:
        String activeButton;
        switch (buf[0]) {
          case 128:
            activeButton = "volUp";
            break;
          case 64:
            activeButton = "volDown";
            break;
          case 32:
            activeButton = "mute";
            break;
          case 16:
            activeButton = "nextTrack";
            break;
          case 8:
            activeButton = "previousTrack";
            break;
          case 4:
            activeButton = "source";
        }
        if (activeButton == NULL) {
          switch(buf[1]) {
            case 128:
              activeButton = "phone";
              break;
            case 64:
              activeButton = "voice";
              break;
            case 2:
              activeButton = "up";
              break;
            case 1:
              activeButton = "down";
          }
        }
        if (activeButton != NULL) {
          Serial.print("activeSwcButton:");
          Serial.println(activeButton);
        }
    }
  } else {
    if (millis() - lastCanActive >= 1000 && !isCanDead) {
      digitalWrite(3, LOW);
      digitalWrite(4, LOW);
      Serial.println("KILL RPI");
      isCanDead = true;
    }
  }
}

void loop() {
  parseCan();
}