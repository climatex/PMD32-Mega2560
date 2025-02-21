// PMD32-Mega2560 (c) 2025 J. Bogin, https://boginjr.com
// Based on PMD32-SD (c) 2012 R. Borik, https://pmd85.borik.net/
// Touchscreen support

#pragma once
#include "config.h"

#ifdef TOUCH_SCREEN_ACTIVE
class Touch : private XPT2046_Bitbang
#else
class Touch : private TouchScreen
#endif
{

public:

#ifdef TOUCH_SCREEN_ACTIVE
  Touch() : XPT2046_Bitbang(TOUCH_DIN, TOUCH_DOUT, TOUCH_CLK, TOUCH_CS) {};
#else
  Touch() : TouchScreen(TOUCH_XPLUS, TOUCH_YPLUS, TOUCH_XMINUS, TOUCH_YMINUS, TOUCH_RX) {};
#endif

  virtual ~Touch() {};
   
  TSPoint getPointRaw()
  {
#ifdef TOUCH_SCREEN_ACTIVE
    TSPoint result = XPT2046_Bitbang::getPoint();
#else
    TSPoint result = TouchScreen::getPoint();
  
    // these two are usually shared with the TFT pins; restore their mode
    pinMode(TOUCH_YPLUS, OUTPUT);
    pinMode(TOUCH_XMINUS, OUTPUT);
#endif

    return result;
  }
  
  TSPoint getPoint() // calibrated
  {
    TSPoint result = getPointRaw();    
   
    const int16_t x = map(result.y, TOUCH_CAL_X1, TOUCH_CAL_X2, 0, DISP_WIDTH);
    const int16_t y = map(result.x, TOUCH_CAL_Y1, TOUCH_CAL_Y2, 0, DISP_HEIGHT);    
    return TSPoint(x, y, result.z);
  }
  
};
