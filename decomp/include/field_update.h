// The field mode update (field_update.cpp, 0x1011a060, game mode 15): globals it owns and its declaration.
#ifndef GURUMIN_FIELD_UPDATE_H
#define GURUMIN_FIELD_UPDATE_H

#include "matrix4.h"
#include "node.h"
#include "vec4.h"

extern "C" int DAT_1020b838;       // third value of the guide marker state (cleared with DAT_1020b830)
extern "C" int DAT_1020c2dc;       // 1 while the menu key is held (copy of the input frame's key byte)
extern "C" int DAT_1020c2f4;       // frames the pointer arrow shows (counts up to 15, down to 0)
extern "C" int DAT_1020c2f8;       // 1 while the player stands near a prompt marker
extern "C" int DAT_1020c328;       // texture slot of the third cloud shadow
extern "C" int DAT_1020c340;       // 1 while the prompt of a nearby character is shown
extern "C" int DAT_104bf880;       // non-zero = the engine ticks are not caught up
extern "C" int DAT_10c63508;       // set after an effect prompt was queued
extern "C" int DAT_10c63514;       // cleared when the queued walk is reset
extern "C" int DAT_10c636dc;       // non-zero = the game was started from a save with the first events done
extern "C" int DAT_10c64700;       // 0 = the four home ghosts use their start state
extern "C" int DAT_10c64a34;       // state of the third home ghost (2 = shown)
extern "C" int DAT_10c64a38;       // state of the fourth home ghost
extern "C" int DAT_10c64a3c;       // state of the second home ghost
extern "C" int DAT_10c64a40;       // state of the first home ghost
extern "C" int DAT_10c655f4;       // set once the costume unlock bit 8 was seen
extern "C" int DAT_10c655f8;       // set together with DAT_10c655f4
extern "C" int DAT_10c657c4;       // 1 once the first event number was queued
extern "C" Node *DAT_1021c638;     // "scop-2pt" (costume 6 only)
extern "C" Node *DAT_1021c63c;     // "scop-3pt" (costume 6 only)
extern "C" float DAT_10f65368;     // the sway of the field (a sine of the frame counter)
extern "C" int DAT_10b35ff8;       // number of lines of the playback command table
extern "C" char DAT_10b3602c[][0x10];   // command word of each line (SET, DIS, RND or DAT)
extern "C" int DAT_10b3792c[][8];  // the six values of each line

// 0x1011a060 (Field_Update; C++ linkage: the original has an EH frame). Game mode 15: one frame of the field.
// Returns 0.
int Field_Update();

#endif
