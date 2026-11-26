// Key bank of the EN build: the keyframes of one animation channel. Declared once here (JP: decomp/include/key_track.h).
#ifndef GURUMIN_EN_KEY_BANK_H
#define GURUMIN_EN_KEY_BANK_H

#include "vec4.h"

struct KeyTrack;

// Data-less base of the key containers in the JP build (KeyTrack and KeyBank derive from it there); the EN callers load
// ecx with their own container before calling Spline, which never reads `this`.
struct KeyInterpolator
{
    // EN 0x771b60 (JP 0x101ad920): cubic Hermite interpolation between `b` (t = 0) and `c` (t = 1) with neighbours `a` and `d`.
    float Spline(float t, float a, float b, float c, float d);
};

const int KEY_BANK_SIZE = 0x80;     // keys a bank holds
const char KEY_FLAG_MOTION = 4;     // key flag: the key is a motion frame
const int KEY_SAMPLE_POSITION = 0;  // KeyBank::Sample flags: plain component-wise interpolation
const int KEY_SAMPLE_ROTATION = 1;  // KeyBank::Sample flags: the values are quaternions

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
    int RemoveFrame(int frame);                                  // EN 0x772920 (key_remove.cpp, EN-only): removes the key at `frame` and moves the later keys up; 1 when found, else 0
    int RemoveAt(int index);                                    // EN 0x772a40 (key_remove.cpp, EN-only): takes key `index` out and moves the later keys up; returns 0
    int FindFrame(int frame);                                   // EN 0x773bd0 (key_find.cpp, EN-only): index of the key at `frame`, -1 when there is none
    int MoveFrame(int frame, int newFrame, float x, float y, float z, float w);   // EN 0x776dd0 (key_move.cpp, EN-only): re-keys the key at `frame` to `newFrame` with new values (KEY_UNCHANGED keeps the key's own); 1 when found, else 0
    int MoveKeyTo(int index, int frame, float x, float y, float z, float w);   // EN 0x777120 (key_move.cpp, EN-only): key `index` gets frame `frame` (kept between its neighbours) and the values; keeps its flags; returns 1
    int ShiftKey(int index, int offset, float x, float y, float z, float w);   // EN 0x776f10 (key_move.cpp, EN-only): like MoveKeyTo with the frame `offset` after the key's own; returns 1
};

#endif
