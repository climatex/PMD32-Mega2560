// PMD32-Mega2560 (c) 2025 J. Bogin, https://boginjr.com
// Based on PMD32-SD (c) 2012 R. Borik, https://pmd85.borik.net/
// Program memory data

#pragma once
#include "config.h"

// maximum number of characters for each string in PROGMEM, MAX+1 size of buffer for pgm_read_ptr()
#define MAX_PROGMEM_STRING_LEN 32

// bleh
#define PROGMEM_DATA inline static const unsigned char

class Progmem
{
public:
  
  // enum in sync with order of m_stringTable
  enum BYTE
  {
    Empty = 0,
    uiTitle,
    uiCardDetails,
    uiNoCardPresent,
    uiUnsupportedFS,
    uiMountedDrives,
    uiCardSafeToEject,
    uiMountQuestion,
    uiMountCaption,
    uiMountReadOnly,
    uiUnmountQuestion,
    uiUnmountCaption,
    uiCreateQuestion,
    uiCreateCaption,
    uiCreateConfirm,
    uiPickerDetails,
    uiPickerRootDir,
    uiPickerOneLevelUp,
    uiUpdatingEEPROM,
    uiLoadingEEPROM,
    uiError,
    uiErrorMemory,
    uiErrorFS,
    uiErrorPath,
    uiErrorFileOpen,
    uiErrorFileOpened,
    uiErrorFileCreate,
    uiErrorFileSize,
    uiBusy,
    btnOK,
    btnCancel,
    btnYes,
    btnNo,
    btnBack,
    btnMount,
    btnUnmount,
    btnCreate,
    btnEject,
    btnDriveA,
    btnDriveB,
    btnDriveC,
    btnDriveD,
    btnUp,
    btnDown,
    btnPgUp,
    btnPgDn,
    btnOpen
  };
  
  // retrieve string from progmem, buffer valid until next call
  static const char* getString(BYTE stringIndex)
  {   
    strncpy_P(m_strBuffer, pgm_read_ptr(&(m_stringTable[stringIndex])), MAX_PROGMEM_STRING_LEN);
    return (const char*)&m_strBuffer[0];
  }
  
// messages (definition order does not matter here). Length max MAX_PROGMEM_STRING_LEN
// variadics can be loaded only one at a time from PROGMEM,
// as only one buffer is used to transfer these between program space and our RAM
private:
  PROGMEM_DATA m_Empty[]              PROGMEM = "";
  PROGMEM_DATA m_uiTitle[]            PROGMEM = "PMD 32 / Mega2560";
  PROGMEM_DATA m_uiCardDetails[]      PROGMEM = "%u %s (micro)SD%s card inserted";
  PROGMEM_DATA m_uiNoCardPresent[]    PROGMEM = "No memory card recognized";
  PROGMEM_DATA m_uiUnsupportedFS[]    PROGMEM = "Must be FAT16/FAT32/exFAT on MBR";
  PROGMEM_DATA m_uiMountedDrives[]    PROGMEM = "%u mounted drive image(s)";
  PROGMEM_DATA m_uiCardSafeToEject[]  PROGMEM = "Memory card can now be ejected";
  PROGMEM_DATA m_uiMountQuestion[]    PROGMEM = "Which drive to mount?";
  PROGMEM_DATA m_uiMountCaption[]     PROGMEM = "Mount drive image";
  PROGMEM_DATA m_uiMountReadOnly[]    PROGMEM = "Mount as read-only?";
  PROGMEM_DATA m_uiUnmountQuestion[]  PROGMEM = "Which drive to unmount?";
  PROGMEM_DATA m_uiUnmountCaption[]   PROGMEM = "Unmount drive image";  
  PROGMEM_DATA m_uiCreateQuestion[]   PROGMEM = "Assign new image to drive:";
  PROGMEM_DATA m_uiCreateCaption[]    PROGMEM = "Create new image";
  PROGMEM_DATA m_uiCreateConfirm[]    PROGMEM = "Create and mount %s ?";
  PROGMEM_DATA m_uiPickerDetails[]    PROGMEM = "Page %lu of %lu; Filter: *.p32";
  PROGMEM_DATA m_uiPickerRootDir[]    PROGMEM = "[.]";
  PROGMEM_DATA m_uiPickerOneLevelUp[] PROGMEM = "[..]";
  PROGMEM_DATA m_uiUpdatingEEPROM[]   PROGMEM = "Updating EEPROM...";
  PROGMEM_DATA m_uiLoadingEEPROM[]    PROGMEM = "Auto-mounting drives...";
  PROGMEM_DATA m_uiError[]            PROGMEM = "Error";
  PROGMEM_DATA m_uiErrorMemory[]      PROGMEM = "Memory allocation error";
  PROGMEM_DATA m_uiErrorFS[]          PROGMEM = "SD card filesystem error";
  PROGMEM_DATA m_uiErrorPath[]        PROGMEM = "Path nested too deep";
  PROGMEM_DATA m_uiErrorFileOpen[]    PROGMEM = "Cannot open file";
  PROGMEM_DATA m_uiErrorFileOpened[]  PROGMEM = "This image is already mounted";
  PROGMEM_DATA m_uiErrorFileCreate[]  PROGMEM = "Cannot create file";
  PROGMEM_DATA m_uiErrorFileSize[]    PROGMEM = "Invalid P32 - must be 360K";
  PROGMEM_DATA m_uiBusy[]             PROGMEM = "Busy...";
  PROGMEM_DATA m_btnOK[]              PROGMEM = "OK";
  PROGMEM_DATA m_btnCancel[]          PROGMEM = "Cancel";
  PROGMEM_DATA m_btnYes[]             PROGMEM = "Yes";
  PROGMEM_DATA m_btnNo[]              PROGMEM = "No";
  PROGMEM_DATA m_btnBack[]            PROGMEM = "Back";
  PROGMEM_DATA m_btnMount[]           PROGMEM = "Mount";
  PROGMEM_DATA m_btnUnmount[]         PROGMEM = "Unmount";
  PROGMEM_DATA m_btnCreate[]          PROGMEM = "Create";
  PROGMEM_DATA m_btnEject[]           PROGMEM = "Eject SD";
  PROGMEM_DATA m_btnDriveA[]          PROGMEM = "A:";
  PROGMEM_DATA m_btnDriveB[]          PROGMEM = "B:";
  PROGMEM_DATA m_btnDriveC[]          PROGMEM = "C:";
  PROGMEM_DATA m_btnDriveD[]          PROGMEM = "D:";
  PROGMEM_DATA m_btnUp[]              PROGMEM = "/\\";
  PROGMEM_DATA m_btnDown[]            PROGMEM = "\\/";
  PROGMEM_DATA m_btnPgUp[]            PROGMEM = "<<";
  PROGMEM_DATA m_btnPgDn[]            PROGMEM = ">>";
  PROGMEM_DATA m_btnOpen[]            PROGMEM = "Open";
  
// string table
  PROGMEM_DATA* const m_stringTable[] PROGMEM = {  
                                                  m_Empty,
                                                  
                                                  m_uiTitle,
                                                  
                                                  m_uiCardDetails, m_uiNoCardPresent, m_uiUnsupportedFS, m_uiMountedDrives,
                                                  m_uiCardSafeToEject, m_uiMountQuestion, m_uiMountCaption, m_uiMountReadOnly,
                                                  m_uiUnmountQuestion, m_uiUnmountCaption, m_uiCreateQuestion, m_uiCreateCaption,
                                                  m_uiCreateConfirm,
                                                  
                                                  m_uiPickerDetails, m_uiPickerRootDir, m_uiPickerOneLevelUp,
                                                  m_uiUpdatingEEPROM, m_uiLoadingEEPROM,
                                                  m_uiError, m_uiErrorMemory, m_uiErrorFS, m_uiErrorPath, m_uiErrorFileOpen,
                                                  m_uiErrorFileOpened, m_uiErrorFileCreate, m_uiErrorFileSize,
                                                  
                                                  m_uiBusy,

                                                  m_btnOK, m_btnCancel, m_btnYes, m_btnNo, m_btnBack, m_btnMount, 
                                                  m_btnUnmount, m_btnCreate, m_btnEject, m_btnDriveA, m_btnDriveB,
                                                  m_btnDriveC, m_btnDriveD, m_btnUp, m_btnDown, m_btnPgUp, m_btnPgDn, m_btnOpen
                                                };

// provides for transfering messages between program space and our address space
// even though we're in class Progmem, this is in RAM, not in PROGMEM itself
  inline static unsigned char m_strBuffer[MAX_PROGMEM_STRING_LEN + 1];
  
// 8x16 monospace "VGA" font
private:
  PROGMEM_DATA m_vgaFontBitmap[] PROGMEM =
  {
    0x6F, 0xFF, 0x66, 0x60, 0x66, 0xCF, 0x3C, 0xD2, 0x6C, 0xDB, 0xFB, 0x66,
    0xCD, 0xBF, 0xB6, 0x6C, 0x18, 0x31, 0xF6, 0x3C, 0x38, 0x1F, 0x03, 0x07,
    0x0F, 0x1B, 0xE1, 0x83, 0x00, 0xC3, 0x8C, 0x30, 0xC3, 0x0C, 0x31, 0xC3,
    0x38, 0xD9, 0xB1, 0xC7, 0x7B, 0xB3, 0x66, 0xCC, 0xEC, 0x6D, 0xE0, 0x36,
    0xCC, 0xCC, 0xCC, 0x63, 0xC6, 0x33, 0x33, 0x33, 0x6C, 0x66, 0x3C, 0xFF,
    0x3C, 0x66, 0x30, 0xCF, 0xCC, 0x30, 0x6D, 0xE0, 0xFE, 0xF0, 0x02, 0x0C,
    0x38, 0xE3, 0x8E, 0x38, 0x60, 0x80, 0x38, 0xDB, 0x1E, 0x3D, 0x7A, 0xF1,
    0xE3, 0x6C, 0x70, 0x31, 0xCF, 0x0C, 0x30, 0xC3, 0x0C, 0x33, 0xF0, 0x7D,
    0x8C, 0x18, 0x61, 0x86, 0x18, 0x60, 0xC7, 0xFC, 0x7D, 0x8C, 0x18, 0x33,
    0xC0, 0xC1, 0x83, 0xC6, 0xF8, 0x0C, 0x38, 0xF3, 0x6C, 0xDF, 0xC3, 0x06,
    0x0C, 0x3C, 0xFF, 0x83, 0x06, 0x0F, 0xC0, 0xC1, 0x83, 0xC6, 0xF8, 0x38,
    0xC3, 0x06, 0x0F, 0xD8, 0xF1, 0xE3, 0xC6, 0xF8, 0xFF, 0x8C, 0x18, 0x30,
    0xC3, 0x0C, 0x18, 0x30, 0x60, 0x7D, 0x8F, 0x1E, 0x37, 0xD8, 0xF1, 0xE3,
    0xC6, 0xF8, 0x7D, 0x8F, 0x1E, 0x37, 0xE0, 0xC1, 0x83, 0x0C, 0xF0, 0xF0,
    0x3C, 0x6C, 0x00, 0xDE, 0x0C, 0x63, 0x18, 0xC1, 0x83, 0x06, 0x0C, 0xFC,
    0x00, 0x3F, 0xC1, 0x83, 0x06, 0x0C, 0x63, 0x18, 0xC0, 0x7D, 0x8F, 0x18,
    0x61, 0x83, 0x06, 0x00, 0x18, 0x30, 0x7D, 0x8F, 0x1E, 0xFD, 0xFB, 0xF7,
    0x60, 0x7C, 0x10, 0x71, 0xB6, 0x3C, 0x7F, 0xF1, 0xE3, 0xC7, 0x8C, 0xFC,
    0xCD, 0x9B, 0x37, 0xCC, 0xD9, 0xB3, 0x67, 0xF8, 0x3C, 0xCF, 0x0E, 0x0C,
    0x18, 0x30, 0x61, 0x66, 0x78, 0xF8, 0xD9, 0x9B, 0x36, 0x6C, 0xD9, 0xB3,
    0x6D, 0xF0, 0xFE, 0xCD, 0x8B, 0x47, 0x8D, 0x18, 0x31, 0x67, 0xFC, 0xFE,
    0xCD, 0x8B, 0x47, 0x8D, 0x18, 0x30, 0x61, 0xE0, 0x3C, 0xCF, 0x0E, 0x0C,
    0x1B, 0xF1, 0xE3, 0x66, 0x74, 0xC7, 0x8F, 0x1E, 0x3F, 0xF8, 0xF1, 0xE3,
    0xC7, 0x8C, 0xF6, 0x66, 0x66, 0x66, 0x6F, 0x1E, 0x18, 0x30, 0x60, 0xC1,
    0xB3, 0x66, 0xCC, 0xF0, 0xE6, 0xCD, 0x9B, 0x67, 0x8F, 0x1B, 0x33, 0x67,
    0xCC, 0xF0, 0xC1, 0x83, 0x06, 0x0C, 0x18, 0x31, 0x67, 0xFC, 0xC7, 0xDF,
    0xFF, 0xFD, 0x78, 0xF1, 0xE3, 0xC7, 0x8C, 0xC7, 0xCF, 0xDF, 0xFD, 0xF9,
    0xF1, 0xE3, 0xC7, 0x8C, 0x7D, 0x8F, 0x1E, 0x3C, 0x78, 0xF1, 0xE3, 0xC6,
    0xF8, 0xFC, 0xCD, 0x9B, 0x37, 0xCC, 0x18, 0x30, 0x61, 0xE0, 0x7D, 0x8F,
    0x1E, 0x3C, 0x78, 0xF1, 0xEB, 0xDE, 0xF8, 0x30, 0x70, 0xFC, 0xCD, 0x9B,
    0x37, 0xCD, 0x99, 0xB3, 0x67, 0xCC, 0x7D, 0x8F, 0x1B, 0x03, 0x81, 0x81,
    0xE3, 0xC6, 0xF8, 0xFF, 0xFB, 0x4C, 0x30, 0xC3, 0x0C, 0x31, 0xE0, 0xC7,
    0x8F, 0x1E, 0x3C, 0x78, 0xF1, 0xE3, 0xC6, 0xF8, 0xC7, 0x8F, 0x1E, 0x3C,
    0x78, 0xF1, 0xB6, 0x38, 0x20, 0xC7, 0x8F, 0x1E, 0x3D, 0x7A, 0xF5, 0xFF,
    0xEE, 0xD8, 0xC7, 0x8D, 0xB3, 0xE3, 0x87, 0x1F, 0x36, 0xC7, 0x8C, 0xCF,
    0x3C, 0xF3, 0x78, 0xC3, 0x0C, 0x31, 0xE0, 0xFF, 0x8E, 0x18, 0x61, 0x86,
    0x18, 0x61, 0xC7, 0xFC, 0xFC, 0xCC, 0xCC, 0xCC, 0xCF, 0x81, 0x83, 0x83,
    0x83, 0x83, 0x83, 0x83, 0x02, 0xF3, 0x33, 0x33, 0x33, 0x3F, 0x10, 0x71,
    0xB6, 0x30, 0xFF, 0xD9, 0x80, 0x78, 0x19, 0xF6, 0x6C, 0xD9, 0x9D, 0x80,
    0xE0, 0xC1, 0x83, 0xC6, 0xCC, 0xD9, 0xB3, 0x66, 0xF8, 0x7D, 0x8F, 0x06,
    0x0C, 0x18, 0xDF, 0x00, 0x1C, 0x18, 0x31, 0xE6, 0xD9, 0xB3, 0x66, 0xCC,
    0xEC, 0x7D, 0x8F, 0xFE, 0x0C, 0x18, 0xDF, 0x00, 0x39, 0xB6, 0x58, 0xF1,
    0x86, 0x18, 0x63, 0xC0, 0x77, 0x9B, 0x36, 0x6C, 0xD9, 0x9F, 0x06, 0xCC,
    0xF0, 0xE0, 0xC1, 0x83, 0x67, 0x6C, 0xD9, 0xB3, 0x67, 0xCC, 0x66, 0x0E,
    0x66, 0x66, 0x6F, 0x0C, 0x30, 0x07, 0x0C, 0x30, 0xC3, 0x0C, 0x3C, 0xF3,
    0x78, 0xE0, 0xC1, 0x83, 0x36, 0xCF, 0x1E, 0x36, 0x67, 0xCC, 0xE6, 0x66,
    0x66, 0x66, 0x6F, 0xED, 0xFF, 0x5E, 0xBD, 0x7A, 0xF1, 0x80, 0xDC, 0xCD,
    0x9B, 0x36, 0x6C, 0xD9, 0x80, 0x7D, 0x8F, 0x1E, 0x3C, 0x78, 0xDF, 0x00,
    0xDC, 0xCD, 0x9B, 0x36, 0x6C, 0xDF, 0x30, 0x61, 0xE0, 0x77, 0x9B, 0x36,
    0x6C, 0xD9, 0x9F, 0x06, 0x0C, 0x3C, 0xDC, 0xED, 0x9B, 0x06, 0x0C, 0x3C,
    0x00, 0x7D, 0x8D, 0x81, 0xC0, 0xD8, 0xDF, 0x00, 0x10, 0x60, 0xC7, 0xE3,
    0x06, 0x0C, 0x18, 0x36, 0x38, 0xCD, 0x9B, 0x36, 0x6C, 0xD9, 0x9D, 0x80,
    0xCF, 0x3C, 0xF3, 0xCD, 0xE3, 0x00, 0xC7, 0x8F, 0x5E, 0xBD, 0x7F, 0xDB,
    0x00, 0xC6, 0xD8, 0xE1, 0xC3, 0x8D, 0xB1, 0x80, 0xC7, 0x8F, 0x1E, 0x3C,
    0x78, 0xDF, 0x83, 0x0D, 0xF0, 0xFF, 0x98, 0x61, 0x86, 0x18, 0xFF, 0x80,
    0x1C, 0xC3, 0x0C, 0xE0, 0xC3, 0x0C, 0x30, 0x70, 0xFF, 0x3F, 0xF0, 0xE0,
    0xC3, 0x0C, 0x1C, 0xC3, 0x0C, 0x33, 0x80, 0x77, 0xB8
  };
  
  inline static const GFXglyph m_vgaFontGlyphs[] PROGMEM = 
  {
    {     0,   0,   0,   8,    0,    1 },   // 0x20 ' '
    {     0,   4,  10,   8,    2,   -9 },   // 0x21 '!'
    {     5,   6,   4,   8,    1,  -10 },   // 0x22 '"'
    {     8,   7,   9,   8,    0,   -8 },   // 0x23 '#'
    {    16,   7,  14,   8,    0,  -11 },   // 0x24 '$'
    {    29,   7,   8,   8,    0,   -7 },   // 0x25 '%'
    {    36,   7,  10,   8,    0,   -9 },   // 0x26 '&'
    {    45,   3,   4,   8,    1,  -10 },   // 0x27 '''
    {    47,   4,  10,   8,    2,   -9 },   // 0x28 '('
    {    52,   4,  10,   8,    2,   -9 },   // 0x29 ')'
    {    57,   8,   5,   8,    0,   -6 },   // 0x2A '*'
    {    62,   6,   5,   8,    1,   -6 },   // 0x2B '+'
    {    66,   3,   4,   8,    2,   -2 },   // 0x2C ','
    {    68,   7,   1,   8,    0,   -4 },   // 0x2D '-'
    {    69,   2,   2,   8,    3,   -1 },   // 0x2E '.'
    {    70,   7,   9,   8,    0,   -8 },   // 0x2F '/'
    {    78,   7,  10,   8,    0,   -9 },   // 0x30 '0'
    {    87,   6,  10,   8,    1,   -9 },   // 0x31 '1'
    {    95,   7,  10,   8,    0,   -9 },   // 0x32 '2'
    {   104,   7,  10,   8,    0,   -9 },   // 0x33 '3'
    {   113,   7,  10,   8,    0,   -9 },   // 0x34 '4'
    {   122,   7,  10,   8,    0,   -9 },   // 0x35 '5'
    {   131,   7,  10,   8,    0,   -9 },   // 0x36 '6'
    {   140,   7,  10,   8,    0,   -9 },   // 0x37 '7'
    {   149,   7,  10,   8,    0,   -9 },   // 0x38 '8'
    {   158,   7,  10,   8,    0,   -9 },   // 0x39 '9'
    {   167,   2,   7,   8,    3,   -7 },   // 0x3A ':'
    {   169,   3,   8,   8,    2,   -7 },   // 0x3B ';'
    {   172,   6,   9,   8,    1,   -8 },   // 0x3C '<'
    {   179,   6,   4,   8,    1,   -6 },   // 0x3D '='
    {   182,   6,   9,   8,    1,   -8 },   // 0x3E '>'
    {   189,   7,  10,   8,    0,   -9 },   // 0x3F '?'
    {   198,   7,   9,   8,    0,   -8 },   // 0x40 '@'
    {   206,   7,  10,   8,    0,   -9 },   // 0x41 'A'
    {   215,   7,  10,   8,    0,   -9 },   // 0x42 'B'
    {   224,   7,  10,   8,    0,   -9 },   // 0x43 'C'
    {   233,   7,  10,   8,    0,   -9 },   // 0x44 'D'
    {   242,   7,  10,   8,    0,   -9 },   // 0x45 'E'
    {   251,   7,  10,   8,    0,   -9 },   // 0x46 'F'
    {   260,   7,  10,   8,    0,   -9 },   // 0x47 'G'
    {   269,   7,  10,   8,    0,   -9 },   // 0x48 'H'
    {   278,   4,  10,   8,    2,   -9 },   // 0x49 'I'
    {   283,   7,  10,   8,    0,   -9 },   // 0x4A 'J'
    {   292,   7,  10,   8,    0,   -9 },   // 0x4B 'K'
    {   301,   7,  10,   8,    0,   -9 },   // 0x4C 'L'
    {   310,   7,  10,   8,    0,   -9 },   // 0x4D 'M'
    {   319,   7,  10,   8,    0,   -9 },   // 0x4E 'N'
    {   328,   7,  10,   8,    0,   -9 },   // 0x4F 'O'
    {   337,   7,  10,   8,    0,   -9 },   // 0x50 'P'
    {   346,   7,  12,   8,    0,   -9 },   // 0x51 'Q'
    {   357,   7,  10,   8,    0,   -9 },   // 0x52 'R'
    {   366,   7,  10,   8,    0,   -9 },   // 0x53 'S'
    {   375,   6,  10,   8,    1,   -9 },   // 0x54 'T'
    {   383,   7,  10,   8,    0,   -9 },   // 0x55 'U'
    {   392,   7,  10,   8,    0,   -9 },   // 0x56 'V'
    {   401,   7,  10,   8,    0,   -9 },   // 0x57 'W'
    {   410,   7,  10,   8,    0,   -9 },   // 0x58 'X'
    {   419,   6,  10,   8,    1,   -9 },   // 0x59 'Y'
    {   427,   7,  10,   8,    0,   -9 },   // 0x5A 'Z'
    {   436,   4,  10,   8,    2,   -9 },   // 0x5B '['
    {   441,   7,   9,   8,    0,   -8 },   // 0x5C '\'
    {   449,   4,  10,   8,    2,   -9 },   // 0x5D ']'
    {   454,   7,   4,   8,    0,  -11 },   // 0x5E '^'
    {   458,   8,   1,   8,    0,    2 },   // 0x5F '_'
    {   459,   3,   3,   8,    2,  -11 },   // 0x60 '`'
    {   461,   7,   7,   8,    0,   -6 },   // 0x61 'a'
    {   468,   7,  10,   8,    0,   -9 },   // 0x62 'b'
    {   477,   7,   7,   8,    0,   -6 },   // 0x63 'c'
    {   484,   7,  10,   8,    0,   -9 },   // 0x64 'd'
    {   493,   7,   7,   8,    0,   -6 },   // 0x65 'e'
    {   500,   6,  10,   8,    0,   -9 },   // 0x66 'f'
    {   508,   7,  10,   8,    0,   -6 },   // 0x67 'g'
    {   517,   7,  10,   8,    0,   -9 },   // 0x68 'h'
    {   526,   4,  10,   8,    2,   -9 },   // 0x69 'i'
    {   531,   6,  13,   8,    1,   -9 },   // 0x6A 'j'
    {   541,   7,  10,   8,    0,   -9 },   // 0x6B 'k'
    {   550,   4,  10,   8,    2,   -9 },   // 0x6C 'l'
    {   555,   7,   7,   8,    0,   -6 },   // 0x6D 'm'
    {   562,   7,   7,   8,    0,   -6 },   // 0x6E 'n'
    {   569,   7,   7,   8,    0,   -6 },   // 0x6F 'o'
    {   576,   7,  10,   8,    0,   -6 },   // 0x70 'p'
    {   585,   7,  10,   8,    0,   -6 },   // 0x71 'q'
    {   594,   7,   7,   8,    0,   -6 },   // 0x72 'r'
    {   601,   7,   7,   8,    0,   -6 },   // 0x73 's'
    {   608,   7,  10,   8,    0,   -9 },   // 0x74 't'
    {   617,   7,   7,   8,    0,   -6 },   // 0x75 'u'
    {   624,   6,   7,   8,    1,   -6 },   // 0x76 'v'
    {   630,   7,   7,   8,    0,   -6 },   // 0x77 'w'
    {   637,   7,   7,   8,    0,   -6 },   // 0x78 'x'
    {   644,   7,  10,   8,    0,   -6 },   // 0x79 'y'
    {   653,   7,   7,   8,    0,   -6 },   // 0x7A 'z'
    {   660,   6,  10,   8,    1,   -9 },   // 0x7B '{'
    {   668,   2,  10,   8,    3,   -9 },   // 0x7C '|'
    {   671,   6,  10,   8,    1,   -9 },   // 0x7D '}'
    {   679,   7,   2,   8,    0,   -9 }    // 0x7E '~'
  }; 

public:
  inline static const GFXfont m_vgaFont PROGMEM =
  {
    m_vgaFontBitmap,
    m_vgaFontGlyphs,
    0x20, 0x7E, 16
  };
  
};