// Unlock progress state of the EN build (written by the save loader, EN 0x5ff130). This is the one declaration of these
// globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_PROGRESS_UNLOCK_H
#define GURUMIN_EN_PROGRESS_UNLOCK_H

// g_unlockBits value with every unlock bit set.
const unsigned int UNLOCK_BITS_ALL = 0x1f1f1f1f;
// Divisors that split g_unlockCode into its decimal parts.
const int UNLOCK_CODE_HIGH_DIVISOR = 1000000;
const int UNLOCK_CODE_MID_DIVISOR = 10000;
const int UNLOCK_CODE_MID_RANGE = 100;

extern "C" unsigned int g_unlockBits;   // EN 0x01334884: unlock flag bits set by the save loader (UNLOCK_BITS_ALL = everything unlocked)
extern "C" int g_unlockCode;   // EN 0x01334008: decimal-coded value of the save; its two upper digit groups tell whether it is set

// 1 when every unlock bit is set and g_unlockCode has a non-zero high or middle digit group, else 0. Called by the
// scene setup (EN 0x5fd1b0). EN 0x5c14b0 (see progress_unlock.cpp).
extern "C" bool Unlock_IsBonusAvailable(void);

#endif
