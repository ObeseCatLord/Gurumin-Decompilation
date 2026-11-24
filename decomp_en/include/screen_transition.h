// Screen transitions (fades between scenes) of the EN build.
#ifndef GURUMIN_EN_SCREEN_TRANSITION_H
#define GURUMIN_EN_SCREEN_TRANSITION_H

// Starts (mode != 0) or ends (mode 0) a transition: swaps the double-buffered capture target, records the mode and
// redraws the frame once. Returns 0. EN 0x77dc60 (JP 0x1015c9e0).
extern "C" int Screen_StartTransition(int mode);

#endif
