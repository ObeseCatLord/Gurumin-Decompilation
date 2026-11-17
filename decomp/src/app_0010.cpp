// Sound slots, menus, character helpers and option flags.

#include <stdlib.h>
#include "world.h"
#include "game.h"

struct NamedPath
{
    char path[0x4b34];
};

extern "C" Game DAT_104b7200;
extern "C" int DAT_104c01b0;
extern "C" short DAT_10447338[0x44c];
extern "C" int FUN_10001b40(void);
extern "C" char DAT_1020e0f8[];
extern "C" NamedPath DAT_10b314c8[];
extern "C" const char *FUN_1013f8a0(const char *format, ...);
extern "C" int DAT_104c0198;
extern "C" int DAT_104c00d0;
extern "C" int DAT_1049f0f0;
extern "C" Vec3 *DAT_1032d5b8[];
extern "C" char DAT_1021bbc0[];
extern "C" int DAT_10c66590;
extern "C" int DAT_10c66594;
extern "C" char DAT_104c0131;
extern "C" char DAT_104c0130;
extern "C" short DAT_10219ec0[0x80];
extern "C" char DAT_1021f3c0[0x80];

// Marks all 0x44c looping-sound slots free (-11) and stops the sound process' loops (command 0x221, sent
// twice when the first call succeeds) unless sound is muted.
extern "C" void FUN_100102e0(void)
{
    if (DAT_104c01b0 == 0)
    {
        for (int i = 0; i < 0x44c; i++)
            DAT_10447338[i] = -11;
        if (FUN_10001b40())
            FUN_10001b40();
    }
}

// Closes the yes/no dialog (menu 0x216) and menu 0x218 and clears the message text. Returns 1.
extern "C" int FUN_100106a0(void)
{
    DAT_104b7200.M_100083e0(0x218);
    DAT_104b7200.M_100083e0(0x216);
    DAT_1020e0f8[0] = 0;
    return 1;
}

// Marks character `index` as defeated (m_00a5) unless it is busy; returns 1 when busy.
extern "C" int FUN_10011f80(int index)
{
    if (DAT_10210d30[index] != 0)
        return 1;
    DAT_10daab84[index].m_00a5 = 1;
    return 0;
}

// Like the game-over start in engine_base.cpp without the sound: stores `value`, sets the scene timer and plays
// the player's defeat motion.
extern "C" int FUN_10012100(int value)
{
    DAT_104c0198 = value;
    DAT_104c00d0 = 0x816;
    DAT_10db6ed8[0].model.M_1017b0e0(0x59, 1, 5, 1.0f);
    rand();
    return 0;
}

// Reloads character `index`'s model from the model path table.
extern "C" int FUN_100121e0(int index)
{
    return DAT_10db6ed8[index].model.M_1017aeb0(FUN_1013f8a0(DAT_10b314c8[DAT_10daab84[index].m_0060].path, 0));
}

// Adds character `index`'s position to the list of positions the camera keeps in view.
extern "C" int FUN_100143c0(int index)
{
    DAT_1032d5b8[DAT_1049f0f0] = &DAT_10db6ed8[index].m_2368;
    DAT_1021bbc0[DAT_1049f0f0] = 0;
    DAT_1049f0f0++;
    return 1;
}

// Copies two option settings into their runtime flags: DAT_10c66590 (0..2) to DAT_104c0131 and DAT_10c66594
// (on/off) to DAT_104c0130. Not byte-identical yet (see open.tsv).
extern "C" void FUN_10015910(void)
{
    DAT_104c0131 = 0;
    if (DAT_10c66590 == 0)
        DAT_104c0131 = 0;
    if (DAT_10c66590 == 1)
        DAT_104c0131 = 1;
    if (DAT_10c66590 == 2)
        DAT_104c0131 = 2;
    DAT_104c0130 = 1;
    if (DAT_10c66594 == 0)
        DAT_104c0130 = 0;
}

// Returns the NPC slot (0x5c..0x7f) whose script id is `id`, or -1.
extern "C" int FUN_10017aa0(int id)
{
    for (int i = 0x5c; i < 0x80; i++)
    {
        if (DAT_10219ec0[i] == id)
            return i;
    }
    return -1;
}

// Returns the NPC slot of kind 'F', 'M' or 'N' (shop and town people) whose character id is `id`, or -1.
extern "C" int FUN_10017ad0(int id)
{
    for (int i = 0x5c; i < 0x80; i++)
    {
        char kind = DAT_1021f3c0[i];
        if ((kind == 'F' || kind == 'M' || kind == 'N') && DAT_10db6ed8[i].id == id)
            return i;
    }
    return -1;
}

