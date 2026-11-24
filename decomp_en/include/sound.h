// The sound engine's command entry point (EN build, compiled into the exe). Declared once here.
#ifndef GURUMIN_EN_SOUND_H
#define GURUMIN_EN_SOUND_H

// Sends a command to the sound engine: six arguments (all 0 stops every sound). EN 0x7aa3a0 (JP soundarc 0x10001170).
extern "C" void __stdcall Sound_Command(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5);

// Stops all sound. Returns 1. EN 0x7aa280 (JP 0x10001550).
extern "C" int Sound_StopAll(void);

#endif
