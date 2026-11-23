// StepRecord: declared here once (first used by src/route_walk.cpp).
#ifndef GURUMIN_STEP_RECORD_H
#define GURUMIN_STEP_RECORD_H

#include "anim_model.h"

// A 0x40-byte record per character slot (DAT_10802c08): the last step taken.
struct StepRecord
{
    Vec4 step;
    char pad_0010[0x30];
};

#endif
