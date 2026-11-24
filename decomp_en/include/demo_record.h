// DemoRecording of the EN build (JP: decomp/include/demo_recording.h, input_state.h): a recording of the input of
// up to 36000 frames. This is the one declaration of it in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_DEMO_RECORD_H
#define GURUMIN_EN_DEMO_RECORD_H

// Number of recorded button bytes per frame, values per frame and frames per recording (20 minutes at 30 Hz).
const int DEMO_BUTTON_COUNT = 0x1c;
const int DEMO_VALUE_COUNT = 5;
const int DEMO_MAX_FRAMES = 36000;

struct DemoRecording
{
    char m_0000[4];                                 // 0x00: not used by EN code yet
    char buttons_now[DEMO_BUTTON_COUNT];            // 0x04: the current frame's button bytes
    int values_now[DEMO_VALUE_COUNT];               // 0x20: the current frame's five values
    char m_0034[0x74 - 0x34];                       // 0x34: floats and padding (not used by EN code yet)
    int playFrame;                                  // 0x74: next frame to record or play back
    int frameCount;                                 // 0x78: number of recorded frames
    int m_007c;                                     // 0x7c: not used by EN code yet
    int values[DEMO_VALUE_COUNT][DEMO_MAX_FRAMES];  // 0x80: five 32-bit values per frame
    char buttons[DEMO_BUTTON_COUNT][DEMO_MAX_FRAMES];  // 0xafd00: 28 button bytes per frame

    int Load(const char *path);     // EN 0x7af4d0 (JP 0x1015cc90)
    int PlayFrame();                // EN 0x7af5c0 (JP 0x1015cf30)
    int RecordFrame();              // EN 0x7af420 (JP 0x1015cd70)
};

#endif
