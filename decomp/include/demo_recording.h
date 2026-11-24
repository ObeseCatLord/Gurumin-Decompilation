// DemoRecording: declared here once (first used by src/demo_record.cpp).
#ifndef GURUMIN_DEMO_RECORDING_H
#define GURUMIN_DEMO_RECORDING_H

#include "input_state.h"

struct DemoRecording : InputState
{
    int values_now[5];              // 0x20: its five values
    char pad_0034[0xc];
    float floats_now[9];            // 0x40: its nine floats (positions of the player, the second model and the camera target)
    char pad_0064[0x10];
    int playFrame;                  // 0x74: next frame to play back
    int frameCount;                 // 0x78: number of recorded frames (at most 36000 = 20 minutes at 30 Hz)
    int pad_007c;
    int values[5][36000];           // 0x80: five 32-bit values per frame (analogue input and the like)
    char buttons[0x1c][36000];      // 0xafd00: 28 button bytes per frame
    int Load(const char *path);
    int PlayFrame();
    int RecordFrame();
    int M_1015caa0(int value, int index);     // 0x1015caa0: stores value `index` of values_now
    int M_1015cac0(float value, int index);   // 0x1015cac0: stores float `index` of floats_now
};

#endif
