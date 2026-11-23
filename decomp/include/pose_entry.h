// PoseEntry: declared here once (first used by src/char_reset.cpp).
#ifndef GURUMIN_POSE_ENTRY_H
#define GURUMIN_POSE_ENTRY_H

// Per-character pose table entry, 0x24 bytes (pulse_scale.cpp).
struct PoseEntry
{
    int words[9];
};

#endif
