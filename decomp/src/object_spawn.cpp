// Object spawning: placed scene objects share the character table (DAT_10db6ed8) with the player, NPCs and
// enemies. DAT_1021f338 is the next free slot; objects may use slots up to 0x7a. Slot kind 6 in DAT_1021f3c0
// marks an object.

#include <string.h>
#include "world.h"
#include "vec4.h"

// Root node of a character model (scene_node.cpp Node).
struct RootNode
{
    void M_10191280();
    void M_10194630(int value, int mask);
    void M_101946b0(int value, int mask);
    void M_10194730(int value, int mask);
};

extern "C" int DAT_1021f338;
extern "C" char DAT_1021f3c0[0x80];
extern "C" const char *FUN_1013f8a0(const char *name, int folder);

// Message window state (see Message_Show, app_0013.cpp).
extern "C" int DAT_104bfd4c;
extern "C" char DAT_104c01fc;
extern "C" int DAT_104bf8c8;
extern "C" int DAT_104bf8cc;
extern "C" int DAT_104bf8d0;
extern "C" char DAT_10467ea0[];
extern "C" char DAT_10219bc0[];
extern "C" const char DAT_101d76d0[];  // "オブジェ数オーバーだぞよ"

// Character fields used only by objects.
struct ObjectView
{
    char pad_0000[0x22c];
    Vec4 colour;              // 0x22c: model colour (r, g, b, a)
    char pad_023c[0x3a];
    char m_0276;              // 0x276: set to 1 for objects
};

// Loads model file `name` into the next free object slot and places it at (x, y, z). The model's colour values
// are set to (slot, 0xff, 0x4f) on every node and its colour to opaque white. Returns the slot, or 0 after
// showing "オブジェ数オーバーだぞよ" ("too many objects") when the table is full.
extern "C" int FUN_10017c00(const char *name, int x, int y, int z)
{
    if (DAT_1021f338 >= 0x7b)
    {
        DAT_104bfd4c = 0x50;
        DAT_104c01fc = 0;
        DAT_104bf8c8 = 1;
        DAT_104bf8cc = 0;
        DAT_104bf8d0 = 0;
        DAT_10467ea0[0] = 0;
        strcpy(DAT_10219bc0, DAT_101d76d0);
        return 0;
    }
    int slot = DAT_1021f338++;
    DAT_10db6ed8[slot].model.M_1017b9b0(FUN_1013f8a0(name, 0));
    DAT_10db6ed8[slot].m_22fc->M_10191280();
    DAT_10db6ed8[slot].model.M_10162420((float)x, (float)y, (float)z);
    DAT_1021f3c0[slot] = 6;
    ((ObjectView *)&DAT_10db6ed8[slot])->m_0276 = 1;
    DAT_10db6ed8[slot].m_22fc->M_10194630(slot, -1);
    DAT_10db6ed8[slot].m_22fc->M_101946b0(0xff, -1);
    DAT_10db6ed8[slot].m_22fc->M_10194730(0x4f, -1);
    ((ObjectView *)&DAT_10db6ed8[slot])->colour.Set(1.0f, 1.0f, 1.0f, 1.0f);
    return slot;
}
