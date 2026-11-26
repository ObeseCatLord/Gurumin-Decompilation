// The message shown while the game is paused (EN build, EN-only: no JP twin found).
#include <string.h>
#include "frame_step.h"

// Copies `text` into the pause message buffer. Returns 0. No direct caller.
extern "C" int Pause_SetMessage(const char *text)
{
    strcpy(&g_pauseMessage, text);
    return 0;
}
