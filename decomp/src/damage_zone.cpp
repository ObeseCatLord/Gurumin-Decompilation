// Damage zones: a zone character (its m_0280 picks the zone) hurts the enemies (slots 0x3a-0x5b) standing in its
// box, checking every step-th enemy per frame. Enemies that are away (x < -20000) are tested at their saved position.

#include <stdlib.h>

// A zone (0x100 bytes per entry): an integer box and how it hurts.
struct DamageZone
{
    int left;                     // 0x00: x range
    int right;                    // 0x04
    int pad_0008[2];
    int front;                    // 0x10: y range
    int back;                     // 0x14
    int pad_0018[2];
    int top;                      // 0x20: z range (top first)
    int bottom;                   // 0x24
    int pad_0028[2];
    int damage;                   // 0x30
    int step;                     // 0x34: enemies checked per frame are every step-th one
    int onFlag;                   // 0x38: script flag that must be set (0 = always on)
    int offFlag;                  // 0x3c: script flag that turns the zone off (0 = none)
    char pad_0040[0xc0];
};

#include "vec3.h"

// A character record (world.h Character), only the fields used here.
struct Character
{
    char pad_0000[0x280];
    int zone;                     // 0x280: the zone a zone character runs
    char pad_0284[0x20e4];
    Vec3 position;                // 0x2368: the matrix translation
    char pad_2374[0x128];
};

// The saved position of each character slot (16 bytes per slot).
struct SavedPosition
{
    float x;
    float y;
    float z;
    float w;
};

// Per-slot state table at 0x10daaaec (actor_scan.cpp ActorSlot).
struct ActorSlot
{
    int health;
    char pad_0004[0x184];
};

struct Game
{
    int TestScriptFlag(int index);   // 0x100022b0: the out-of-line copy (game.h)
};

// Per-character AI state (world.h CharState): the zone's turn counter.
struct ZoneState
{
    int turn;
    char pad_0004[0x3c];
};

extern "C" Character DAT_10db6ed8[0x80];
extern "C" DamageZone DAT_10210fc0[];
extern "C" ZoneState DAT_1020b430[0x80];
extern "C" ActorSlot DAT_10daaaec[0x80];
extern "C" SavedPosition DAT_10867e18[0x80];
extern "C" int DAT_10210d30[0x80];               // non-zero = the character is being defeated
extern "C" char DAT_1020d540[0x80];              // per zone character: also hurt enemies that are away
extern "C" Game DAT_10874848;
extern "C" int FUN_100119d0(int index, int damage, int a);   // shows the damage on character `index`

// 0x1002ebf0 (DamageZone_Update). One frame of zone character `index`: unless its zone's on flag is clear or its
// off flag set, the turn counter advances (wrapping after `step`) and the enemies from slot 0x3a + turn on, every
// step-th one, that stand inside the box (or are away with their saved position inside it, when the zone
// character allows that) lose `damage` health. Returns 0.
extern "C" int FUN_1002ebf0(int index)
{
    int zone = DAT_10db6ed8[index].zone;
    if (DAT_10210fc0[zone].onFlag != 0)
    {
        int offFlag = DAT_10210fc0[zone].offFlag;
        int on = DAT_10874848.TestScriptFlag(DAT_10210fc0[zone].onFlag);
        if (offFlag == 0)
        {
            if (on == 0)
                return 0;
        }
        else
        {
            if (on == 0)
                return 0;
            if (DAT_10874848.TestScriptFlag(offFlag) != 0)
                return 0;
        }
    }
    int step = DAT_10210fc0[zone].step;
    if (++DAT_1020b430[index].turn > step)
        DAT_1020b430[index].turn = 0;
    for (int slot = DAT_1020b430[index].turn + 0x3a; slot < 0x5c; slot += __max(1, step))
    {
        if ((DAT_10db6ed8[slot].position.x > DAT_10210fc0[zone].left &&
             DAT_10db6ed8[slot].position.x < DAT_10210fc0[zone].right &&
             DAT_10db6ed8[slot].position.y > DAT_10210fc0[zone].front &&
             DAT_10db6ed8[slot].position.y < DAT_10210fc0[zone].back &&
             DAT_10db6ed8[slot].position.z > DAT_10210fc0[zone].bottom &&
             DAT_10db6ed8[slot].position.z < DAT_10210fc0[zone].top) ||
            (DAT_10210d30[slot] == 0 && DAT_1020d540[index] != 0 && DAT_10db6ed8[slot].position.x < -20000.0f &&
             DAT_10867e18[slot].x > DAT_10210fc0[zone].left && DAT_10867e18[slot].x < DAT_10210fc0[zone].right &&
             DAT_10867e18[slot].y > DAT_10210fc0[zone].front && DAT_10867e18[slot].y < DAT_10210fc0[zone].back &&
             DAT_10867e18[slot].z > DAT_10210fc0[zone].bottom && DAT_10867e18[slot].z < DAT_10210fc0[zone].top))
        {
            int damage = DAT_10210fc0[zone].damage;
            FUN_100119d0(slot, damage, 0);
            DAT_10daaaec[slot].health -= damage;
        }
    }
    return 0;
}
