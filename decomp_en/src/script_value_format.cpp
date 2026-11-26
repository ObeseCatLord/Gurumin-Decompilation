// Formatting an evaluated script value as text (EN build, EN-only: no JP twin found).
#include <stdio.h>
#include "listbank.h"

// Index of the last character of a text buffer (LISTBANK_TEXT_SIZE bytes), which is always cleared.
#define LAST_TEXT_INDEX (LISTBANK_TEXT_SIZE - 1)

// ScriptValue_Format: writes `value` as text into `buffer` (LISTBANK_TEXT_SIZE bytes): an int with "%d", a float with
// "%f", a text with "%s"; any other type leaves the buffer as it is. The last character of the buffer is cleared in
// every case except the text type. Returns 1. Called when a script value is printed or concatenated (EN 0x5e4390).
int __stdcall ScriptValue_Format(char *buffer, ScriptValue *value, char type)
{
    if (type == SCRIPT_TYPE_INT)
    {
        sprintf(buffer, "%d", value->i());
        buffer[LAST_TEXT_INDEX] = 0;
        return 1;
    }
    if (type == SCRIPT_TYPE_FLOAT)
    {
        sprintf(buffer, "%f", value->f());
        buffer[LAST_TEXT_INDEX] = 0;
        return 1;
    }
    if (type == SCRIPT_TYPE_TEXT)
    {
        sprintf(buffer, "%s", value->text);
        buffer[LAST_TEXT_INDEX] = 0;
        return 1;
    }
    buffer[LAST_TEXT_INDEX] = 0;
    return 1;
}
