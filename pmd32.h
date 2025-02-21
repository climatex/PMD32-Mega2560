// PMD32-Mega2560 (c) 2025 J. Bogin, https://boginjr.com
// Based on PMD32-SD (c) 2012 R. Borik, https://pmd85.borik.net/
// PMD32 drive emulation with PMD32-SD extras

#pragma once
#include "config.h"

#ifdef USE_MEGA_16BIT_SHIELD // 16bit display in use, defined in mcufriend_special.h
  #define PMD_CTRL_DDR DDRF  // PMD cable connected on "analog" pins
  #define PMD_CTRL_OUT PORTF
  #define PMD_CTRL_IN  PINF
  #define PMD_DATA_DDR DDRK
  #define PMD_DATA_OUT PORTK
  #define PMD_DATA_IN  PINK
#else                        // 8bit display, PMD cable connected to rightmost pinheader
  #define PMD_CTRL_DDR DDRA
  #define PMD_CTRL_OUT PORTA
  #define PMD_CTRL_IN  PINA
  #define PMD_DATA_DDR DDRC
  #define PMD_DATA_OUT PORTC
  #define PMD_DATA_IN  PINC
#endif

// commands - PMD32 original
#define PMD32_READ_BOOT      0x42 // 'B'
#define PMD32_READ_LOGICAL1  0x51 // 'Q'
#define PMD32_READ_LOGICAL2  0x52 // 'R', same as above, just different byte
#define PMD32_WRITE_LOGICAL1 0x54 // 'T'
#define PMD32_WRITE_LOGICAL2 0x57 // 'W', ditto
#define PMD32_WRITE_PHYSICAL 0x53 // 'S'
#define PMD32_CHANGE_DRIVE   0x49 // 'I'
#define PMD32_FORMAT_TRACK   0x46 // 'F'
#define PMD32_READ_RAM       0x43 // 'C', skipped
#define PMD32_WRITE_RAM      0x55 // 'U', skipped
#define PMD32_EXECUTE_RAM    0x4A // 'J', skipped
#define PMD32_SLOW_MODE      0x40 // '@', skipped
#define PMD32_FAST_MODE      0x2A // '*', skipped
// commands - PMD32-SD extra
#define PMD32_GET_IMAGE_PATH 0x47 // 'G'
#define PMD32_MOUNT_IMAGE    0x48 // 'H'
#define PMD32_GET_CWD        0x4B // 'K'
#define PMD32_DIR_LISTING    0x4C // 'L'
#define PMD32_CHANGE_CWD     0x4D // 'M'
#define PMD32_CREATE_IMAGE   0x4E // 'N'
#define PMD32_IMAGE_INFO     0x50 // 'P'

// responses - PMD32 original
#define PMD32_IDLE           0xAA // drive present
#define PMD32_ACK            0x33
#define PMD32_NAK            0x99
#define PMD32_OK             0
#define PMD32_WRITE_PROTECT  1
#define PMD32_FORMAT_ERROR   2
#define PMD32_READ_ERROR     3
#define PMD32_WRITE_ERROR    4
// responses - PMD32-SD extra
#define PMD32_INVALID_DRIVE  5
#define PMD32_PATH_NOT_FOUND 6
#define PMD32_PATH_TOO_LONG  7
#define PMD32_CREATE_ERROR   8
#define PMD32_IMAGE_UNKNOWN  0x0C

// ms
#define TIMEOUT_READ         5
#define TIMEOUT_READ_IDLE    10
#define TIMEOUT_READ_CMD     30

#define TIMEOUT_SEND         30
#define TIMEOUT_SEND_IDLE    15
#define TIMEOUT_SEND_RESULT  50
#define TIMEOUT_SEND_ACK     500
#define TIMEOUT_SEND_NAK     0

class PMD32
{
public: 
 
  PMD32();
  virtual ~PMD32() {};
  
  bool processCommand();
  
private:
// PMD32
  BYTE m_CRC;
  bool m_hostResponding;
  BYTE m_ioBuffer[512];
  
  bool readByte(BYTE& data, DWORD timeout = TIMEOUT_READ, bool checkCRC = false);  
  bool sendByte(BYTE data, DWORD timeout = TIMEOUT_SEND);    
  void doRWOperation(bool write, bool format, bool readBootSector, WORD bytes);
  void changeDrive();
  void dummyCommand(BYTE inputArgumentsCount = 0, BYTE outputZerosCount = 1);
  
// PMD32-SD extra
  File m_dirListing;
  BYTE m_cwdPath[64 + 1];
  
  void extraSendMaxLengthString(BYTE maxLength, const char* str);
  void extraGetImagePath();
  void extraMountImage();
  void extraGetCurrentWorkingDirectory();
  void extraDoDirectoryListing();
  void extraChangeCurrentWorkingDirectory();
  void extraCreateImage();
  void extraImageInfo();
};