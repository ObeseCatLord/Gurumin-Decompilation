// DemoRecording: declared here once (first used by src/demo_record.cpp).
#ifndef GURUMIN_DEMO_RECORDING_H
#define GURUMIN_DEMO_RECORDING_H

struct DemoRecording
{
    int pad_0000;
    char buttons_now[0x1c];         // 0x04: the 28 button bytes of the frame being played back
    int values_now[5];              // 0x20: its five values
    char pad_0034[0x40];
    int playFrame;                  // 0x74: next frame to play back
    int frameCount;                 // 0x78: number of recorded frames (at most 36000 = 20 minutes at 30 Hz)
    int pad_007c;
    int values[5][36000];           // 0x80: five 32-bit values per frame (analogue input and the like)
    char buttons[0x1c][36000];      // 0xafd00: 28 button bytes per frame
    int Load(const char *path);
    int PlayFrame();
    int RecordFrame();
};

#endif
