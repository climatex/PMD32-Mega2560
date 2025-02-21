// PMD32-Mega2560 (c) 2025 J. Bogin, https://boginjr.com
// Based on PMD32-SD (c) 2012 R. Borik, https://pmd85.borik.net/
// PMD32 drive emulation with PMD32-SD extras

#include "config.h"

#ifndef TOUCH_SCREEN_CALIBRATION

PMD32::PMD32()
{
  // initialize control lines: DIR, /ACK, /STB output high, IBF, /OBF input pullup
  PMD_CTRL_DDR = 0x15;
  PMD_CTRL_OUT = 0x1F;
  
  // data lines input high-impedance
  PMD_DATA_DDR = 0;
  PMD_DATA_OUT = 0;
  
  m_CRC = 0; // 8-bit XOR
  m_hostResponding = false; // accepting commands
  
  // sector data buffer, and for string I/O
  memset(m_ioBuffer, 0, sizeof(m_ioBuffer));
  
  // PMD32-SD path string for use with CD.COM that keeps current working directory
  memset(m_cwdPath, 0, sizeof(m_cwdPath));
  m_cwdPath[0] = '/';
}

bool PMD32::processCommand()
{  
  // do one; returns true if there was a command to process, false otherwise
  BYTE command = 0;
 
  // exchange "is-present" byte if communication yet not established, or read command timeout
  if (!m_hostResponding)
  {
    if (sendByte(PMD32_IDLE, TIMEOUT_SEND_IDLE))
    {
      if (readByte(command, TIMEOUT_READ_IDLE))
      {
        if (command == PMD32_IDLE)
        {
          m_hostResponding = true;      
        }
      }
    }
    
    if (!m_hostResponding)
    {
      return false;
    }
  }  
  
  if (!readByte(command, TIMEOUT_READ_CMD))
  {
    m_hostResponding = false;
    return false;
  }
  if (command == PMD32_IDLE)
  {
    return false;
  }
  
  m_CRC = command; // command byte also part of CRC
  
  switch(command)
  {
    
  // standard PMD32 drive commands  
  case PMD32_READ_BOOT:
    doRWOperation(false, false, true, 128);  // read first 128 bytes of A:
    break;
  case PMD32_READ_LOGICAL1:
  case PMD32_READ_LOGICAL2:
    doRWOperation(false, false, false, 128); // read 128 bytes
    break;
  case PMD32_WRITE_LOGICAL1:
  case PMD32_WRITE_LOGICAL2:
    doRWOperation(true, false, false, 128);  // write 128 bytes
    break;
  case PMD32_WRITE_PHYSICAL:
    doRWOperation(true, false, false, 512);  // write 512 bytes, 513 supplied
    break;
  case PMD32_FORMAT_TRACK:
    doRWOperation(false, true, false, 0);    // format track on one side
    break;
  case PMD32_CHANGE_DRIVE:
    changeDrive();                           // originally, drive select and recal to track 0
    break;
  case PMD32_READ_RAM:
    dummyCommand(4, 2);
    break;
  case PMD32_WRITE_RAM:
    dummyCommand(5);
    break;
  case PMD32_EXECUTE_RAM:
    dummyCommand(2);
    break;
  case PMD32_SLOW_MODE:
  case PMD32_FAST_MODE:
    dummyCommand();
    break;
    
  // PMD32-SD extra commands, used by CD.COM and other utilities
  case PMD32_GET_IMAGE_PATH:
    extraGetImagePath();
    break;
  case PMD32_MOUNT_IMAGE:
    extraMountImage();
    break;
  case PMD32_GET_CWD:
    extraGetCurrentWorkingDirectory();
    break;
  case PMD32_DIR_LISTING:
    extraDoDirectoryListing();
    break;
  case PMD32_CHANGE_CWD:
    extraChangeCurrentWorkingDirectory();
    break;
  case PMD32_CREATE_IMAGE:
    extraCreateImage();
    break;
  case PMD32_IMAGE_INFO:
    extraImageInfo();
    break;
    
  // unrecognized
  default:
    sendByte(PMD32_NAK, TIMEOUT_SEND_NAK);
    return false;
  }
  
  return true;
}

void PMD32::doRWOperation(bool write, bool format, bool readBootSector, WORD bytes)
{
  // sanity check :-)
  const BYTE check = (BYTE)write + (BYTE)format + (BYTE)readBootSector;
  if ((check > 1) || (bytes > sizeof(m_ioBuffer)) || ((bytes == 0) && !format))
  {
    return;
  }
  
  BYTE data;
  
  // default if reading bootsector
  BYTE drive = 0;
  BYTE sector = 0;
  BYTE track = 0;
     
  // other read, write, format
  if (!readBootSector)
  {
    // determine drive and sector number
    if (!readByte(data))
    {
      return;
    }  
    
    // drive number: 2 MSB; numbers 1 and 2 swapped due to bit 6 originally reserved to zero
    drive = data >> 6;    
    if ((drive == 1) || (drive == 2))
    {
      drive ^= 3;
    }
        
    // sector number: bits 0-5 (128B) 2-5 (512B)
    sector = (bytes == 128) ? data & 0x3F : data & 0x3C;
       
    // track number
    if (!readByte(track))
    {
      return;
    }
    
    // prepare write buffer if writing
    if (write)
    {
      WORD index = 0;
      for (; index < bytes; index++)
      {
        if (!readByte(m_ioBuffer[index]))
        {
          return;
        }
      }
      
      // "write physical sector", one extra byte being received for some reason
      if (index == 512)
      {
        if (!readByte(data))
        {
          return;
        }
      }
    }
  }
  
  // verify CRC and send ACK
  if (!readByte(data, TIMEOUT_READ, true))
  {
    return;
  }
  
  // assume I/O error
  if (write)
  {
    data = PMD32_WRITE_ERROR;
  }
  else if (format)
  {
    data = PMD32_FORMAT_ERROR;
  }
  else // read, read bootsector
  {
    data = PMD32_READ_ERROR;
  }
  
  File* file = fsGetFile(drive);
  if (file && file->isOpen())
  {
    // P32: 360K (40 tracks per side, 80 total, 36 sectors of 128B)
    DWORD offset = (DWORD)track * 36L * 128L;
    if (!format)
    {
      offset += (DWORD)sector * 128L; // if not formatting, also add current sector to offset
    }    
    file->seekSet(offset);
    
    if (write)
    {
      if (!file->isWritable()) // read-only or write protected
      {
        data = PMD32_WRITE_PROTECT;
      }      
      else if (file->write(m_ioBuffer, bytes) == bytes) // attempt to write
      {
        data = PMD32_OK;
      }
    }
    
    else if (format)
    {
      if (!file->isWritable())
      {
        data = PMD32_WRITE_PROTECT;
      }
      else // fill whole 36x128B with 0xE5 format fill
      {
        for (WORD index = 0; index < 36*128; index++)
        {
          if (file->write(0xE5) != 1)
          {
            break;
          }
        }
        
        file->sync();
        data = PMD32_OK;
      }
    }
    
    else // read, read bootsector
    {
      if (file->read(m_ioBuffer, bytes) == bytes)
      {
        data = PMD32_OK;
      }
    }   
  }
  else
  {
    // drive not mounted or invalid number
    data = PMD32_INVALID_DRIVE;
  }
  
  // nothing follows if the I/O operation failed
  if (!sendByte(data, TIMEOUT_SEND_RESULT) || (data != PMD32_OK))
  {
    return;
  }
  
  // read, read bootsector - pass on buffer and CRC
  if (!write && !format)
  {
    m_CRC = 0;
    
    for (WORD index = 0; index < bytes; index++)
    {
      data = m_ioBuffer[index];
      m_CRC ^= data;
      if (!sendByte(data))
      {
        return;
      }
    }
    
    sendByte(m_CRC);
  }
}

void PMD32::changeDrive()
{
  BYTE drive;
  if (!readByte(drive))
  {
    return;
  }
  BYTE data;
  if (!readByte(data, TIMEOUT_READ, true)) //CRC + ACK/NAK
  {
    return;
  }
  
  data = PMD32_INVALID_DRIVE;
  File* file = fsGetFile(drive);
  if (file && file->isOpen())
  {
    data = PMD32_OK;
  }
  
  sendByte(data, TIMEOUT_SEND_RESULT);
}

void PMD32::dummyCommand(BYTE inputArgumentsCount, BYTE outputZerosCount)
{
  // for known unsupported commands:
  // reads the rest of supplied arguments followed by CRC byte,
  // then sends ACK and zero bytes of outputZeros count - without doing anything
  // normally, does not fail with NAK unless the communication is broken
  
  BYTE dummy;  
  while (inputArgumentsCount)
  {
    inputArgumentsCount--;
    if (!readByte(dummy))
    {
      return;
    }
  }
  
  // CRC + ACK
  if (!readByte(dummy, TIMEOUT_READ, true))
  {
    return;
  }
  
  while (outputZerosCount)
  {
    outputZerosCount--;
    if (!sendByte(0))
    {
      return;
    }
  }
}

bool PMD32::readByte(BYTE& data, DWORD timeout, bool checkCRC)
{
  bool read = false;
  const DWORD timeStart = millis();
  
  while ((millis() - timeStart) <= timeout)
  {
    // /OBF will go low when data is available
    if (PMD_CTRL_IN & 2)
    {
      continue;
    }
    
    // bring /ACK low to set 8255 tristate to output
    PMD_CTRL_OUT ^= 4;
    _delay_us(1);
    data = PMD_DATA_IN; // read
    PMD_CTRL_OUT ^= 4;
    
    read = true;
    break;
  }  
  if (!read) // failed
  {
    if (checkCRC)
    {
      sendByte(PMD32_NAK, TIMEOUT_SEND_NAK);
    }
    return false;
  }
   
  m_CRC ^= data; // XOR rcv'd byte
  if (!checkCRC)
  {
    return true;
  }
  
  // now check if all xor'ed to 0 and ACK/NAK accordingly
  if (m_CRC == 0)
  {
    if (sendByte(PMD32_ACK, TIMEOUT_SEND_ACK))
    {
      return true;
    }
  }
  sendByte(PMD32_NAK, TIMEOUT_SEND_NAK);
  return false;
}

bool PMD32::sendByte(BYTE data, DWORD timeout)
{  
  // DIR low, data lines as output and write  
  PMD_CTRL_OUT &= 0xFE;  
  PMD_DATA_DDR = 0xFF;
  PMD_DATA_OUT = data;  
  
  // strobe /STB
  PMD_CTRL_OUT ^= 0x10;
  _delay_us(1);
  PMD_CTRL_OUT ^= 0x10;
  
  bool result = false;
  const DWORD timeStart = millis();
  while ((millis() - timeStart) <= timeout)
  {
    // IBF went low, accepted
    if (!(PMD_CTRL_IN & 8))
    {
      result = true;
      break;
    }
  }
  
  // data lines hi-impedance, DIR high
  PMD_DATA_DDR = 0;
  PMD_DATA_OUT = 0;
  PMD_CTRL_OUT |= 1;
  
  return result;  
}

// *********************************************************************** PMD32-SD extra functions *********************************************************************** 

void PMD32::extraSendMaxLengthString(BYTE maxLength, const char* str)
{
  if (!str)
  {
    return;
  }
 
  // send length
  maxLength = (strlen(str) > maxLength) ? maxLength : strlen(str);
  m_CRC ^= maxLength;
  if (!sendByte(maxLength))
  {
    return;
  }
 
  // send string  
  for (BYTE index = 0; index < maxLength; index++)
  {
    m_CRC ^= str[index];
    if (!sendByte(str[index]))
    {
      return;
    }
  }
  
  // send CRC
  sendByte(m_CRC);  
}

void PMD32::extraGetImagePath()
{
  BYTE drive;
  if (!readByte(drive))
  {
    return;
  }
  BYTE data;
  if (!readByte(data, TIMEOUT_READ, true))
  {
    return;
  }  
  m_CRC = 0;  
  
  File* file = fsGetFile(drive);  
  if (file)
  {
    if (file->isOpen())
    {
      // ERR 0
      if (!sendByte(PMD32_OK, TIMEOUT_SEND_RESULT))
      {
        return;
      } 
      
      // WP
      data = file->isWritable() ? 0 : 1;
      m_CRC ^= data;
      if (!sendByte(data))
      {
        return;
      }
           
      //PMD32-Mega2560 has MAX_PATH defined 255, but truncate this for CD.COM to 63 bytes
      const char* path = fsGetImagePath(drive) + 1; // skip the root '/'
      extraSendMaxLengthString(63, path);
    }
    else
    {
      // drive not mounted
      if (!sendByte(PMD32_OK, TIMEOUT_SEND_RESULT)) // ERR 0
      {
        return;
      }
      
      sendByte(0); // not write protect
      sendByte(0); // LEN 0 - no image 
      sendByte(0); // CRC 0
    }
  }
  else
  {
    // ERR invalid drive number
    sendByte(PMD32_INVALID_DRIVE, TIMEOUT_SEND_RESULT);
  }
}

void PMD32::extraMountImage()
{
  BYTE drive;
  BYTE readOnly;
  BYTE length;
  
  if (!readByte(drive))
  {
    return;
  } 
  if (!readByte(readOnly))
  {
    return;
  }
  if (!readByte(length))
  {
    return;
  }
  
  BYTE data = 0;
  
  // prepend current path to given file
  m_ioBuffer[0] = 0;  
  strcpy(m_ioBuffer, m_cwdPath);
  const WORD startIndex = strlen(m_ioBuffer);
  
  // 0 or FF do not follow with path string
  if ((length != 0) && (length != 0xFF))
  {
    WORD index = startIndex;
    for (; index < ((WORD)length+startIndex); index++)
    {
      if (!readByte(m_ioBuffer[index]))
      {
        return;
      }
    }
    
    m_ioBuffer[index] = 0; // make sure the string is ended
  }  
  
  if (!readByte(data, TIMEOUT_READ, true))
  {
    return;
  }
  m_CRC = 0;
  
  if (drive > 3)
  {
    sendByte(PMD32_INVALID_DRIVE, TIMEOUT_SEND_RESULT);
    return;
  }
  
  fsUnmount(drive);  
  if (length == 0) // unmount only
  {
    sendByte(PMD32_OK, TIMEOUT_SEND_RESULT);
    return;
  }
 
  // mount new or remount same image with new O_RDONLY/O_RDWR flag
  if (length != 0xFF)
  {
    strncpy(fsGetImagePath(drive), m_ioBuffer, MAX_PATH);
  }
  
  if (fsMount(drive, data, readOnly))
  {
    sendByte(PMD32_OK, TIMEOUT_SEND_RESULT);
  }
  
  // get error code
  else if (data == Progmem::uiErrorFileOpen)
  {
    sendByte(PMD32_PATH_NOT_FOUND, TIMEOUT_SEND_RESULT);
  }
  else if (data == Progmem::uiErrorFileSize)
  {
    sendByte(PMD32_IMAGE_UNKNOWN, TIMEOUT_SEND_RESULT);
  }
  else
  {
    sendByte(PMD32_NAK, TIMEOUT_SEND_RESULT); // image already mounted, etc
  }
}

void PMD32::extraGetCurrentWorkingDirectory()
{
  BYTE data;
  if (!readByte(data, TIMEOUT_READ, true))
  {
    return;
  }
  m_CRC = 0;
  
  extraSendMaxLengthString(63, &m_cwdPath[1]); // skip the root '/'
}

void PMD32::extraDoDirectoryListing()
{
  BYTE nextEntry;
  if (!readByte(nextEntry))
  {
    return;
  }  
  
  BYTE data;
  if (!readByte(data, TIMEOUT_READ, true))
  {
    return;
  }
  m_CRC = 0;
  
  m_ioBuffer[0] = 0;
  m_ioBuffer[1] = 0;
   
  if (!nextEntry) // first entry
  {
    if (m_dirListing.isOpen())
    {
      m_dirListing.close();
    }
    
    m_dirListing = sd.open(m_cwdPath, O_RDONLY);
    if (!m_dirListing.isOpen())
    {
      sendByte(PMD32_PATH_NOT_FOUND, TIMEOUT_SEND_RESULT);
      return;
    }
    
    // root or one level up
    const bool isRoot = strrchr(m_cwdPath, '/') == m_cwdPath;
    strcpy(m_ioBuffer, isRoot ? "[.]" : "[..]");
  }

  else if (m_dirListing.isOpen()) // next entry
  {
    File file = m_dirListing.openNextFile(O_RDONLY);
    if (file)
    {   
      bool isDirectory = file.isDir() || file.isSubDir();
      const BYTE maxLen = isDirectory ? 61 : 63; // [DIRECTORY] in brackets, 63 char maximum for PMD32-SD
      m_ioBuffer[0] = isDirectory ? '[' : 0;
      file.getName(&m_ioBuffer[isDirectory ? 1 : 0], maxLen); 
      file.close();
      if (isDirectory)
      {
        strcat(m_ioBuffer, "]");
      }
    }
    
    // no more files, close directory listing
    else
    {
      m_dirListing.close();
    }
  }
  
  extraSendMaxLengthString(strlen(m_ioBuffer), m_ioBuffer);
}

void PMD32::extraChangeCurrentWorkingDirectory()
{
  BYTE length;
  if (!readByte(length))
  {
    return;
  }
  
  // prepend current working directory to supplied
  m_ioBuffer[0] = 0;
  strcpy(m_ioBuffer, m_cwdPath);
  const WORD startIndex = strlen(m_ioBuffer);
  
  WORD data = startIndex;  
  for (; data < ((WORD)length+startIndex); data++)
  {
    if (!readByte(m_ioBuffer[data]))
    {
      return;
    }
  }
  m_ioBuffer[data] = 0; // end the string
  
  BYTE dummy;
  if (!readByte(dummy, TIMEOUT_READ, true))
  {
    return;
  }
  m_CRC = 0;
  
  // check ./..
  const char* supplied = &m_ioBuffer[startIndex];
  if (strcmp(supplied, ".") == 0)
  {
    sendByte(PMD32_OK, TIMEOUT_SEND_RESULT); // . supplied, don't do anything
    return;
  }
  else if (strcmp(supplied, "..") == 0) // .., one level up
  {
    const bool isRoot = strrchr(m_cwdPath, '/') == m_cwdPath;
    if (!isRoot)
    {
      m_cwdPath[strlen(m_cwdPath)-1] = 0;
      char* lastSlash = strrchr(m_cwdPath, '/');
      if (lastSlash)
      {
        lastSlash[1] = 0;
      }
    }
    
    sendByte(PMD32_OK, TIMEOUT_SEND_RESULT);
    return;
  }
   
  // end the prepended string with a backslash if it doesn't  
  if (m_ioBuffer[data-1] != '/')
  {
    strcat(m_ioBuffer, "/");
  }
   
  // too long?
  if (strlen(m_ioBuffer) > sizeof(m_cwdPath)-1) // 64, but we send out max 63 chars as the root '/' is skipped
  {
    sendByte(PMD32_PATH_TOO_LONG, TIMEOUT_SEND_RESULT);
    return;
  }
  
  // check if the new working directory exists by doing a chdir operation
  if (!sd.chdir(m_ioBuffer))
  {
    sd.chdir(m_cwdPath); // last good known
    sendByte(PMD32_PATH_NOT_FOUND, TIMEOUT_SEND_RESULT);
    return;
  }  
  
  // OK, update cwd
  strncpy(m_cwdPath, m_ioBuffer, sizeof(m_cwdPath)-1);
  sendByte(PMD32_OK, TIMEOUT_SEND_RESULT);
}

void PMD32::extraCreateImage() // but do not mount
{
  BYTE length;
  if (!readByte(length))
  {
    return;
  }
  
  m_ioBuffer[0] = 0;
  strcpy(m_ioBuffer, m_cwdPath);
  const WORD startIndex = strlen(m_ioBuffer);
  
  WORD data = startIndex;  
  for (; data < ((WORD)length+startIndex); data++)
  {
    if (!readByte(m_ioBuffer[data]))
    {
      return;
    }
  }
  m_ioBuffer[data] = 0;
  
  BYTE dummy;
  if (!readByte(dummy, TIMEOUT_READ, true))
  {
    return;
  }
  m_CRC = 0;
  
  // check if this filename ends with .P32
  if (!strcasestr(m_ioBuffer, ".p32"))
  {
    strcat(m_ioBuffer, ".p32");
  }
  
  if (strlen(m_ioBuffer) > sizeof(m_cwdPath)-1)
  {
    sendByte(PMD32_PATH_TOO_LONG, TIMEOUT_SEND_RESULT);
    return;
  }
  
  // now try to create
  File file = sd.open(m_ioBuffer, O_RDWR | O_CREAT | O_TRUNC);
  if (!file)
  {
    sendByte(PMD32_CREATE_ERROR, TIMEOUT_SEND_RESULT);
    return;
  }
  
  // we don't need the path in m_ioBuffer any longer
  const DWORD imageSize = 368640L;
  const WORD bufSize = sizeof(m_ioBuffer);
  memset(m_ioBuffer, 0xE5, bufSize);
  
  WORD count = imageSize / bufSize;
  while(count)
  {
    if (!file.write(m_ioBuffer, bufSize))
    {
      file.close();
      sendByte(PMD32_CREATE_ERROR, TIMEOUT_SEND_RESULT);
      return;
    }      
    count--;
  }
  
  file.close();
  sendByte(PMD32_OK, TIMEOUT_SEND_RESULT);
}

void PMD32::extraImageInfo()
{  
  BYTE drive;
  if (!readByte(drive))
  {
    return;
  }
  BYTE data;
  if (!readByte(data, TIMEOUT_READ, true))
  {
    return;
  }  
  m_CRC = 0;
  
  if (drive > 3)
  {
    sendByte(PMD32_INVALID_DRIVE, TIMEOUT_SEND_RESULT);
    return;
  }
  
  if (!sendByte(PMD32_OK, TIMEOUT_SEND_RESULT))
  {
    return;
  }
  
  // 360K P32 in a PMD 85; 36x128B logical sectors (9x512B physical), 2 sides, 40 tracks per side
  const bool isDriveMounted = fsIsDriveMounted(drive);
  const BYTE tracks = isDriveMounted ? 80 : 0; // meant on both sides
  const BYTE sect128BPerTrack = isDriveMounted ? 36 : 0;
  const BYTE physSectorSize = isDriveMounted ? 2 : 0; // 2: 512B
  
  data = tracks;
  m_CRC ^= data;
  if (!sendByte(data))
  {
    return;
  }
  
  data = sect128BPerTrack;
  m_CRC ^= data;
  if (!sendByte(data))
  {
    return;
  }
  
  data = physSectorSize;
  m_CRC ^= data;
  if (!sendByte(data))
  {
    return;
  }
  
  sendByte(m_CRC);
}

#endif // TOUCH_SCREEN_CALIBRATION