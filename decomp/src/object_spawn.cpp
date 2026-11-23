// Object spawning: placed scene objects share the character table (DAT_10db6ed8) with the player, NPCs and
// enemies. DAT_1021f338 is the next free slot; objects may use slots up to 0x7a. Slot kind 6 in DAT_1021f3c0
// marks an object.

#include <string.h>
#include "world.h"
#include "vec4.h"
#include "anim_model.h"
#include "char_state.h"

#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"

extern "C" const char *FUN_1013f8a0(const char *name, int folder);

// Message window state (see Message_Show, app_0013.cpp).

#include "object_view.h"

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
    DAT_10db6ee8[slot].LoadFile(FUN_1013f8a0(name, 0));
    DAT_10db6ee8[slot].root->ReleaseUniqueMeshData();
    DAT_10db6ee8[slot].Teleport((float)x, (float)y, (float)z);
    DAT_1021f3c0[slot] = 6;
    ((ObjectView *)&DAT_10db6ee8[slot])->m_0276 = 1;
    DAT_10db6ee8[slot].root->M_10194630(slot, -1);
    DAT_10db6ee8[slot].root->M_101946b0(0xff, -1);
    DAT_10db6ee8[slot].root->M_10194730(0x4f, -1);
    ((ObjectView *)&DAT_10db6ee8[slot])->colour.Set(1.0f, 1.0f, 1.0f, 1.0f);
    return slot;
}
