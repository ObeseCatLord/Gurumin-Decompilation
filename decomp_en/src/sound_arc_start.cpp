// Starting the sound engine with the game window (EN build, EN-only: no JP twin found).
#include "bgm.h"

// Remembers the window (a non-zero value also switches the music bars on), hands it to the sound engine and finishes
// the engine's start-up. `unused` is not read. Returns 1. Called by the game start-up at EN 0x5f2cd0.
extern "C" int SoundArc_Start(HWND window, const char *unused)
{
    g_bgmBarsShown = (int)window;
    g_soundArcSetWindow(window);
    g_soundArcFinishStart();
    return 1;
}
