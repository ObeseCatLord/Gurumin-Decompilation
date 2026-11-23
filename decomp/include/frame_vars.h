// Globals of the frame step (Screenshot_Capture 0x10137b10): per-frame counters and flags that the step clears.
#ifndef GURUMIN_FRAME_VARS_H
#define GURUMIN_FRAME_VARS_H

extern "C" char DAT_104c0348;                             // anti-tamper: times the max-health limit was clamped in the frame step
extern "C" int DAT_102026d0;                              // non-zero = a game load is pending
extern "C" int DAT_11b044d4;
extern "C" int DAT_11b044d8;
extern "C" int DAT_10f623c8;
extern "C" int DAT_10f1fcf4;
extern "C" int DAT_11b07528;
extern "C" int DAT_11b002b0;
extern "C" int DAT_10f623b8;
extern "C" int DAT_10f623bc;
extern "C" int DAT_10f623c0;
extern "C" int DAT_10f623c4;
extern "C" int DAT_1101d858;
extern "C" int DAT_10210f30;                              // controller state read by the frame step (non-zero ends it)
extern "C" void (*DAT_11b1bb80)(const char *path, unsigned char *pixels, int width, int height, int quality, int flags);   // saves a BGR picture as a JPEG

#endif
