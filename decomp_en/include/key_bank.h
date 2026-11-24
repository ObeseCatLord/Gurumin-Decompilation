// Key bank of the EN build: the keyframes of one animation channel. Declared once here (JP: decomp/include/key_track.h).
#ifndef GURUMIN_EN_KEY_BANK_H
#define GURUMIN_EN_KEY_BANK_H

#include "vec4.h"

const int KEY_BANK_SIZE = 0x80;     // keys a bank holds
const char KEY_FLAG_MOTION = 4;     // key flag: the key is a motion frame

struct KeyBank
{
    int frames[KEY_BANK_SIZE];      // 0x000: frame number of each key, ascending
    Vec4 values[KEY_BANK_SIZE];     // 0x200: value of each key
    char flags[KEY_BANK_SIZE];      // 0xa00: KEY_FLAG_* bits of each key
    int count;                      // 0xa80: keys in use

    void Clear();                                                // EN 0x7785e0 (JP 0x101ad730): back to one zero key at frame 0
    int Set(int frame, float x, float y, float z, float w);      // EN 0x779690 (JP 0x101ad820): returns the key's index
};

#endif
