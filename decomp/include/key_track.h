// A key-frame track (0x14 bytes): a growable list of (frame, Vec4 value, flag) keys kept sorted by frame, so the last
// key's frame is the track's length. Scene nodes animate through five of them (NodeExtra in node_data.h), camera
// paths keep one per path, and AnimChannels (anim_channels.cpp) holds twelve. Most methods are in containers.cpp;
// vec4.cpp defines Reserve. Flag 4 (KEY_FLAG_MOTION) marks a key as a motion frame key.
// This is the one declaration of KeyTrack: include this header, never redeclare it.
#ifndef GURUMIN_KEY_TRACK_H
#define GURUMIN_KEY_TRACK_H

#include "vec4.h"

// Per-key flag: the key is a motion frame key (set on every key by MarkAll).
#define KEY_FLAG_MOTION 4
// Per-key flag of a KeyBank: hold the previous key's value up to this key (KeyBank::Sample).
#define KEY_FLAG_HOLD 0x10

struct KeyTrack
{
    int capacity;     // 0x00: allocated length of the three arrays
    int *frames;      // 0x04: frame number of each key, ascending
    Vec4 *values;     // 0x08: value of each key (a position, a quaternion or a scale)
    char *flags;      // 0x0c: KEY_FLAG_* bits of each key
    int count;        // 0x10: keys in use

    KeyTrack();                                                    // empty track (anim_channels.cpp's members)
    void Free();                                                   // 0x101ad9f0: frees the arrays, empties the track
    void Reserve(int extra);                                       // 0x101ada30 (vec4.cpp): grows the arrays by `extra`
    void Init();                                                   // 0x101adb40: empties a track allocated raw
    int MarkAll();                                                 // 0x101adb80: sets KEY_FLAG_MOTION on every key
    int Insert(int frame, float x, float y, float z, float w);     // 0x101adbb0: adds or replaces the key at `frame`
    int LastFrame();                                               // 0x101adc90: frame of the last key
    int HasFrame(int frame);                                       // 0x101adcb0: 1 when a key sits at `frame`
    int Set(int frame, float x, float y, float z, float w);        // 0x101adcf0
    int AddKey(int frame, float x, float y, float z, float w);     // appends a key (used by CopyFrom)
    int CopyFrom(const KeyTrack *source);                          // 0x101ade10: makes this track a copy of `source`
    // 0x101afe00: the value at `frame` (with `rotation` set, a quaternion interpolated as a rotation).
    int Sample(int frame, float *x, float *y, float *z, float *w, int rotation);
};

// Capacity of a KeyBank.
#define KEY_BANK_SIZE 0x80

// A fixed key-frame bank (0xa84 bytes): up to 0x80 keys kept sorted by frame, used while loading and inside motion
// channels (anim_model.h), effect models and the world's tables. Constructor and Insert/Set in vec4.cpp, the rest in
// containers.cpp; Sample at 0x101af4c0.
struct KeyBank
{
    int frames[KEY_BANK_SIZE];     // 0x000: frame number of each key, ascending
    Vec4 values[KEY_BANK_SIZE];    // 0x200: value of each key
    char flags[KEY_BANK_SIZE];     // 0xa00: KEY_FLAG_* bits of each key
    int count;                     // 0xa80: keys in use

    KeyBank();                                                     // 0x101af420: one zero key at frame 0
    void Clear();                                                  // 0x101ad730: back to one zero key at frame 0
    int Insert(int frame, float x, float y, float z, float w);     // 0x101ad760
    int Set(int frame, float x, float y, float z, float w);        // 0x101ad820
    int CopyTo(KeyTrack *target);                                  // 0x101af450: appends every key to `target`
    // 0x101af4c0: the value at `frame` (flags as KeyTrack::Sample's `rotation`).
    int Sample(int frame, float *x, float *y, float *z, float *w, int flags);
};

#endif
