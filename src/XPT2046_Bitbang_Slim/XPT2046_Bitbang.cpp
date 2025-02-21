// C. Näveke's XPT2046 implementation adapted to output Adafruit TSPoint

#include "XPT2046_Bitbang.h"

XPT2046_Bitbang::XPT2046_Bitbang(uint8_t mosiPin, uint8_t misoPin, uint8_t clkPin, uint8_t csPin) : 
                                        _mosiPin(mosiPin), _misoPin(misoPin), _clkPin(clkPin), _csPin(csPin)
{
  beginXPT();
}

void XPT2046_Bitbang::beginXPT()
{
    pinMode(_mosiPin, OUTPUT);
    pinMode(_misoPin, INPUT);
    pinMode(_clkPin, OUTPUT);
    pinMode(_csPin, OUTPUT);
    digitalWrite(_csPin, HIGH);
    digitalWrite(_clkPin, LOW);
}

void XPT2046_Bitbang::writeSPI(byte command)
{
    for(int i = 7; i >= 0; i--)
    {
        digitalWrite(_mosiPin, command & (1 << i));
        digitalWrite(_clkPin, LOW);
        delayMicroseconds(DELAY);
        digitalWrite(_clkPin, HIGH);
        delayMicroseconds(DELAY);
    }
    digitalWrite(_mosiPin, LOW);
    digitalWrite(_clkPin, LOW);
}

uint16_t XPT2046_Bitbang::readSPI(byte command)
{
    writeSPI(command);

    uint16_t result = 0;

    for(int i = 15; i >= 0; i--)
    {
        digitalWrite(_clkPin, HIGH);
        delayMicroseconds(DELAY);
        digitalWrite(_clkPin, LOW);
        delayMicroseconds(DELAY);
        result |= (digitalRead(_misoPin) << i);
    }

    return result >> 4;
}

TSPoint XPT2046_Bitbang::getPoint()
{
    digitalWrite(_csPin, LOW);

    uint16_t z1 = readSPI(CMD_READ_Z1);
    uint16_t z = z1 + 4095;
    uint16_t z2 = readSPI(CMD_READ_Z2);
    z -= z2;

    if(z < 100)
    {
        return TSPoint{0, 0, 0};
    }

    // convert to Adafruit TSPoint (0..1023)
    uint16_t xRaw = readSPI(CMD_READ_X);
    uint16_t yRaw = 4095 - readSPI(CMD_READ_Y & ~((byte)1));
    digitalWrite(_csPin, HIGH);
    uint16_t y = map(xRaw, 0, 4095, 0, 1023);
    uint16_t x = map(yRaw, 0, 4095, 0, 1023);
        
    return TSPoint{x, y, z};
}
