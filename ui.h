// PMD32-Mega2560 (c) 2025 J. Bogin, https://boginjr.com
// Based on PMD32-SD (c) 2012 R. Borik, https://pmd85.borik.net/
// User interface

#pragma once
#include "config.h"

#define BUTTONS_COUNTOF(b) (sizeof((b)) / sizeof(Ui::Button))

class Ui
{
public:

  struct Button
  {
    Button(BYTE a = 0, BYTE c = 0)
    {
      action = a;
      progmemCaption = c;
    }
    
    BYTE action;
    BYTE progmemCaption;
  };
  
  struct ButtonRow
  {
    BYTE action;
    WORD X1;
    WORD Y1;
    WORD X2;
    WORD Y2;    
  };
  
  enum ButtonAction
  {
    NoButton = 0,
    OK,
    Cancel,
    Yes,
    No,
    Back,
    Mount,
    Unmount,
    Create,
    Eject,
    DriveA,
    DriveB,
    DriveC,
    DriveD,
    Up,
    Down,
    PgUp,
    PgDn,
    Open,
    FilePicked    
  }; 

  static Ui* get()
  {
    static Ui ui;
    return &ui;
  }
  
  //leave more for variadic expansion
  inline static char m_stringBuffer[MAX_PROGMEM_STRING_LEN + 24] = {0};
  
  WORD getCursorX() { return m_tft.getCursorX(); }
  WORD getCursorY() { return m_tft.getCursorY(); }
  void setCursorX(WORD X) { m_tft.setCursor(X, m_tft.getCursorY()); }
  void setCursorY(WORD Y) { m_tft.setCursor(m_tft.getCursorX(), Y); }
  void setCursor(WORD X, WORD Y) { m_tft.setCursor(X, Y); }
  
  void clearScreen();
  void outText(const char* text, bool centerHorz = false, bool centerVert = false, bool clearLine = false);
  void outButtons(const Button* buttons, const BYTE count, WORD widthEach, WORD heightEach);
  ButtonAction buttonPressed();
  void messageBox(BYTE progmemContent, BYTE progmemCaption = 0, bool hasButtons = true);
  void messageBox(const char* content, const char* caption = NULL, bool hasButtons = true);
  
  void drawFilePicker(bool rootDirectory, char* entriesPipeDelimited, BYTE curSel, DWORD curPage, DWORD pages);
  BYTE getFilePickerCount() { return m_filePickerCount; }
  BYTE getFilePickerSel() { return m_filePickerSel; }
  
private:  
  Ui();
    
  MCUFRIEND_kbv m_tft;
  Touch m_touch;
  
  ButtonRow* m_buttonRow;
  BYTE m_buttonsCount;
  
  bool m_filePicking;
  BYTE m_filePickerCount;
  BYTE m_filePickerSel;
};