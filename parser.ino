#include <SPI.h>
#include "mcp2515_can.h"
#include <math.h>

const int SPI_CS_PIN = 10;
mcp2515_can CAN(SPI_CS_PIN);

boolean canDead = false;

void setup() {
  Serial.begin(115200);
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
    if (canDead) {
      Serial.println("BOOTING RPI!!");
    }
    canDead = false;
    CAN.readMsgBuf(&len, buf);
    unsigned long canId = CAN.getCanId();

    switch(canId) {
      case 0x3C3:
        Serial.print(">brightness:");
        Serial.print(round(buf[2] / 112 * 100));
        Serial.print(">dayTime:");
        Serial.print(buf[5] == 16);
        break;
      case 0x380:
        Serial.print(">inReverse:");
        Serial.print(buf[2] == 76);
        break;
      case 0x3C4:
        Serial.print(">activeSwcButton:");
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
        if (!activeButton) {
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
        Serial.print(activeButton);
    }
  }
  else {
    if (!canDead) {
      Serial.println("CAN DEAD! SHUTTING DOWN RPI!");
    }
    canDead = true;
  }
}

void loop() {
  parseCan();
}