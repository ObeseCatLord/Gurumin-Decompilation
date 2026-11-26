// The per-frame step of the game (EN build; JP: frame_vars.h, frame_step.cpp) and the globals only it uses. Declared
// once here.
#ifndef GURUMIN_EN_FRAME_STEP_H
#define GURUMIN_EN_FRAME_STEP_H

#include <windows.h>

extern "C" int g_windowActive;            // EN 0x00909350 (JP DAT_10f656a0): 1 = keep running while the window is inactive
extern "C" HWND g_windowHandle;           // EN 0x0090934c (JP DAT_10f6569c): the game window
extern "C" int g_healthClampCountA;       // EN 0x00bc8888 (JP DAT_104c0344): anti-tamper: times the first and third check clamped the max health
extern "C" char g_healthClampCountB;      // EN 0x00bc887e (JP DAT_104c0348): anti-tamper: times the second check clamped the max health
extern "C" int g_loadDataPending;         // EN 0x00908444 (JP DAT_102026d0): non-zero = the map data still has to be loaded
extern "C" char g_frameStepDataReady;     // EN 0x00b91c81 (JP DAT_104bfd71): set to 1 by every frame step
extern "C" char g_pauseMessage;           // EN 0x0092f230 (JP DAT_104bfec0): first byte of the message shown while the game is paused (0 = none)
// Copies `text` into the pause message buffer (g_pauseMessage). Returns 0. EN 0x5e79f0 (pause_message.cpp, EN-only).
extern "C" int Pause_SetMessage(const char *text);
extern "C" int g_pauseControls;          // EN 0x00924d54 (JP DAT_10210f30): controller state read while the pause message shows
// Per-frame statistics and counters the frame step clears before the game mode runs.
extern "C" int g_frameCounterA;           // EN 0x0217faf8 (JP DAT_11b044d4)
extern "C" int g_frameCounterB;           // EN 0x0217b8d8 (JP DAT_11b044d8)
extern "C" int g_frameCounterC;           // EN 0x021a349c (JP DAT_10f623c8)
extern "C" int g_frameCounterD;           // EN 0x021a34a0 (JP DAT_10f1fcf4)
extern "C" int g_frameCounterE;           // EN 0x0217fb4c (JP DAT_11b002b0)
extern "C" int g_frameCounterGroup[4];    // EN 0x021a351c (JP DAT_10f623b8..10f623c4)
extern "C" int g_frameCounterF;           // EN 0x0166396c (JP DAT_1101d858)

// Game modes (g_gameMode) that have an update of their own in the frame step.
const int MODE_TITLE = 0;
const int MODE_PLAY = 1;
const int MODE_ITEM_DICTIONARY = 5;
const int MODE_NAME_ENTRY = 10;
const int MODE_FIELD = 15;

// The mode updates the frame step dispatches to.
extern "C" int TitleScreen_Update(void);      // EN 0x62cb70 (JP 0x10114950)
extern "C" int Gameplay_Update(void);         // EN 0x60cb10 (JP 0x1011d810)
extern "C" int ItemDictionary_Update(void);   // EN 0x6e1700 (JP 0x10115cd0)
extern "C" int NameEntry_Update(void);        // EN 0x5b9bf0 (JP 0x10116550)
extern "C" int Field_Update(void);            // EN 0x631890 (JP 0x1011a060)
// Reads the map data (bin\map.txt) in steps; returns 0 while it is not finished. EN 0x5f2cd0 (JP 0x100cb0c0).
extern "C" int Game_LoadData(void);

// Pumps the window messages, limits the player's health (breaches close the game with some probability), clears the
// per-frame counters, runs the update of the current game mode, paces and presents the frame. Returns 1 to go on, 0
// when the game should end. EN 0x60c750 (JP 0x10137b10; the EN build has no screenshot code). Called by the game loop.
extern "C" int Game_FrameStep(void);

#endif
