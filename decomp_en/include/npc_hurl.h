// The missile-throwing NPC (EN build; JP: decomp/include/npc_hurl.h). Declared once here. Used by src/npc_hurl.cpp.
#ifndef GURUMIN_EN_NPC_HURL_H
#define GURUMIN_EN_NPC_HURL_H

#include "char.h"

// Per-slot mark of the thrower: 0xff while it has not been hit, 0 once it was; its meaning is not recovered. EN 0x0091de38 (JP DAT_10219e40).
extern "C" char g_charHurlMark[CHAR_SLOT_COUNT];

// One frame of the thrower `index` (behaviour 0x26): watches the player, winds up, aims and throws. Returns 0. EN 0x6d2280
// (JP 0x100b6740).
int Npc_HurlAtPlayer(int index);

#endif
