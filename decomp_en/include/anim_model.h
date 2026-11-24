// An animated model of the EN build (JP: decomp/include/anim_model.h, 0x249c bytes there). Only the members EN code
// has used so far are named; offsets are proven by matched EN functions. This is the one declaration of AnimModel and
// MotionChannels in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_ANIM_MODEL_H
#define GURUMIN_EN_ANIM_MODEL_H

#include "key_bank.h"
#include "vec4.h"

struct Node;

// First frame of an empty channel: later than any real key, so the first key added becomes the start.
const int CHANNEL_EMPTY_FIRST_FRAME = 10000;

// One key-framed motion channel of a model: a key bank and its playback range.
struct MotionChannels
{
    KeyBank keys;     // 0x000: the keys
    int last;         // 0xa84: last frame (exclusive for track C)
    int first;        // 0xa88: first frame (CHANNEL_EMPTY_FIRST_FRAME when empty)
    int frame;        // 0xa8c: current frame while playing
    int playing;      // 0xa90: 1 while playing
    int looping;      // 0xa94: 1 = loop back to `first` at the end
};

struct AnimModel
{
    char m_0000[0x2ac];           // 0x000: not yet named
    MotionChannels motion;        // 0x2ac: the motion channel (track A)
    char m_0d44[0xc];             // 0xd44: not yet named
    MotionChannels trackB;        // 0xd50: track B
    char m_17e8;                  // 0x17e8: not yet named
    char locked;                  // 0x17e9: non-zero while the model ignores queued pushes
    char pad_17ea[2];
    MotionChannels trackC;        // 0x17ec: track C
    int m_2284;                   // 0x2284: not yet named
    char pushFlag;                // 0x2288: flag of the queued push
    char pad_2289[3];
    Vec4 pushVector;              // 0x228c: queued push (x, y, z)
    char m_229c[0x2300 - 0x229c]; // 0x229c: not yet named
    Node *swapNode1;              // 0x2300: first node of the material swap
    Node *swapNode2;              // 0x2304: second node of the material swap

    int SetTrackCOptions(int playing, int looping);                      // EN 0x743960 (JP 0x10161b40)
    int SetMotionOptions(int playing, int looping);                      // EN 0x745c00 (JP 0x10161be0)
    int SetTrackBOptions(int playing, int looping);                      // EN 0x7469b0 (JP 0x10161f60)
    int AddTrackCKey(int frame, float x, float y, float z);              // EN 0x70ad20 (JP 0x10161af0)
    int AddMotionKey(int frame, float value);                            // EN 0x70af70 (JP 0x10161b90)
    int SetPendingPush(float x, float y, float z, char flag);            // EN 0x73f940 (JP 0x101628b0)
    int ClearTrackC();                                                   // EN 0x741bd0 (JP 0x10161a60)
    int ClearMotionKeys();                                               // EN 0x741c10 (JP 0x10161b60)
    int ClearTrackB();                                                   // EN 0x741fe0 (JP 0x10161f30)
};

#endif
