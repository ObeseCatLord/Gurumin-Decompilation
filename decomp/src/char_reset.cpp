// Resetting a character's state (called by many enemy and NPC behaviours when they start over). The character
// records are viewed from their model part at 0x10db6ee8 (like char_move.cpp), where all the fields used here fall
// inside one DAT_10db6ee8[index].

#include <string.h>

#include "vec4.h"

#include "anim_model.h"

#include "char_slot.h"

#include "char_state.h"

#include "start_vectors.h"

#include "pose_entry.h"

extern "C" StartVectors DAT_10802bf8[0x80];
extern "C" PoseEntry DAT_1021bdf8[0x80];
#include "char_tables.h"

extern "C" int DAT_102199b8[0x80];    // per-character group timer
extern "C" void FUN_1000ec70(int frames, float r, float g, float b, float a);   // ColourFade_Start
extern "C" int FUN_100404e0(int index, int a, int b, int c, int d, int e, int f);
extern "C" int FUN_1001b0a0(int index, int a, int b);

// 0x10043cc0 (Char_ResetState). Starts character `index` over: a kind 0x62 character in mode 11 or 12 first fades
// the screen to white over 15 frames. The AI state, the start vectors and the pose entry are cleared, a few flags
// are reset, the group timer is set to 16 and the character is marked in play. Then FUN_100404e0 is called with the
// four fields of its packed value (BB, CC, DD, AA) and FUN_1001b0a0 with zeros. Returns 0.
extern "C" int FUN_10043cc0(int index)
{
    if (DAT_10daaae8[index].kind == 0x62 && (DAT_1020b430[index].behaviour == 11 || DAT_1020b430[index].behaviour == 12))
        FUN_1000ec70(15, 1.0f, 1.0f, 1.0f, 1.0f);
    DAT_10daaae8[index].m_0141 = 0;
    memset(&DAT_1020b430[index], 0, sizeof(CharState));
    DAT_10db6ee8[index].m_0267 = 0;
    DAT_10db6ee8[index].mode = 1;
    DAT_10255a88[index] = 1;
    DAT_10db6ee8[index].m_008d = 0;
    DAT_10802bf8[index].position.Fill(0);
    DAT_10802bf8[index].m_0010.Fill(0);
    DAT_10db6ee8[index].m_0012 = 0;
    memset(&DAT_1021bdf8[index], 0, sizeof(PoseEntry));
    int value = DAT_102548c0[index];
    DAT_102199b8[index] = 16;
    DAT_1020d540[index] = 1;
    FUN_100404e0(index, value / 10000 % 100, value / 100 % 100, value % 100, value / 1000000 % 100, 1, 0);
    FUN_1001b0a0(index, 0, 0);
    return 0;
}
