// The partner follower (partner_follow.cpp): the per-frame update of a partner in the party line behind the player
// (enemy_queue.h), and the globals and engine calls it uses.
#ifndef GURUMIN_PARTNER_FOLLOW_H
#define GURUMIN_PARTNER_FOLLOW_H

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "effect_manager.h"

// Character slot of the partner whose leaving is a show (a screen-wide colour fade, the fog dimmed, rings of effects).
#define PARTNER_GALAXY 0x19

extern "C" int DAT_10c66cdc[3];        // where a new partner appears: x, y, z in whole units
extern "C" int DAT_1020b5d4;           // slot 6 stays while this is 1..99 (unknown counter)
extern "C" int DAT_1020b6f0;           // set to 6 when slot 0xb has faded back in
extern "C" int DAT_1020b85c;           // set to 0x4b when slot 0x10 appears
extern "C" Node *DAT_1021c03c;         // slot 0x10's "Bip01 Footsteps" node
extern "C" Node *DAT_1021bed4;         // slot 6's "Bip01" node
extern "C" Node *DAT_1021bf88;         // slot 0xb's "Bip01 Head" node
extern "C" Node *DAT_1021bf8c;         // the player's "@Dummy05" node (found for slot 0xb)
extern "C" int DAT_1020ba9c;           // frames since the show partner began to leave (0 = not leaving)
extern "C" int DAT_1020baa8;           // frames since the show partner began its exit show (0 = none)
extern "C" float DAT_102552f8;         // camera value saved while the show partner talks (DAT_1020344c)
extern "C" Vec4 DAT_108689b8;          // the map root's fog colour saved before the show partner dims it
extern "C" Vec4 DAT_108689c8;          // the same for the second fog root (DAT_10255f0c)
extern "C" Matrix4 DAT_1086adb8;       // the matrix the show partner's effects follow (at the player)

// Engine calls with no header of their own yet.
extern "C" int FUN_10056260(int slot, const char *line1, const char *line2, const char *line3,
                            const char *line4);                                              // Message_Open
extern "C" void FUN_1000ec70(int frames, float r, float g, float b, float a);                // ColourFade_Start
extern "C" int FUN_10054550(int sound, int delay, const Vec4 *position);                     // Sound_PlayDelayed

#endif
