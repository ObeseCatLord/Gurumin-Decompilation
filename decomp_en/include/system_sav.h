// The system save record (EN build). Declared once here.
#ifndef GURUMIN_EN_SYSTEM_SAV_H
#define GURUMIN_EN_SYSTEM_SAV_H

// Bytes of the system save file as kept in the cloud.
const int SYSTEM_SAV_SIZE = 0x4000;

// Sets record `index` (a dword index) of the system save `path` to `value` in the cloud copy of the file: the copy
// is read when it exists and, when `write` is nonzero, written back. Returns 0. EN 0x7af2a0 (JP SystemSav_WriteRecord).
extern "C" int SystemSav_WriteRecord(const char *path, int index, unsigned int value, char write);

#endif
