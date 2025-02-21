// PMD32-Mega2560 (c) 2025 J. Bogin, https://boginjr.com
// Based on PMD32-SD (c) 2012 R. Borik, https://pmd85.borik.net/
// Touchscreen calibration
// Adapted from TouchScreen_Calibr_native (c) D. Prentice

#include "config.h"

#ifdef TOUCH_SCREEN_CALIBRATION

MCUFRIEND_kbv tft;
Touch ts;
TSPoint tp;

#define WHITE 0xFFFF
#define RED   0xF800
#define BLUE  0x001F
#define BLACK 0x0000

#define TITLE "Touch screen calibration"

bool ISPRESSED(void)
{
    int count = 0;
    bool state, oldstate;
    while (count < 10)
    {
        tp = ts.getPointRaw();
        state = tp.z > 20;
        if (state == oldstate) count++;
        else count = 0;
        oldstate = state;
        delay(1);
    }
    return oldstate;
}

uint32_t cx, cy, cz;
uint32_t rx[8], ry[8];
int32_t clx, crx, cty, cby;
float px, py;
int dispx, dispy, text_y_center, swapxy;
uint32_t calx, caly, cals;

void centerprint(const char *s, int y)
{
    int len = strlen(s) * 8;
    tft.fillRect((dispx - len) / 2, y-10, len, 12, RED);
    tft.setCursor((dispx - len) / 2, y);
    tft.print(s);
}

void centertitle(const char *s)
{
    tft.fillScreen(BLACK);
    tft.fillRect(0, 0, dispx, 24, RED);
    tft.fillRect(0, 24, dispx, 1, WHITE);
    centerprint(s, 16);
    tft.setCursor(0, 30);
    tft.setTextColor(WHITE, BLACK);
}

void startup()
{
    centertitle(TITLE);

    tft.println(F("\nUse a stylus or something similar"));
    tft.println(F("to touch, as close to the center,"));
    tft.println(F("each WHITE crosshair.\n"));    
    tft.println(F("Keep holding until it turns RED !"));
    tft.println(F("Repeat for all crosshairs.\n"));
    tft.println(F("Touch screen to continue"));

    while (ISPRESSED() == false) {}
    while (ISPRESSED() == true) {}
}

void fail()
{
    centertitle("Touch Calibration FAILED");

    tft.println(F("\nUnable to read the position of"));
    tft.println(F("the press; this is a HW issue.\n"));
#ifndef TOUCH_SCREEN_ACTIVE
    tft.println(F("Check resistance between pins:"));
    tft.println(F("XP <-> XM; YP <-> YM"));    
    tft.println(F("should be about 300 ohms."));
#endif

    while (true) {};
}

void drawCrossHair(int x, int y, uint16_t color)
{
    tft.drawRect(x - 10, y - 10, 20, 20, color);
    tft.drawLine(x - 5, y, x + 5, y, color);
    tft.drawLine(x, y - 5, x, y + 5, color);
}

void readCoordinates()
{
    int iter = 5000;
    int failcount = 0;
    int cnt = 0;
    uint32_t tx = 0;
    uint32_t ty = 0;
    uint32_t tz = 0;
    bool OK = false;

    while (OK == false)
    {
        centerprint("*  PRESS  *", text_y_center);
        while (ISPRESSED() == false) {}
        centerprint("*  HOLD!  *", text_y_center);
        cnt = 0;
        iter = 400;
        do
        {
            tp = ts.getPointRaw();
            if (tp.z > 200)  //.kbv
            {
                tx += tp.x;
                ty += tp.y;
                tz += tp.z;
                cnt++;
            }
            else
                failcount++;
        } while ((cnt < iter) && (failcount < 10000));
        if (cnt >= iter)
        {
            OK = true;
        }
        else
        {
            tx = 0;
            ty = 0;
            tz = 0;
            cnt = 0;
        }
        if (failcount >= 10000)
            fail();
    }

    cx = tx / iter;
    cy = ty / iter;
    cz = tz / iter;
}

void calibrate(int x, int y, int i)
{
    drawCrossHair(x, y, WHITE);
    readCoordinates();
    centerprint("* RELEASE *", text_y_center);
    drawCrossHair(x, y, RED);
    rx[i] = cx;
    ry[i] = cy;
    while (ISPRESSED() == true) {}
}

void report()
{
    uint16_t CAL_X1, CAL_Y1, CAL_X2, CAL_Y2;
    char buf[60];
    centertitle(TITLE);

    tft.println(F("\nCalibration done!\n"));
    tft.println(F("Open \"config.h\", find these defines"));
    tft.println(F("and set them to the following values:\n"));   
    
    CAL_X1  = (calx >> 14) & 0x3FFF;
    CAL_Y1  = (caly >> 14) & 0x3FFF;    
    CAL_X2  = (calx >>  0) & 0x3FFF;
    CAL_Y2  = (caly >>  0) & 0x3FFF;    
    
    sprintf(buf, "#define TOUCH_CAL_X1 %d", CAL_X1);
    tft.println(buf);
    sprintf(buf, "#define TOUCH_CAL_Y1 %d", CAL_Y1);
    tft.println(buf);
    sprintf(buf, "#define TOUCH_CAL_X2 %d", CAL_X2);
    tft.println(buf);
    sprintf(buf, "#define TOUCH_CAL_Y2 %d", CAL_Y2);
    tft.println(buf);
}

void setup()
{
#ifdef USE_MEGA_16BIT_SHIELD
    uint16_t ID = DISP_ID_16BIT;
#else
    uint16_t ID = tft.readID();
#endif
    tft.begin(ID);
    tft.setRotation(1);
    tft.setFont(&Progmem::m_vgaFont);
    dispx = DISP_WIDTH;
    dispy = DISP_HEIGHT;
    text_y_center = (dispy / 2);
}

void loop()
{
    startup();

    int x, y, cnt, idx = 0;
    tft.fillScreen(BLACK);
    for (x = 10, cnt = 0; x < dispx; x += (dispx - 20) / 2) {
        for (y = 10; y < dispy; y += (dispy - 20) / 2) {
            if (++cnt != 5) drawCrossHair(x, y, BLUE);
        }
    }
    centerprint("***********", text_y_center - 12);
    centerprint("***********", text_y_center + 12);
    for (x = 10, cnt = 0; x < dispx; x += (dispx - 20) / 2) {
        for (y = 10; y < dispy; y += (dispy - 20) / 2) {
            if (++cnt != 5) calibrate(x, y, idx++);
        }
    }

    cals = (long(dispx - 1) << 12) + (dispy - 1);
    swapxy = rx[2] - rx[0];
    swapxy = (swapxy < -400 || swapxy > 400);
    if (swapxy != 0) {
        clx = (ry[0] + ry[1] + ry[2]); //rotate 90
        crx = (ry[5] + ry[6] + ry[7]);
        cty = (rx[0] + rx[3] + rx[5]);
        cby = (rx[2] + rx[4] + rx[7]);
    } else {
        clx = (rx[0] + rx[1] + rx[2]); //regular
        crx = (rx[5] + rx[6] + rx[7]);
        cty = (ry[0] + ry[3] + ry[5]);
        cby = (ry[2] + ry[4] + ry[7]);
    }
    clx /= 3;
    crx /= 3;
    cty /= 3;
    cby /= 3;
    px = float(crx - clx) / (dispx - 20);
    py = float(cby - cty) / (dispy - 20);
    //  px = 0;
    clx -= px * 10;
    crx += px * 10;
    cty -= py * 10;
    cby += py * 10;

    calx = (long(clx) << 14) + long(crx);
    caly = (long(cty) << 14) + long(cby);
    if (swapxy)
        cals |= (1L << 31);

    report();          // report results
    while (true) {}    // tread water
}

#endif // TOUCH_SCREEN_CALIBRATION
