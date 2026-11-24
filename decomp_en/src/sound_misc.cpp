// EN sound commands (JP twin: app_0002.cpp).

#include "sound.h"

// Stops all sound (a command with no arguments). Returns 1.
extern "C" int Sound_StopAll(void)
{
    Sound_Command(0, 0, 0, 0, 0, 0);
    return 1;
}
