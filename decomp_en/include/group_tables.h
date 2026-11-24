// Attachment slots and smoothing helpers of the EN build (JP: group_tables.cpp). This is the one declaration of
// these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_GROUP_TABLES_H
#define GURUMIN_EN_GROUP_TABLES_H

#include "vec4.h"

// Attachment slots that can be in use at once.
const int ATTACH_SLOT_COUNT = 6;

// Owner of each attachment slot (0 = free). EN 0x0091c570 (JP DAT_102108d0).
extern "C" int g_attachOwner[ATTACH_SLOT_COUNT];
// Transform of each attachment slot. EN 0x015e9b48 (JP DAT_10f08880).
extern "C" Matrix4 g_attachMatrix[ATTACH_SLOT_COUNT];

// Stores a copy of `matrix` moved by (x, y, z) in the first free attachment slot for `owner`. Returns the slot, or 0
// when all are in use. EN 0x5ec130 (JP 0x10034f70).
extern "C" int AttachSlot_Add(const Matrix4 *matrix, float x, float y, float z, int owner);
// Moves `current` towards `target` in shrinking steps; the result goes to *out. EN 0x5ee370 (JP 0x10034810).
extern "C" int SmoothApproach(float target, float current, float *out, float steps, int first);

#endif
