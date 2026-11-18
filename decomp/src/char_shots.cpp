// Shots fired by characters. Each character slot `owner` has six helper slots owner+2 .. owner+7 that its shots
// use; a helper slot is free while its state (CharState::m_0000) is 0.
#include "world.h"

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    void Copy(const Vec4 &source);
};

// Per-slot position, 0x40 bytes per slot.
struct SlotPosition
{
    Vec4 position;
    char pad_0010[0x30];
};

// Per-slot flight direction and speed.
struct SlotMotion
{
    float pitch;              // copied from the owner
    float heading;            // radians
    float speed;
    int pad_000c;
};

extern "C" SlotPosition DAT_10802bf8[0x80];
extern "C" SlotMotion DAT_10255160[0x80];
extern "C" int FUN_1013f880(void);  // the game's random number

// 0x1001a680 (Char_FireShot). Starts a shot of character `owner` from `position` in the first free helper slot:
// state 1, 300 frames to live, `spread` stored in the slot, the owner's pitch and target heading, speed 5. With
// `spread` set the heading is turned by a random amount of up to +-0.025 radians. Returns 0, also when all six
// helper slots are in use.
extern "C" int FUN_1001a680(int owner, const Vec4 *position, int spread)
{
    for (int slot = owner + 2; slot < owner + 8; slot++)
    {
        if (DAT_1020b430[slot].m_0000 != 0)
            continue;
        DAT_1020b430[slot].m_0000 = 1;
        DAT_10802bf8[slot].position.Copy(*position);
        DAT_1020b430[slot].m_0004[0] = 300;
        DAT_1020b430[slot].m_0004[1] = spread;
        DAT_1020b430[slot].m_0004[2] = 0;
        DAT_10255160[slot].pitch = DAT_10255160[owner].pitch;
        DAT_10255160[slot].heading = DAT_10db6ed8[owner].targetHeading;
        if (spread != 0)
            DAT_10255160[slot].heading = (float)(FUN_1013f880() % 250 - 125) / 5000.0f + DAT_10255160[slot].heading;
        DAT_10255160[slot].speed = 5.0f;
        return 0;
    }
    return 0;
}
