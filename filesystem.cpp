// PMD32-Mega2560 (c) 2025 J. Bogin, https://boginjr.com
// Based on PMD32-SD (c) 2012 R. Borik, https://pmd85.borik.net/
// Image filesystem operations

#include "config.h"

#ifndef TOUCH_SCREEN_CALIBRATION

// A: to D:
bool imageMounted[] = {false, false, false, false};
char imagePath[4][MAX_PATH+1] = {0};
File files[4];

bool fsIsDriveMounted(BYTE drive)
{
  if (drive > 3)
  {
    return false;
  }
  
  return imageMounted[drive];
}

bool fsIsFileNameInUse(const char* fileName)
{
  if (!fileName || !mountedDrives)
  {
    return false;
  }
  
  for (BYTE index = 0; index < sizeof(imageMounted); index++)
  {
    if (!imageMounted[index])
    {
      continue;
    }
    if (strcmp(imagePath[index], fileName) == 0)
    {
      return true;
    }
  }
  
  return false;
}

bool fsMount(BYTE drive, BYTE& progmemResult, bool readOnly)
{
  progmemResult = Progmem::Empty;
  if (drive > 3)
  {
    return false;
  }
  
  bool& mount = imageMounted[drive];
  if (!mount)
  {
    if (fsIsFileNameInUse(imagePath[drive]))
    {
      progmemResult = Progmem::uiErrorFileOpened;
      return false;
    }
    
    File& file = files[drive];
    file = sd.open(imagePath[drive], readOnly ? O_RDONLY : O_RDWR);
    if (!file)
    {
      progmemResult = Progmem::uiErrorFileOpen;
      return false;
    }
    
    if (file.fileSize() != 368640L)
    {
      file.close();
      progmemResult = Progmem::uiErrorFileSize;
      return false;
    }
    
    file.rewind();  
    mount = true;
    mountedDrives++;
    return true;
  }
  
  return true;
}

bool fsCreateAndMount(BYTE drive, BYTE& progmemResult)
{
  progmemResult = Progmem::Empty;
  if (drive > 3)
  {
    return false;
  }
  
  bool& mount = imageMounted[drive];
  if (!mount)
  {
    if (fsIsFileNameInUse(imagePath[drive]))
    {
      progmemResult = Progmem::uiErrorFileOpened;
      return false;
    }
    
    progmemResult = Progmem::uiErrorFileCreate;
    File& file = files[drive];
    file = sd.open(imagePath[drive], O_RDWR | O_CREAT | O_TRUNC);
    if (!file)
    {
      return false;
    }
    
    // create 360K image with E5 format fill
    const DWORD imageSize = 368640L;
    const WORD bufSize = 1024;
    BYTE* buf = new BYTE[bufSize];
    if (!buf)
    {
      progmemResult = Progmem::uiErrorMemory;
      file.close();
      return false;
    }
    memset(buf, 0xE5, bufSize);
    
    WORD count = imageSize / bufSize;
    while(count)
    {
      if (!file.write(buf, bufSize))
      {
        delete[] buf;
        file.close();
        return false;
      }      
      count--;
    }
    
    delete[] buf;
    file.sync();
    file.rewind();
    progmemResult = Progmem::Empty;
     
    mount = true;
    mountedDrives++;
    return true;
  }
  
  return true;
}

void fsUnmount(BYTE drive)
{
  if (drive > 3)
  {
    return;
  }
  
  bool& mount = imageMounted[drive];
  if (mount)
  {
    File& file = files[drive];
    file.sync();
    file.close();
    mount = false;
    mountedDrives--;
  }
}

void fsUnmountAll()
{
  if (mountedDrives)
  {
    for (BYTE drive = 0; drive < 4; drive++)
    {
      fsUnmount(drive);
    }
  }
}

char* fsGetImagePath(BYTE drive)
{
  if (drive > 3)
  {
    return NULL;
  }
  
  return &imagePath[drive][0];
}

File* fsGetFile(BYTE drive)
{
  if (drive > 3)
  {
    return NULL;
  }
  
  return &files[drive];
}

void fsStoreDriveToEEPROM(BYTE drive)
{
#ifdef EEPROM_IMAGE_AUTOMOUNT
   
  // structure (index, size in bytes: description)
  // +0, 1:          is drive mounted (0 false, 1 normal, 2 readonly)
  // +1, 1:          8-bit checksum of the whole path buffer
  // +2, MAX_PATH+1: path incl. terminating null
  
  if (drive > 3)
  {
    return;
  }  
  
  WORD offset = (WORD)drive * (MAX_PATH+3);  
  EEPROM.update(offset++, imageMounted[drive] ? (files[drive].isWritable() ? 1 : 2) : 0);
  if (!imageMounted[drive])  
  {
    return;
  }
  
  ui->messageBox(Progmem::uiUpdatingEEPROM, Progmem::uiBusy, false);
  BYTE checkSum = 0;
  const WORD checkSumOffset = offset++;
  
  for (WORD index = 0; index < sizeof(imagePath[drive]); index++)
  {
    BYTE ch = (BYTE)imagePath[drive][index];
    EEPROM.update(offset++, ch);
    checkSum += ch;
  }  
  EEPROM.update(checkSumOffset, checkSum);
  
#endif // EEPROM_IMAGE_AUTOMOUNT
}

void fsAutoLoadImagesFromEEPROM()
{
#ifdef EEPROM_IMAGE_AUTOMOUNT

  ui->messageBox(Progmem::uiLoadingEEPROM, Progmem::uiBusy, false);
  
  for (BYTE drive = 0; drive < 4; drive++)
  {
    WORD offset = (WORD)drive * (MAX_PATH+3);
    const BYTE mounted = EEPROM.read(offset++);
    if (!mounted || (mounted > 2))
    {
      // not mounted or invalid
      continue;
    }
       
    const BYTE checkSum = EEPROM.read(offset++);
    BYTE calculated = 0;
    for (WORD index = 0; index < sizeof(imagePath[drive]); index++)
    {
      const BYTE ch = EEPROM.read(offset++);      
      calculated += ch;
      imagePath[drive][index] = ch;
    }
            
    // invalid or path not existent
    if ((checkSum != calculated) || !sd.exists(imagePath[drive]))
    {
      imagePath[drive][0] = 0;
      continue;
    }   
    
    // try to mount, dismiss errors
    BYTE dummy;
    fsMount(drive, dummy, mounted == 2); // readonly?
  }
  
  ui->clearScreen();
  
#endif // EEPROM_IMAGE_AUTOMOUNT
}

#endif // TOUCH_SCREEN_CALIBRATION