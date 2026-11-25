// Frame pacer state of the EN build (JP: render_vars.h, win_vars.h).
#ifndef GURUMIN_EN_FRAME_PACER_H
#define GURUMIN_EN_FRAME_PACER_H

extern "C" int g_smoothMode;                  // EN 0x021e47d4 (JP DAT_10f654f0): 1 = wait with Sleep and run one step per frame
extern "C" int g_debugKeysEnabled;            // EN 0x0090933c (JP DAT_10202b08): non-zero = right Shift and right Ctrl change the pacing
extern "C" unsigned int g_frameStepCount;     // EN 0x021e47f8 (JP DAT_10f65508): 60 Hz steps so far; every second one is an engine tick
extern "C" int g_engineTickCount;             // EN 0x021e47f4 (JP DAT_10f65504): engine ticks so far
extern "C" int g_framesSinceMeasure;          // EN 0x021e47c8 (JP DAT_10f654dc): frames since the frame rate was last measured
extern "C" unsigned long g_measureStartTime;  // EN 0x021e47cc (JP DAT_10f654e0): timeGetTime() when the frame rate was last measured
extern "C" float g_measuredFrameRate;         // EN 0x021e47d0 (JP DAT_10f654e8): measured frames per second

// Ages the texture slots and the texture cache once per drawn frame. EN 0x77d3e0 (JP 0x10147a00), 0x7a24d0 (JP 0x101483f0).
extern "C" void TextureSlots_Age(void);
extern "C" void TextureCache_Age(void);
// Steps of the 60 Hz clock: waits for the deadline and runs the engine ticks that fell due. Returns 1.
// EN 0x7a8460 (JP 0x1014ade0).
extern "C" int Frame_Pace(void);

#endif
