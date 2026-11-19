// Switches and levers held by characters or objects. Each of the 128 hold slots records who holds it
// (DAT_10255b08): a character slot (< 0x80) or 0x80 + an object index; DAT_1030bbf8 marks the slot as used.

#include "world.h"

// Map object (0x10874848) that animates the switch nodes ("LEVERON" and its off state).
struct MapObject
{
    int M_10054f30(int node, int on);
    int M_10055110(int id, int on);   // switches the map gimmick with `id` on or off
};

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
};




extern "C" MapObject DAT_10874848;
extern "C" int DAT_1030bbf8[0x80];       // the held object of each slot (non-zero while used, switch_effects.cpp)
extern "C" int DAT_10255b08[0x80];       // holder of each slot
extern "C" short DAT_10254838[0x80];     // per-object switch state; 1000 is added while it is held

// Releases the hold slot of `holder`: a character's hold count drops by one and its switch node is turned off; an
// object loses its "held" bonus of 1000. Returns 1, or 0 when `holder` holds nothing.
extern "C" int FUN_10057b90(int holder)
{
    for (int i = 0; i < 0x80; i++)
    {
        if (DAT_1030bbf8[i] != 0 && DAT_10255b08[i] == holder)
        {
            int who = DAT_10255b08[i];
            if (who < 0x80)
            {
                DAT_10daab84[who].m_00a4--;
                DAT_10874848.M_10054f30(DAT_10db6ed8[who].m_0280, 0);
            }
            else if (DAT_10254838[who % 0x80] >= 1000)
                DAT_10254838[who % 0x80] -= 1000;
            DAT_1030bbf8[i] = 0;
            return 1;
        }
    }
    return 0;
}

extern "C" char DAT_1021f3c0[0x80];       // slot kind (0x13 / 0x14 = the two switch kinds)
extern "C" short DAT_1032d4b8[0x80];      // script flag set by the switch (0 = none)
extern "C" short DAT_1020b128[0x80];      // switch animation frame
extern "C" short DAT_10219ec0[0x80];      // gimmick id controlled by the switch
extern "C" int DAT_10c62edc[0x1000];      // script flags
extern "C" int DAT_104bfe6c;              // 1 while sounds are muted (cut-scenes)
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);


// Turns the switch in slot `slot` on: kind 0x13 shows frame 1 and plays sound 0x38a, kind 0x14 shows frame 0x1f and
// plays 0x389. Both set their script flag and switch their gimmick (on for 0x13, off for 0x14). Returns 0.
extern "C" int FUN_1005f530(int slot)
{
    if (DAT_1021f3c0[slot] == 0x13)
    {
        int flag = DAT_1032d4b8[slot];
        DAT_1020b128[slot] = 1;
        if (flag <= 3999 && flag != 0)
            DAT_10c62edc[flag] = 1;
        DAT_10874848.M_10055110(DAT_10219ec0[slot], 1);
        if (DAT_104bfe6c == 0)
            FUN_1003ad20(0x38a, &DAT_10db6ed8[slot].matrix.GetTranslation(), 1.0f);
    }
    if (DAT_1021f3c0[slot] == 0x14)
    {
        int flag = DAT_1032d4b8[slot];
        DAT_1020b128[slot] = 0x1f;
        if (flag <= 3999 && flag != 0)
            DAT_10c62edc[flag] = 1;
        DAT_10874848.M_10055110(DAT_10219ec0[slot], 0);
        if (DAT_104bfe6c == 0)
            FUN_1003ad20(0x389, &DAT_10db6ed8[slot].matrix.GetTranslation(), 1.0f);
    }
    return 0;
}

// Turns the switch back: kind 0x13 shows frame 0x3d (sound 0x38c), kind 0x14 frame 0x5b (sound 0x38b); the script
// flag is cleared. Returns 0.
extern "C" int FUN_1005f640(int slot)
{
    if (DAT_1021f3c0[slot] == 0x13)
    {
        int flag = DAT_1032d4b8[slot];
        DAT_1020b128[slot] = 0x3d;
        if (flag <= 3999 && flag != 0)
            DAT_10c62edc[flag] = 0;
        DAT_10874848.M_10055110(DAT_10219ec0[slot], 1);
        if (DAT_104bfe6c == 0)
            FUN_1003ad20(0x38c, &DAT_10db6ed8[slot].matrix.GetTranslation(), 1.0f);
    }
    if (DAT_1021f3c0[slot] == 0x14)
    {
        int flag = DAT_1032d4b8[slot];
        DAT_1020b128[slot] = 0x5b;
        if (flag <= 3999 && flag != 0)
            DAT_10c62edc[flag] = 0;
        DAT_10874848.M_10055110(DAT_10219ec0[slot], 0);
        if (DAT_104bfe6c == 0)
            FUN_1003ad20(0x38b, &DAT_10db6ed8[slot].matrix.GetTranslation(), 1.0f);
    }
    return 0;
}
