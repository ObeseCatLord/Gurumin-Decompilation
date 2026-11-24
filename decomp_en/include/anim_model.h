// An animated model of the EN build (JP: decomp/include/anim_model.h, 0x249c bytes there). Only the members EN code
// has used so far are named; offsets are proven by matched EN functions. This is the one declaration of AnimModel and
// MotionChannels in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_ANIM_MODEL_H
#define GURUMIN_EN_ANIM_MODEL_H

#include "key_bank.h"
#include "vec4.h"

struct Node;

// Layers of a Playback (layer numbers above PLAYBACK_LAYER_COUNT - 1 are rejected).
const int PLAYBACK_LAYER_COUNT = 0x90;
// Motion slots per layer.
const int PLAYBACK_SLOT_COUNT = 16;
// Bytes of a Playback block.
const int PLAYBACK_SIZE = 0x7c84;
// Playback::endState value when the playback has reached its end.
const int PLAYBACK_END_REACHED = 1;
// Bytes of an AnimModel.
const int ANIM_MODEL_SIZE = 0x249c;

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

struct AnimModel;

// The motion playback state of a model (0x7c84 bytes): layers of frame ranges and per-slot offsets. Allocated on
// first use through AnimModel::GetPlayback. Only the members EN code has used are named.
struct Playback
{
    char m_0000[0xbec];                // 0x0000: not yet named
    int endState;                      // 0x0bec: PLAYBACK_END_REACHED once the playback has reached its end
    char m_0bf0[0x1070 - 0xbf0];       // 0x0bf0: not yet named
    int slotOffsetX[PLAYBACK_LAYER_COUNT][PLAYBACK_SLOT_COUNT];  // 0x1070: x offset per layer and slot
    int slotOffsetY[PLAYBACK_LAYER_COUNT][PLAYBACK_SLOT_COUNT];  // 0x3470: y offset per layer and slot
    char m_5870[PLAYBACK_SIZE - 0x5870];  // 0x5870: not yet named

    void Init();                                                       // EN 0x7412b0 (JP 0x10169f90)
    int BindModel(AnimModel *owner);                                   // EN 0x71c060 (JP 0x1016a020)
    int SetLayer(int layer, int a, int first, int last, int b, int c); // EN 0x7437e0 (JP 0x1016a100)
    int SetSlotOffset(int layer, int slot, int x, int y);              // EN 0x746490 (JP 0x1016a260)
};

struct AnimModel
{
    char m_0000[0x265];           // 0x000: not yet named
    char important;               // 0x265: set when a character is assigned to a slot (CharacterBank::Assign)
    char m_0266[0x298 - 0x266];   // 0x266: not yet named
    Playback *playback;           // 0x298: the motion playback state (allocated on first use)
    float speedScale;             // 0x29c: motion speed scale
    char m_02a0[0x2ac - 0x2a0];   // 0x2a0: not yet named
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
    char m_229c[0x22ec - 0x229c]; // 0x229c: not yet named
    Node *root;                   // 0x22ec: the model's node tree (0 = free entry of the model bank)
    char m_22f0[0x2300 - 0x22f0]; // 0x22f0: not yet named
    Node *swapNode1;              // 0x2300: first node of the material swap
    Node *swapNode2;              // 0x2304: second node of the material swap
    char m_2308[0x231c - 0x2308]; // 0x2308: not yet named
    int value231c;                // 0x231c: set by SetValue231c
    char m_2320[0x23d8 - 0x2320]; // 0x2320: not yet named
    Vec4 moveStep;                // 0x23d8: this frame's movement: x, y scaled by speedScale, z the vertical step
    char m_23e8[ANIM_MODEL_SIZE - 0x23e8];  // 0x23e8: not yet named

    int SetTrackCOptions(int playing, int looping);                      // EN 0x743960 (JP 0x10161b40)
    int SetMotionOptions(int playing, int looping);                      // EN 0x745c00 (JP 0x10161be0)
    int SetTrackBOptions(int playing, int looping);                      // EN 0x7469b0 (JP 0x10161f60)
    int AddTrackCKey(int frame, float x, float y, float z);              // EN 0x70ad20 (JP 0x10161af0)
    int AddMotionKey(int frame, float value);                            // EN 0x70af70 (JP 0x10161b90)
    int SetPendingPush(float x, float y, float z, char flag);            // EN 0x73f940 (JP 0x101628b0)
    int ClearTrackC();                                                   // EN 0x741bd0 (JP 0x10161a60)
    int ClearMotionKeys();                                               // EN 0x741c10 (JP 0x10161b60)
    int ClearTrackB();                                                   // EN 0x741fe0 (JP 0x10161f30)
    int ApplyMaterialSwap(int count);                                    // EN 0x71ac90 (JP 0x10161a20)
    int SetValue231c(int value);                                         // EN 0x746bc0 (JP 0x10163e70)
    int SetMoveStep(float x, float y, float z);                          // EN 0x73f990 (JP model_params.cpp)
    int IsPlaybackAtEnd();                                               // EN 0x7356b0 (JP 0x1017b160)
    int SetLayer(int layer, int a, int first, int last, int b, int c);   // EN 0x7438f0: Playback::SetLayer on the playback
};

#endif
