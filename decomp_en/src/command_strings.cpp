// Setting the command strings of the scene script reader (EN build, EN-only: no JP twin found).
#include <string.h>
#include "command_strings.h"

// Replaces command string `index` (clamped to the first and last string) with `text` and rewinds its read position.
// Returns 0. Called by the script frame update at the start of a scene and when a script sets a string.
extern "C" int CommandString_Set(int index, const char *text)
{
    int slot = index > 0 ? index : 0;
    slot = slot > COMMAND_STRING_COUNT - 1 ? COMMAND_STRING_COUNT - 1 : slot;
    strcpy(g_commandStrings[slot], text);
    g_commandStringCursors[slot] = 0;
    return 0;
}
