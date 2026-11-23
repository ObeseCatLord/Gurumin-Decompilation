// Face (expression) animation of characters: each character has a table of 0x40 face frames (texture names) and a
// face set that says, for each frame, how long it is shown and which frame follows (app_0009.cpp counts the delays
// down each frame).

#include <string.h>
#include "texture_load.h"
#include "world.h"
#include "anim_model.h"
#include "char_state.h"

// Loads ITI texture `name` from the data folder (the ITI branch of LoadTexture, texture_load.h).
inline int LoadIti(const char *name)
{
    return FUN_10174ce0(FUN_1013f8a0(name, 0), 0, 0);
}

struct FaceSet
{
    int delay[0x40];          // frames to show each face before switching
    int next[0x40];           // following face (the same face = stay)
    char pad_0200[0x100];
    char names[0x40][0x10];   // 0x300: face texture names (indexed by character, see below)
};

#include "char_slot.h"

extern "C" FaceSet DAT_1087664c[0x80];
extern "C" short DAT_10b314c0;           // last face frame set
extern "C" int DAT_10c655f0;             // current costume
extern "C" int DAT_1030c278[0x80];       // frames until the next face change (0 = none)
extern "C" int DAT_10447dd8[0x80];       // face to show next

// Not byte-identical yet: register roles of index/frame and the face-table index form differ (see open.tsv).
// Shows face `frame` on character `index` and schedules the next face of its set. The player in costume 6 uses the
// costume's own face textures: the same names with "14" in characters 3 and 4. Returns 0.
extern "C" int FUN_1000f360(int index, int frame)
{
    if (DAT_10daaaec[index].faceSet < 0)
        return 0;
    DAT_10b314c0 = (short)frame;
    if (index == 0 && DAT_10c655f0 == 6)
    {
        char name[256];
        strcpy(name, DAT_1087664c[0].names[frame]);
        name[2] = '1';
        name[3] = '4';
        DAT_10db6ee8[index].M_10161a20(LoadIti(name));
    }
    else
        DAT_10db6ee8[index].M_10161a20(LoadIti(DAT_1087664c[index].names[frame]));
    int next = DAT_1087664c[DAT_10daaaec[index].faceSet].next[frame];
    if (next == frame)
        DAT_1030c278[index] = 0;
    else
    {
        DAT_1030c278[index] = DAT_1087664c[DAT_10daaaec[index].faceSet].delay[frame] + 1;
        DAT_10447dd8[index] = next;
    }
    return 0;
}
