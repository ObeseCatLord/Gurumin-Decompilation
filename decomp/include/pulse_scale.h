// Pose, PoseRef: declared here once (first used by src/pulse_scale.cpp).
#ifndef GURUMIN_PULSE_SCALE_H
#define GURUMIN_PULSE_SCALE_H

#include "matrix4.h"

// Pose object of a character (pointer table DAT_1021bdf8, 0x24 bytes per character); matrix at 0x1cc.
struct Pose
{
    char pad_0000[0x1cc];
    Matrix4 matrix;
};
struct PoseRef
{
    Pose *pose;
    int pad_0004[8];
};

#endif
