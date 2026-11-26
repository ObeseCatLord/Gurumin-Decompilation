// Text helpers of the script reader (EN build, EN-only: no JP twin found).
#include <string.h>

#include "script_text.h"

// Tells whether `text` is a quoted string: its first and its last character are double quotes. Returns 1 or 0. No direct caller.
int __stdcall Text_IsQuoted(const char *text)
{
    size_t length = strlen(text);
    if (text[0] == '"' && text[length - 1] == '"')
        return 1;
    return 0;
}
