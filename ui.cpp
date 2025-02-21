// PMD32-Mega2560 (c) 2025 J. Bogin, https://boginjr.com
// Based on PMD32-SD (c) 2012 R. Borik, https://pmd85.borik.net/
// User interface

#include "config.h"

#ifndef TOUCH_SCREEN_CALIBRATION

// colors
#define COLOR_BLACK      0
#define COLOR_WHITE      0xFFFF
#define COLOR_BLUE       0x10
#define COLOR_BROWN      0x9240
#define COLOR_OUTSIDE    0x410
#define COLOR_BACKGROUND 0xC618
#define COLOR_SHADOW     0x8410

Ui::Ui()
{
  m_buttonRow = NULL;
  m_buttonsCount = 0;
  m_filePicking = false;
  m_filePickerCount = 0;
  m_filePickerSel = 0;
  
  m_tft.reset();
  
#ifdef USE_MEGA_16BIT_SHIELD
  WORD displayId = DISP_ID_16BIT;  // 16bit shields are write only
#else
  WORD displayId = m_tft.readID(); // 8bit: try to autodetect
#endif
    
	m_tft.begin(displayId);
  
#ifdef DISP_FLIP_ORIENTATION
	m_tft.setRotation(3);
#else
  m_tft.setRotation(1);
#endif
  m_tft.fillScreen(COLOR_OUTSIDE);
  m_tft.setFont(&Progmem::m_vgaFont); // 8x16, monospace
  m_tft.setCursor(0, 0);
      
  // make title  
  clearScreen();
}

void Ui::clearScreen()
{
  m_tft.fillRect(DISP_WIDTH/32, DISP_HEIGHT/24, DISP_WIDTH*0.935, DISP_HEIGHT*0.91, COLOR_BACKGROUND);
  m_tft.fillRect(DISP_WIDTH/32, DISP_HEIGHT/24, DISP_WIDTH*0.935, 22, COLOR_BLUE);
  setCursorY((DISP_HEIGHT/24)+15);
  m_tft.setTextColor(COLOR_WHITE);
  outText(Progmem::getString(Progmem::uiTitle), true);  
  m_tft.setTextColor(COLOR_BLACK);
  
  if (m_buttonRow)
  {
    delete[] m_buttonRow;
  }
  
  m_buttonRow = NULL;
  m_buttonsCount = 0;
  m_filePickerCount = 0;
  m_filePickerSel = 0;
}

void Ui::outText(const char* text, bool centerHorz, bool centerVert, bool clearLine)
{
  if (!text)
  {
    return;
  }
  
  if (centerHorz)
  {
    setCursorX((DISP_WIDTH/2) - (strlen(text) * 4));
  }
  if (centerVert)
  {
    setCursorY((DISP_HEIGHT/2) + 8);
  }
  if (clearLine)
  {
    m_tft.fillRect(DISP_WIDTH/32, m_tft.getCursorY()-12, DISP_WIDTH*0.935, 16, COLOR_BACKGROUND);
  }
  
  m_tft.print(text);
  m_filePicking = false;
}

void Ui::outButtons(const Button* buttons, const BYTE count, WORD widthEach, WORD heightEach)
{
  if (!buttons || !count)
  {
    return;
  }
  
  if (m_buttonRow)
  {
    delete[] m_buttonRow;
  }   
  m_buttonRow = new ButtonRow[count];  
  if (!m_buttonRow)
  {
    return;
  }
  m_buttonsCount = count;
  
  const BYTE spacing = DISP_WIDTH/80;
  const WORD widthAll = (widthEach * count) + (spacing * (count-1));
  
  WORD X = (DISP_WIDTH/2) - (widthAll/2) - 1;
  const WORD Y = getCursorY();
    
  for (BYTE at = 0; at < count; at++)
  {    
    m_buttonRow[at].action = buttons[at].action;    
    m_buttonRow[at].X1 = X;
    m_buttonRow[at].Y1 = Y;
    
    X += widthEach;
    m_buttonRow[at].X2 = X;
    m_buttonRow[at].Y2 = Y + heightEach;
    X += spacing;
    
    m_tft.drawFastHLine(m_buttonRow[at].X1, m_buttonRow[at].Y1, m_buttonRow[at].X2 - m_buttonRow[at].X1 - 1, COLOR_WHITE);
    m_tft.drawFastVLine(m_buttonRow[at].X1, m_buttonRow[at].Y1, m_buttonRow[at].Y2 - m_buttonRow[at].Y1 - 1, COLOR_WHITE);
    m_tft.drawFastHLine(m_buttonRow[at].X1, m_buttonRow[at].Y2, m_buttonRow[at].X2 - m_buttonRow[at].X1, COLOR_BLACK);
    m_tft.drawFastVLine(m_buttonRow[at].X2, m_buttonRow[at].Y1, m_buttonRow[at].Y2 - m_buttonRow[at].Y1, COLOR_BLACK);
    m_tft.drawFastHLine(m_buttonRow[at].X1 + 1, m_buttonRow[at].Y2 - 1, m_buttonRow[at].X2 - m_buttonRow[at].X1 - 1, COLOR_SHADOW);
    m_tft.drawFastVLine(m_buttonRow[at].X2 - 1, m_buttonRow[at].Y1 + 1, m_buttonRow[at].Y2 - m_buttonRow[at].Y1 - 1, COLOR_SHADOW);
    
    const char* text = Progmem::getString(buttons[at].progmemCaption);
    setCursor(m_buttonRow[at].X1 + ((m_buttonRow[at].X2 - m_buttonRow[at].X1)/2) - (strlen(text) * 4),
              m_buttonRow[at].Y1 + ((m_buttonRow[at].Y2 - m_buttonRow[at].Y1)/2) + 4);
    m_tft.print(text);
  }
}

void Ui::messageBox(const char* content, const char* caption, bool hasButtons)
{
  if (!content)
  {
    return;
  }
  
  m_tft.setTextColor(COLOR_WHITE);  
  m_tft.fillRect(DISP_WIDTH/16, (DISP_HEIGHT/4)-22, DISP_WIDTH*0.87, (DISP_HEIGHT*0.6)+24, COLOR_BLACK);
  m_tft.fillRect((DISP_WIDTH/16)+2, (DISP_HEIGHT/4)-20, (DISP_WIDTH*0.87)-4, (DISP_HEIGHT*0.6)+20, COLOR_BACKGROUND);
  m_tft.fillRect((DISP_WIDTH/16)+2, (DISP_HEIGHT/4)-20, (DISP_WIDTH*0.87)-4, 20, COLOR_BLUE);
  if (caption)
  {
    setCursorY((DISP_HEIGHT/4)-7);
    outText(caption, true);  
  }  
  
  m_tft.setTextColor(COLOR_BLACK);  
  setCursorY(hasButtons ? (DISP_HEIGHT/2.35)+12 : (DISP_HEIGHT/2)+12);
  outText(content, true);  
  
  // make "parent" bar gray
  m_tft.setTextColor(COLOR_WHITE);
  m_tft.fillRect(DISP_WIDTH/32, DISP_HEIGHT/24, DISP_WIDTH*0.935, 22, COLOR_SHADOW);
  setCursorY((DISP_HEIGHT/24)+15);  
  outText(Progmem::getString(Progmem::uiTitle), true);
  m_tft.setTextColor(COLOR_BLACK);
  
  if (hasButtons)
  {
    setCursorY(DISP_HEIGHT*0.65);
  }
}

void Ui::messageBox(BYTE progmemContent, BYTE progmemCaption, bool hasButtons)
{  
  m_tft.setTextColor(COLOR_WHITE);
  m_tft.fillRect(DISP_WIDTH/16, (DISP_HEIGHT/4)-22, DISP_WIDTH*0.87, (DISP_HEIGHT*0.6)+24, COLOR_BLACK);
  m_tft.fillRect((DISP_WIDTH/16)+2, (DISP_HEIGHT/4)-20, (DISP_WIDTH*0.87)-4, (DISP_HEIGHT*0.6)+20, COLOR_BACKGROUND);
  m_tft.fillRect((DISP_WIDTH/16)+2, (DISP_HEIGHT/4)-20, (DISP_WIDTH*0.87)-4, 20, COLOR_BLUE);
  setCursorY((DISP_HEIGHT/4)-7);
  outText(Progmem::getString(progmemCaption), true); //0 defaults to Progmem::Empty
  
  m_tft.setTextColor(COLOR_BLACK);  
  setCursorY(hasButtons ? (DISP_HEIGHT/2.35)+12 : (DISP_HEIGHT/2)+12);
  outText(Progmem::getString(progmemContent), true);  
  
  m_tft.setTextColor(COLOR_WHITE);
  m_tft.fillRect(DISP_WIDTH/32, DISP_HEIGHT/24, DISP_WIDTH*0.935, 22, COLOR_SHADOW);
  setCursorY((DISP_HEIGHT/24)+15);
  outText(Progmem::getString(Progmem::uiTitle), true);
  m_tft.setTextColor(COLOR_BLACK);
  
  if (hasButtons)
  {
    setCursorY(DISP_HEIGHT*0.65);
  }
}

void Ui::drawFilePicker(bool rootDirectory, char* entriesPipeDelimited, BYTE curSel, DWORD curPage, DWORD pages)
{
  // shows file filter (*.P32), six file entries per page, indicator and button bar
  const BYTE pickerMaxEntries = 6;
  const WORD pickerHeight = DISP_HEIGHT*0.56;
  
  if (!entriesPipeDelimited)
  {
    return;
  }
  const WORD oldLen = strlen(entriesPipeDelimited);
  
  const bool redrawWhole = !m_filePicking;
  if (redrawWhole)
  {
    clearScreen();       
    m_tft.fillRect(DISP_WIDTH/18, DISP_HEIGHT/6.25, DISP_WIDTH*0.89, pickerHeight, COLOR_BLACK);
    
    const Ui::Button buttonRow[] = { {Ui::ButtonAction::PgUp, Progmem::btnPgUp}, 
                                     {Ui::ButtonAction::PgDn, Progmem::btnPgDn},
                                     {Ui::ButtonAction::Up, Progmem::btnUp},
                                     {Ui::ButtonAction::Down, Progmem::btnDown},
                                     {Ui::ButtonAction::Open, Progmem::btnOpen},
                                     {Ui::ButtonAction::Back, Progmem::btnBack} };

    setCursorY(DISP_HEIGHT*0.82);
    outButtons(buttonRow, BUTTONS_COUNTOF(buttonRow), DISP_WIDTH/8, DISP_HEIGHT/10);
  }
  
  snprintf(m_stringBuffer, sizeof(m_stringBuffer)-1, Progmem::getString(Progmem::uiPickerDetails), curPage, pages);
  setCursorY(DISP_HEIGHT*0.78);
  outText(m_stringBuffer, true, false, !redrawWhole);
  
  m_tft.fillRect((DISP_WIDTH/18)+1, (DISP_HEIGHT/6.25)+1, (DISP_WIDTH*0.89)-2, pickerHeight-2, COLOR_WHITE);
  const BYTE pickerItemHeight = pickerHeight / pickerMaxEntries;
  
  char* entry = strtok(entriesPipeDelimited, "|");
  if (!entry)
  {
    entry = entriesPipeDelimited;
  }
  
  for (WORD item = 1; item <= pickerMaxEntries; item++)
  {
    const WORD itemY = (DISP_HEIGHT/6.25)+(item*pickerItemHeight);
    setCursor((DISP_WIDTH/18)+7, itemY-7);
    
    if (item < pickerMaxEntries)
    {
      m_tft.drawFastHLine((DISP_WIDTH/18)+1, itemY, (DISP_WIDTH*0.89)-1, COLOR_SHADOW);  
    }
    
    if (curSel == item)
    {
      m_tft.fillRect((DISP_WIDTH/18)+1, itemY-21, (DISP_WIDTH*0.89)-2, pickerItemHeight, COLOR_BROWN);
      m_tft.setTextColor(COLOR_WHITE);
    }
    
    if ((item == 1) && (curPage == 1))
    {
      outText(Progmem::getString(rootDirectory ? Progmem::uiPickerRootDir : Progmem::uiPickerOneLevelUp));
    }
    else if (entry && strlen(entry))
    {
      outText(entry);
      entry = strtok(NULL, "|");
    }
    
    if (curSel == item)
    {
      m_tft.setTextColor(COLOR_BLACK);
    }
    
    m_filePickerCount = item;
    if (!entry || !strlen(entry))
    {
      break;
    }
  }
  
  // restore pipe delimiters modified by strtok
  for (WORD index = 0; index < oldLen; index++)
  {
    if (entriesPipeDelimited[index] == 0)
    {
      entriesPipeDelimited[index] = '|';
    }
  }
  
  m_filePicking = true;
}

Ui::ButtonAction Ui::buttonPressed()
{ 
  if (!m_buttonRow || !m_buttonsCount)
  {
    return ButtonAction::NoButton;
  }

  TSPoint pt = m_touch.getPoint();
  const bool dataAvailable = pt.z > 0; // pressure detected
  
  // ignore finger held on touchscreen:
  // only works reliably if the touchscreen is active (XPT2046),
  // or its X/Y pins are not shared together with the TFT data bus 
  static bool held = false;
   
  if (dataAvailable && !held)
  { 
    held = true;
 
#ifdef DISP_FLIP_ORIENTATION
    pt.x = DISP_WIDTH - pt.x;
    pt.y = DISP_HEIGHT - pt.y;
#endif 
        
    for (BYTE at = 0; at < m_buttonsCount; at++)
    {
      const ButtonRow* button = &m_buttonRow[at];
      
      if ((pt.x >= button->X1) && (pt.x <= button->X2) && (pt.y >= button->Y1) && (pt.y <= button->Y2))
      {
        return button->action;
      }
    }
    
    // picked a file
    if (m_filePicking)
    {
      const BYTE pickerItemHeight = DISP_HEIGHT*0.56 / 6;

      for (WORD item = 1; item <= m_filePickerCount; item++)
      {
        const WORD X1 = (DISP_WIDTH/18)+1;
        const WORD Y1 = (DISP_HEIGHT/6.25)+(item*pickerItemHeight)-21;
        const WORD X2 = X1 + (DISP_WIDTH*0.89)-2;
        const WORD Y2 = Y1 + pickerItemHeight;
        
        if ((pt.x >= X1) && (pt.x <= X2) && (pt.y >= Y1) && (pt.y <= Y2))
        {
          m_filePickerSel = item;
          return ButtonAction::FilePicked;
        }
      }
    }
  }

  if (held && !dataAvailable)
  {
    held = false;
  }

  return ButtonAction::NoButton;  
}

// :-)
/*
void Ui::BSOD()
{
  const char windows[] = " Windows ";
  m_tft.fillScreen(COLOR_BLUE);
  setCursor((DISP_WIDTH/2) - (strlen(windows) * 4), DISP_HEIGHT*0.3);
  m_tft.fillRect(m_tft.getCursorX(), m_tft.getCursorY()-13, strlen(windows)*8, 20, COLOR_BACKGROUND);
  m_tft.setTextColor(COLOR_BLUE);
  outText(windows, true);
  m_tft.setTextColor(COLOR_WHITE);
  setCursor(DISP_WIDTH*0.09, DISP_HEIGHT*0.44);
  outText("A fatal exception 0E has occured");
  setCursor(DISP_WIDTH*0.09, DISP_HEIGHT*0.51);
  outText("at address F000:0050. The current");
  setCursor(DISP_WIDTH*0.09, DISP_HEIGHT*0.58);
  outText("process will be terminated.");
  setCursorY(DISP_HEIGHT*0.72);
  outText("Touch screen to continue", true);
  for (;;) {}
}
*/

#endif // TOUCH_SCREEN_CALIBRATION
