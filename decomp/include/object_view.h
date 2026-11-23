// ObjectView: declared here once (first used by src/object_spawn.cpp).
#ifndef GURUMIN_OBJECT_VIEW_H
#define GURUMIN_OBJECT_VIEW_H

#include "game_vars.h"

// Character fields used only by objects.
struct ObjectView
{
    char pad_0000[0x22c];
    Vec4 colour;              // 0x22c: model colour (r, g, b, a)
    char pad_023c[0x3a];
    char m_0276;              // 0x276: set to 1 for objects
};

#endif
