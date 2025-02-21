// PMD32-Mega2560 (c) 2025 J. Bogin, https://boginjr.com
// Based on PMD32-SD (c) 2012 R. Borik, https://pmd85.borik.net/
// Image filesystem operations

#pragma once

#define MAX_PATH 255

bool fsIsDriveMounted(BYTE drive);
bool fsIsFileNameInUse(const char* fileName);
bool fsMount(BYTE drive, BYTE& progmemResult, bool readOnly = false);
bool fsCreateAndMount(BYTE drive, BYTE& progmemResult);
void fsUnmount(BYTE drive);
void fsUnmountAll();
char* fsGetImagePath(BYTE drive);
File* fsGetFile(BYTE drive);
void fsStoreDriveToEEPROM(BYTE drive);
void fsAutoLoadImagesFromEEPROM();