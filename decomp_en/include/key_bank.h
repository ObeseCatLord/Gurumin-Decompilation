// Key bank of the EN build: the keyframes of one animation channel. Declared once here (JP: decomp/include/key_track.h).
#ifndef GURUMIN_EN_KEY_BANK_H
#define GURUMIN_EN_KEY_BANK_H

#include "vec4.h"

struct KeyTrack;

const int KEY_BANK_SIZE = 0x80;     // keys a bank holds
const char KEY_FLAG_MOTION = 4;     // key flag: the key is a motion frame

struct KeyBank
{
    int frames[KEY_BANK_SIZE];      // 0x000: frame number of each key, ascending
    Vec4 values[KEY_BANK_SIZE];     // 0x200: value of each key
    char flags[KEY_BANK_SIZE];      // 0xa00: KEY_FLAG_* bits of each key
    int count;                      // 0xa80: keys in use

    int CopyTo(KeyTrack *target);                                // EN 0x771d00 (JP 0x101af450): copies the bank's keys into a growable track
    KeyBank();                                                   // EN 0x76ef50 (JP 0x101af420): an empty bank
    void Clear();                                                // EN 0x7785e0 (JP 0x101ad730): back to one zero key at frame 0
    int Set(int frame, float x, float y, float z, float w);      // EN 0x779690 (JP 0x101ad820): returns the key's index
    int Sample(int frame, float *x, float *y, float *z, float *w, int flags);   // EN 0x7703f0 (JP 0x101af4c0): the value at `frame`
    int Insert(int frame, float x, float y, float z, float w);   // EN 0x76f900 (JP 0x101ad760): inserts a key, returns its index
};

#endif
