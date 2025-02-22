// PMD32-Mega2560 (c) 2025 J. Bogin, https://boginjr.com
// Based on PMD32-SD (c) 2012 R. Borik, https://pmd85.borik.net/
// Entry point

#include "config.h"

#ifndef TOUCH_SCREEN_CALIBRATION

Ui* ui;
PMD32 pmd;
SdFat sd;

BYTE cardStatus;         // 0: undefined, 1: no card, 2: unreadable card, 3: card ready, 4: asked to eject
BYTE uiStatus;           // 0: idle, 1: mounting drives, 2: create new image
BYTE mountedDrives;      // number of drives mounted
BYTE selectedDrive;      // 0 to 3 => A to D

BYTE filePickerSel;      // 1-based item index, 0: nothing selected
BYTE filePickerPage;     // 1-based, current page
BYTE filePickerPages;    // - || - , total pages
char filePickerBuf[MAX_PATH + 1] = {0};

bool DetectCard(bool& firstRun);
void CardAndDriveDetails();
void ProcessUI();
void ProcessPMD32();
void DoDrivePicker(Ui::Button* buttonRow, bool mount, bool create = false);
void DoFilePicker(bool calculateTotalPages = false, BYTE convertSelToFileName = 0, bool* selIsDirectory = NULL);

void setup()
{
  ui = Ui::get();
}

void loop()
{
  cardStatus = 0;
  uiStatus = 0;
  mountedDrives = 0;
  bool firstRun = true;
  
  while(true)
  {   
    if (!DetectCard(firstRun))
    {
      fsUnmountAll();
      uiStatus = 0;      
      continue;
    }
    
    ProcessPMD32();
    ProcessUI();    
  }
}

bool DetectCard(bool& firstRun)
{ 
  DWORD ocr = 0;
  
  if (cardStatus == 3)
  {
    // was in, but disconnected without asking to eject first?
    // operating conditions register cannot be read or card not ready
    if (!sd.card()->readOCR(&ocr) || !(ocr & 0x80000000))
    {
      ui->clearScreen();
      ui->outText(Progmem::getString(Progmem::uiNoCardPresent), true, true);
      cardStatus = 1;   
      
      return false;
    }
  }

  // init card and volume  
  if (cardStatus < 3)
  {
    
#ifndef SD_SOFTWARE_SPI
    static SdSpiConfig cfg(SD_HWSPI_CS, USER_SPI_BEGIN);
#else
    static SoftSpiDriver<SD_SWSPI_MISO, SD_SWSPI_MOSI, SD_SWSPI_CLK> spi;
    static SdSpiConfig cfg(SD_SWSPI_CS, USER_SPI_BEGIN, SD_SCK_MHZ(0), &spi);
#endif

    if ((sd.cardBegin(cfg)) && sd.card()->sectorCount())
    {     
      if (!sd.volumeBegin())
      {
        sd.end();
        if (cardStatus != 2)
        {
          ui->outText(Progmem::getString(Progmem::uiUnsupportedFS), true, true, true);
          cardStatus = 2;
        }
        
        return false;
      }
    }
    else
    {
      if (cardStatus != 1)
      {
        ui->outText(Progmem::getString(Progmem::uiNoCardPresent), true, true, true);
        cardStatus = 1;  
      }
      
      return false;
    }
  }
  
  // asked to eject
  else if (cardStatus == 4)
  {
    if (!sd.card()->readOCR(&ocr) || !(ocr & 0x80000000))
    {
      // card now cannot be detected
      ui->outText(Progmem::getString(Progmem::uiNoCardPresent), true, true, true);
      cardStatus = 1;
    }
    
    sd.end();
    return false;
  }
  
  // passed checks, card is now in
  if (cardStatus != 3)
  {   
    if (firstRun)
    {
      fsAutoLoadImagesFromEEPROM(); // if built with, see config.h
      
      // if nothing was mounted to A: on first run, check and autoload system.p32
      const char systemImage[] = "/system.p32";
      
      if (!fsIsDriveMounted(0) && sd.exists(systemImage))
      {
        BYTE dummyResult;
        strcpy(fsGetImagePath(0), systemImage);
        fsMount(0, dummyResult, true); // readonly
      }
      
      firstRun = false;
    }

    CardAndDriveDetails();
    cardStatus = 3;
  }
  
  return true;
}

void CardAndDriveDetails()
{
  // in powers of ten, as manufacturers of storage media always do...
  const float capacityMB = sd.card()->sectorCount() / 1953.125;
  const WORD capacityGB = round(capacityMB / 1000); // to nearest (as usual)
  
  char sdType[3] = {0};
  switch (sd.card()->type())
  {
  case SD_CARD_TYPE_SDHC:
    strncpy(sdType, (capacityGB > 32) ? "XC" : "HC", sizeof(sdType)-1);
    break;
  case SD_CARD_TYPE_SD1:
  case SD_CARD_TYPE_SD2:
  default:
    break; // just microSD, don't distinguish between versions
  }

  snprintf(Ui::m_stringBuffer, sizeof(Ui::m_stringBuffer)-1, Progmem::getString(Progmem::uiCardDetails),
           (capacityMB > 999) ? capacityGB : (WORD)round(capacityMB),
           (capacityMB > 999) ? "GB" : "MB",
           sdType);
  
  ui->outText("", true, true, true);
  ui->setCursorY(DISP_HEIGHT*0.43);
  ui->outText(Ui::m_stringBuffer, true);
  
  snprintf(Ui::m_stringBuffer, sizeof(Ui::m_stringBuffer)-1, Progmem::getString(Progmem::uiMountedDrives), mountedDrives);
  ui->setCursorY(DISP_HEIGHT*0.53);
  ui->outText(Ui::m_stringBuffer, true);
  
  // draw and link buttons
  ui->setCursorY(DISP_HEIGHT*0.77);
  
  if (!mountedDrives)
  {
    const Ui::Button buttonRow[] = { {Ui::ButtonAction::Mount, Progmem::btnMount}, 
                                     {Ui::ButtonAction::Create, Progmem::btnCreate}, 
                                     {Ui::ButtonAction::Eject, Progmem::btnEject} };
    ui->outButtons(buttonRow, BUTTONS_COUNTOF(buttonRow), DISP_WIDTH/4.6, DISP_HEIGHT/7.5);
  }
  else if (mountedDrives < 4)
  {
    const Ui::Button buttonRow[] = { {Ui::ButtonAction::Mount, Progmem::btnMount}, 
                                     {Ui::ButtonAction::Unmount, Progmem::btnUnmount},
                                     {Ui::ButtonAction::Create, Progmem::btnCreate},
                                     {Ui::ButtonAction::Eject, Progmem::btnEject} };
    ui->outButtons(buttonRow, BUTTONS_COUNTOF(buttonRow), DISP_WIDTH/4.6, DISP_HEIGHT/7.5);
  }
  else
  {
    const Ui::Button buttonRow[] = { {Ui::ButtonAction::Unmount, Progmem::btnUnmount},
                                     {Ui::ButtonAction::Eject, Progmem::btnEject} };
    ui->outButtons(buttonRow, BUTTONS_COUNTOF(buttonRow), DISP_WIDTH/4.6, DISP_HEIGHT/7.5);
  }
}

// "spider" of UI button actions
void ProcessUI()
{
  Ui::ButtonAction action = ui->buttonPressed();
  if (action == Ui::ButtonAction::NoButton)
  {
    return;
  }
        
  if (uiStatus == 0) // idle
  {
    if ((action == Ui::ButtonAction::Mount) || (action == Ui::ButtonAction::Create))
    {
      Ui::Button* buttonRow = new Ui::Button[5 - mountedDrives]; // include "Back" button
      if (buttonRow)
      {
        DoDrivePicker(buttonRow, true, (action == Ui::ButtonAction::Create));        
        delete[] buttonRow;
        
        uiStatus = (action == Ui::ButtonAction::Create) ? 2 : 1;
      }
    }      
    else if (action == Ui::ButtonAction::Unmount)
    {
      Ui::Button* buttonRow = new Ui::Button[mountedDrives + 1];
      if (buttonRow)
      {
        DoDrivePicker(buttonRow, false);        
        delete[] buttonRow;
      }
    }
    else if (action == Ui::ButtonAction::Eject) // eject card gracefully, unmount and flush files
    {
      fsUnmountAll();
      sd.end();
      ui->clearScreen();
      ui->outText(Progmem::getString(Progmem::uiCardSafeToEject), true, true);
      
      cardStatus = 4;
    }
    
    // unmounting
    else
    {
      if ((action >= Ui::ButtonAction::DriveA) && (action <= Ui::ButtonAction::DriveD))
      {
        selectedDrive = (action - Ui::ButtonAction::DriveA);
        fsUnmount(selectedDrive);
        fsStoreDriveToEEPROM(selectedDrive); // if enabled
      }
      
      ui->clearScreen();
      CardAndDriveDetails();
    }
  }
  
  else if (uiStatus == 1) // mounting
  {
    if ((action >= Ui::ButtonAction::DriveA) && (action <= Ui::ButtonAction::DriveD))
    {
      selectedDrive = (action - Ui::ButtonAction::DriveA);
      strcpy(fsGetImagePath(selectedDrive), "/");      
      DoFilePicker(true); // calc number of pages
    }
    
    // file picker buttons
    else if (action == Ui::ButtonAction::PgUp)
    {
      if (filePickerPage == 1)
      {
        return;
      }
      
      filePickerSel = 0;
      filePickerPage--;        
    }
    else if (action == Ui::ButtonAction::PgDn)
    {
      if (filePickerPage == filePickerPages)
      {
        return;
      }
      
      filePickerSel = 0;
      filePickerPage++;
    }
    else if (action == Ui::ButtonAction::Up)
    {
      if (filePickerSel == 0)
      {
        filePickerSel = ui->getFilePickerCount();
      }
      else if (filePickerSel == 1)
      {
        // can we go page up? select last from the previous page
        if (filePickerPage > 1)
        {
          filePickerSel = 6;
          filePickerPage--;
        }
        else
        {
          return;  
        }        
      }
      else
      {
        filePickerSel--;  
      }
    }
    else if (action == Ui::ButtonAction::Down)
    {
      if (filePickerSel == ui->getFilePickerCount())
      {
        // one page down
        if (filePickerPage < filePickerPages)
        {
          filePickerSel = 1;
          filePickerPage++;
        }
        else
        {
          return;  
        }        
      }
      else
      {
        filePickerSel++;  
      }
    }
    else if (action == Ui::ButtonAction::Open)
    {
      char* path = fsGetImagePath(selectedDrive);      
      if (!filePickerSel || !path)
      {
        return;
      }
      
      const bool isRoot = strrchr(path, '/') == path;      
      if ((filePickerSel == 1) && (filePickerPage == 1)) // [.] or [..]
      {
        // [.], root directory
        if (isRoot)
        {
          return;
        }
        
        // [..], go one level up
        else
        {
          path[strlen(path)-1] = 0;                   // "/1/" => "/1"        
          char* lastSlash = strrchr(path, '/');
          if (lastSlash)
          {
            lastSlash[1] = 0;                         // "/1" => "/"
          }

          DoFilePicker(true);                         // update cwd and page count
        }        
      }
      
      // decode selected index, check path depth
      else
      {
        bool isDirectory = false;      
        DoFilePicker(false, filePickerSel, &isDirectory);
        
        if ((strlen(path) + strlen(filePickerBuf) + 1) > MAX_PATH)
        {
          ui->messageBox(Progmem::uiErrorPath, Progmem::uiError);

          const Ui::Button buttonRow[] = { {Ui::ButtonAction::OK, Progmem::btnOK} };
          ui->outButtons(buttonRow, BUTTONS_COUNTOF(buttonRow), DISP_WIDTH/3.5, DISP_HEIGHT/7.5);
          return;
        }
        
        // one level down, update cwd
        if (isDirectory)
        {
          strcat(path, filePickerBuf);
          strcat(path, "/");
          DoFilePicker(true);
        }
        
        // ask whether to mount read only
        else
        {
          ui->messageBox(Progmem::uiMountReadOnly, Progmem::uiMountCaption);

          const Ui::Button buttonRow[] = { {Ui::ButtonAction::Yes, Progmem::btnYes},
                                           {Ui::ButtonAction::No,  Progmem::btnNo},
                                           {Ui::ButtonAction::Cancel,  Progmem::btnCancel} };
          ui->outButtons(buttonRow, BUTTONS_COUNTOF(buttonRow), DISP_WIDTH/4, DISP_HEIGHT/7.5);
          return;
        }        
      }
    }
    
    // image picked, Yes/No to mount as read only
    else if ((action == Ui::ButtonAction::Yes) || (action == Ui::ButtonAction::No))
    {
      char* path = fsGetImagePath(selectedDrive); 
      const BYTE oldLen = strlen(path); // in case the mount fails
      strcat(path, filePickerBuf);
      
      BYTE progmemResult = 0;
      if (fsMount(selectedDrive, progmemResult, action == Ui::ButtonAction::Yes))
      {
        fsStoreDriveToEEPROM(selectedDrive); // if enabled
        
        uiStatus = 0;
        ui->clearScreen();
        CardAndDriveDetails();
        return;
      }
      else if (progmemResult)
      {
        ui->messageBox(progmemResult, Progmem::uiError);

        const Ui::Button buttonRow[] = { {Ui::ButtonAction::OK, Progmem::btnOK} };
        ui->outButtons(buttonRow, BUTTONS_COUNTOF(buttonRow), DISP_WIDTH/3.5, DISP_HEIGHT/7.5);
      }
      
      // strip filename from path before the file picker redraws
      path[oldLen] = 0;
      return;
    }
    else if (action == Ui::ButtonAction::Back)
    {
      uiStatus = 0;
      ui->clearScreen();
      CardAndDriveDetails();
      return;
    }
    
    // update selection highlight
    else if (action == Ui::ButtonAction::FilePicked)
    {
      filePickerSel = ui->getFilePickerSel();
    }
    
    // OK/Cancel buttons used as message box dismiss, show file picker again
    else if ((action == Ui::ButtonAction::OK) || (action == Ui::ButtonAction::Cancel))
    {  
      ui->clearScreen();
    }
    
    // fallthrough
    else
    {
      return;
    }
    
    // draw
    DoFilePicker();
  }
  
  else if (uiStatus == 2) // creating new image
  {
    char fileName[] = "/driveX.p32";
    
    if ((action >= Ui::ButtonAction::DriveA) && (action <= Ui::ButtonAction::DriveD))
    {
      selectedDrive = (action - Ui::ButtonAction::DriveA);
      fileName[6] = 'A' + selectedDrive;
      
      // ask to confirm
      snprintf(Ui::m_stringBuffer, sizeof(Ui::m_stringBuffer)-1, Progmem::getString(Progmem::uiCreateConfirm), fileName);
      ui->messageBox(Ui::m_stringBuffer, Progmem::getString(Progmem::uiCreateCaption));
      
      const Ui::Button buttonRow[] = { {Ui::ButtonAction::Yes, Progmem::btnYes},
                                       {Ui::ButtonAction::No,  Progmem::btnNo} };
      ui->outButtons(buttonRow, BUTTONS_COUNTOF(buttonRow), DISP_WIDTH/3.5, DISP_HEIGHT/7.5);
    }
    
    else if (action == Ui::ButtonAction::Yes)
    {
      ui->messageBox(Progmem::uiBusy, Progmem::uiCreateCaption, false);
      
      fileName[6] = 'A' + selectedDrive;
      strncpy(fsGetImagePath(selectedDrive), fileName, MAX_PATH);
      
      BYTE progmemResult = 0;
      if (fsCreateAndMount(selectedDrive, progmemResult))
      {
        fsStoreDriveToEEPROM(selectedDrive); // if enabled
        
        uiStatus = 0;
        ui->clearScreen();
        CardAndDriveDetails();
        return;
      }
      else if (progmemResult)
      {
        ui->messageBox(progmemResult, Progmem::uiError);

        const Ui::Button buttonRow[] = { {Ui::ButtonAction::OK, Progmem::btnOK} };
        ui->outButtons(buttonRow, BUTTONS_COUNTOF(buttonRow), DISP_WIDTH/3.5, DISP_HEIGHT/7.5);
      }
    }
    
    else
    {
      uiStatus = 0;
      ui->clearScreen();
      CardAndDriveDetails(); 
    }
  }
}

void ProcessPMD32()
{     
  // extra commands of PMD32-SD (i.e. CD.COM) can change this status
  const BYTE old = mountedDrives;
  
  // update the UI only after last command was processed and there were no more coming
  const WORD threshold = 150; // ms
  DWORD wait = 0;
  
  while (true)
  {
    const bool processed = pmd.processCommand();
    
    if (!processed)
    {
      if (!wait || ((millis() - wait) > threshold))
      {
        break;
      }
    }
    else
    {
      wait = millis();
    }
  }
  
  if ((mountedDrives != old) && (uiStatus == 0)) // refresh idle page if we're on it
  {
    ui->clearScreen();
    CardAndDriveDetails();
  }
}

void DoDrivePicker(Ui::Button* buttonRow, bool mount, bool create)
{
  if (!buttonRow)
  {
    return;
  }
  
  if (create)
  {
    ui->messageBox(Progmem::uiCreateQuestion, Progmem::uiCreateCaption);
  }
  else
  {
    ui->messageBox(mount ? Progmem::uiMountQuestion : Progmem::uiUnmountQuestion, 
                   mount ? Progmem::uiMountCaption : Progmem::uiUnmountCaption);  
  }
  
  BYTE button = 0;  
  for (BYTE drive = 0; drive < 4; drive++)
  {
    bool addButton = fsIsDriveMounted(drive);
    if (mount || create)
    {
      addButton = !addButton;
    }
    
    if (addButton)
    {
      buttonRow[button].progmemCaption = Progmem::btnDriveA + drive;
      buttonRow[button].action = Ui::ButtonAction::DriveA + drive;            
      button++;
    }        
  }
  buttonRow[button].progmemCaption = Progmem::btnBack;
  buttonRow[button].action = Ui::ButtonAction::Back;
  button++;

  ui->outButtons(buttonRow, button, DISP_WIDTH/7, DISP_HEIGHT/7.5);
}

void DoFilePicker(bool calculateTotalPages, BYTE convertSelToFileName, bool* selIsDirectory)
{
  // calculateTotalPages TRUE: filters out cwd for *.P32's, other directories and returns number of 6-entry pages to filePickerPages
  // convertSelToFilename NONZERO: converts the sel item index to non-display-shortened file name (in the cwd), returned to filePickerBuf
  //                               selIsDirectory optional: outputs true if this was a directory
  // no arguments: prepare pipe-delimited filePickerBuf and draw via ui->drawFilePicker()
  // cwd: current working directory (fsGetImagePath), all paths MAX_PATH
  
  // reset selection and page counts
  if (calculateTotalPages)
  {
    filePickerPage = 1;
    filePickerPages = 1;
    filePickerSel = 0;
  }
   
  const char* strPath = fsGetImagePath(selectedDrive);
  if (!strPath)
  {
    return;
  }  
  const bool isRoot = strrchr(strPath, '/') == strPath;
   
  File path = sd.open(strPath, O_RDONLY);
  if (!path)
  {
    ui->messageBox(Progmem::uiErrorFS, Progmem::uiError);
    
    const Ui::Button buttonRow[] = { {Ui::ButtonAction::Back, Progmem::btnBack} };
    ui->outButtons(buttonRow, BUTTONS_COUNTOF(buttonRow), DISP_WIDTH/3.5, DISP_HEIGHT/7.5);
    return;
  }
  
  filePickerBuf[0] = 0;
  const BYTE pickerEntries = 6;
  BYTE pickerEntry = (filePickerPage == 1) ? 2 : 1; // leave room for [.] / [..] if on 1st page
  DWORD totalFilterCount = 1;
    
  while (pickerEntry <= pickerEntries)
  {
    File file = path.openNextFile(O_RDONLY);
    if (!file)
    {
      break;
    }

    if (file.isHidden())
    {
      file.close();
      continue;
    }
    
    // filter *.p32
    char name[MAX_PATH + 1];
    name[0] = 0;
    file.getName(name, MAX_PATH);  
    bool isDirectory = file.isDir() || file.isSubDir();
    file.close();
    if (!strlen(name))
    {
      continue;
    }    
    if (!isDirectory)
    {
      const char* ext = strcasestr(name, ".p32");
      if (!ext)
      {
        continue;
      }
      
      // also ending with it?
      const BYTE extPos = ext-&name[0];
      if (extPos != strlen(name)-4)
      {
        continue;
      }
    }
    
    totalFilterCount++;
    if (calculateTotalPages)
    {
      continue; // until !file
    }
    
    const DWORD currentPage = ((totalFilterCount-1) / (DWORD)pickerEntries) + 1;
    if (filePickerPage != currentPage)
    {
      continue;
    }
    
    if (convertSelToFileName)
    {      
      if (convertSelToFileName == pickerEntry)
      {
        strncpy(filePickerBuf, name, MAX_PATH); // unshortened
        
        if (selIsDirectory)
        {
          *selIsDirectory = isDirectory;
        }        
        return;
      }
      
      pickerEntry++;
      continue;
    }
    
    // continue with picker
    // trim long file/dir names for the display
    const BYTE maxShownNameLen = file.isDir() ? 30 : 32; // 32 chars (30 for directories inside brackets)  
    if (isDirectory)
    { 
      if (strlen(name) > maxShownNameLen)
      {
        strcpy(&name[maxShownNameLen-3], "...");        
      }
      
      strcat(filePickerBuf, "[");
      strcat(filePickerBuf, name);
      strcat(filePickerBuf, "]");
    }
    else
    {
      if (strlen(name) > maxShownNameLen)
      {
        strcpy(&name[maxShownNameLen-6], "...p32");
      }
      strcat(filePickerBuf, name);
    }  
    
    if (pickerEntry != pickerEntries)
    {
      strcat(filePickerBuf, "|");
    }
    
    pickerEntry++;
  }
  
  if (calculateTotalPages)
  {
    filePickerPages = ((totalFilterCount-1) / (DWORD)pickerEntries)+1;
    return;
  }
  
  ui->drawFilePicker(isRoot, filePickerBuf, filePickerSel, filePickerPage, filePickerPages);
}

#endif // TOUCH_SCREEN_CALIBRATION