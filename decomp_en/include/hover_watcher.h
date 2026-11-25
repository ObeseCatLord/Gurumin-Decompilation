// Hovering watcher state (EN build; JP: char_tables.h).
#ifndef GURUMIN_EN_HOVER_WATCHER_H
#define GURUMIN_EN_HOVER_WATCHER_H

#include "char.h"
#include "vec4.h"

// Where each wave enemy came in; its z is the height a hovering watcher drifts back to. EN 0x00bc99a8 (JP DAT_104c4438).
extern "C" Vec4 g_charHomes[CHAR_SLOT_COUNT];

// One frame of watcher `index`: faces the player, idles, puts its shadow on the ground, drifts back to its home height and
// turns smoothly. Returns 0. EN 0x6ac8d0 (JP 0x10026730).
extern "C" int Watcher_Update(int index);

#endif
