// C. Näveke's XPT2046 implementation adapted to output Adafruit TSPoint

#pragma once

#include "Arduino.h"
#include "TouchScreen.h"

#define CMD_READ_X   0x91 // Command for XPT2046 to read X position
#define CMD_READ_Y   0xD1 // Command for XPT2046 to read Y position
#define CMD_READ_Z1  0xB1 // Command for XPT2046 to read Z1 position
#define CMD_READ_Z2  0xC1 // Command for XPT2046 to read Z2 position

#define DELAY 5

class XPT2046_Bitbang {
    public:
        XPT2046_Bitbang(uint8_t mosiPin, uint8_t misoPin, uint8_t clkPin, uint8_t csPin);
        void beginXPT();
        TSPoint getPoint();

    private:
        uint8_t _mosiPin;
        uint8_t _misoPin;
        uint8_t _clkPin;
        uint8_t _csPin;
        void writeSPI(byte command);
        uint16_t readSPI(byte command);
};

