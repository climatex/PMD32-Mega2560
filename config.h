// PMD32-Mega2560 (c) 2025 J. Bogin, https://boginjr.com
// Based on PMD32-SD (c) 2012 R. Borik, https://pmd85.borik.net/
// Build and wiring configuration (https://boginjr.com/it/hw/pmd32/ in SK)

#pragma once

// if uncommented, touchscreen is connected via an XPT2046 controller (HR2046 etc)
// commented: passive touchscreen with pins X+, Y+, X-, Y-
#define TOUCH_SCREEN_ACTIVE

// if uncommented, run touchscreen "calibration mode"
// run if the hits are imprecise, or their direction is wrong (left-right, up-down)
//#define TOUCH_SCREEN_CALIBRATION

// if uncommented, rotate display by 180°
//#define DISP_FLIP_ORIENTATION

// if uncommented, store paths of successfully mounted images into EEPROM
// and try to auto-mount them upon next board powerup
#define EEPROM_IMAGE_AUTOMOUNT

// if uncommented, use software SPI for SD card
// example: 8-bit Uno display shield with SD pins fixed on 10-13 instead of 50-53
// with this on, SPI_DRIVER_SELECT inside SdFat/SdFatConfig.h must be set to 2
//#define SD_SOFTWARE_SPI

/*

  Arduino to PMD cable wiring details - connection depends on display type used
  all data lines (8 to 20) routed to Mega2560 through 330ohm resistors
  cable connected to the "K3" FRB female (https://pmd85.borik.net/wiki/Konektory_na_PMD_85)
  
  default: 16-bit parallel display (fast)
  8-bit (slow): comment (disable) USE_MEGA_16BIT_SHIELD in src/MCUFRIEND_kbv/utility/mcufriend_special.h

  FRB        [16-bit display]    {8-bit display}
   1: GND    [GND]               {GND}
   2: N/C
   3: N/C
   4: N/C
   5: N/C
   6: N/C
   7: N/C
   8: DIR    [A0,  PORTF0]       {D22, PORTA0}
   9: /OBF   [A1,  PORTF1]       {D23, PORTA1}
  10: /ACK   [A2,  PORTF2]       {D24, PORTA2}
  11: IBF    [A3,  PORTF3]       {D25, PORTA3}
  12: /STB   [A4,  PORTF4]       {D26, PORTA4}
  13: D1     [A9,  PORTK1]       {D36, PORTC1}
  14: D0     [A8,  PORTK0]       {D37, PORTC0}
  15: D3     [A11, PORTK3]       {D34, PORTC3}
  16: D2     [A10, PORTK2]       {D35, PORTC2}
  17: D5     [A13, PORTK5]       {D32, PORTC5}
  18: D4     [A12, PORTK4]       {D33, PORTC4}
  19: D7     [A15, PORTK7]       {D30, PORTC7}
  20: D6     [A14, PORTK6]       {D31, PORTC6}
  
*/

#undef BYTE
#undef WORD
#undef DWORD
#define BYTE uint8_t
#define WORD uint16_t
#define DWORD uint32_t

// internal includes
#include <Arduino.h>
#include <avr/sfr_defs.h>
#include <string.h>
#include <EEPROM.h>

// default configuration: 320x240 TFT (16-bit parallel), microSD interface (HW SPI on pins 50-53), active touchscreen

// alternate configuration examples:
// (1) 16bit HX8347-I: as default, only DISP_ID_16BIT changed to 0x9595 
// (2) 8bit Uno shield, passive touch, SD on pins 10-13: comment USE_MEGA_16BIT_SHIELD in mcufriend_special.h,
//                                                       set SPI_DRIVER_SELECT 2 in SdFatConfig.h,
//                                                       comment TOUCH_SCREEN_ACTIVE and uncomment SD_SOFTWARE_SPI here
// for other controllers check #define SUPPORT_... in MCUFRIEND_kbv.cpp

// display
#define DISP_WIDTH     320      // px
#define DISP_HEIGHT    240      // px
#define DISP_ID_16BIT  0x9341   // controller ID for 16-bit displays (ILI9341: 0x9341), 8-bit: ignored, try to autodetect
#include "src/MCUFRIEND_kbv/MCUFRIEND_kbv.h"
#include "src/MCUFRIEND_kbv/utility/mcufriend_special.h"

// touchscreen calibration
// to respecify, uncomment TOUCH_SCREEN_CALIBRATION
#define TOUCH_CAL_X1     72 
#define TOUCH_CAL_Y1     96
#define TOUCH_CAL_X2     919
#define TOUCH_CAL_Y2     938

// touchscreen type
#ifdef TOUCH_SCREEN_ACTIVE
  #define TOUCH_CLK      6      // clock
  #define TOUCH_CS       5      // chip select
  #define TOUCH_DIN      4      // data in
  #define TOUCH_DOUT     3      // data out
  #include "src/XPT2046_Bitbang_Slim/XPT2046_Bitbang.h"
#else // passive
  #define TOUCH_XPLUS    8      // X+, XP
  #define TOUCH_YPLUS    A3     // Y+, YP
  #define TOUCH_XMINUS   A2     // X-, XM
  #define TOUCH_YMINUS   9      // Y-, YM
  #define TOUCH_RX       325    // resistance between XP-XM in ohms
  #include <TouchScreen.h>      // Adafruit
#endif

// SD
#include <SdFat.h>
#ifdef SD_SOFTWARE_SPI          // software
  #define SD_SWSPI_CS    10     // SD wired like on Uno
  #define SD_SWSPI_MOSI  11
  #define SD_SWSPI_MISO  12
  #define SD_SWSPI_CLK   13
  
  #if SPI_DRIVER_SELECT != 2
    #error SPI_DRIVER_SELECT must be 2 in SdFat/SdFatConfig.h
  #endif
#else                           // hardware
  #define SD_HWSPI_CS    53     // SD chip select
  
  #if SPI_DRIVER_SELECT >= 2
    #error Set SPI_DRIVER_SELECT to 0 or 1 in SdFat/SdFatConfig.h
  #endif  
#endif

// our common includes
#include "progmem.h"
#include "touch.h"
#include "ui.h"
#include "filesystem.h"
#include "pmd32.h"

// public globals
#ifndef TOUCH_SCREEN_CALIBRATION
  extern Ui* ui;
  extern SdFat sd;
  extern BYTE mountedDrives;
#endif