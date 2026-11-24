// Costume description text (EN build). Ported from the matched JP source decomp/src/text_fields.cpp.
#include <string.h>

#include "ability_text.h"

// Separator between ability names: the Shift-JIS middle dot.
const char *const ABILITY_NAME_SEPARATOR = "\x81\x45";

// Ability_BuildNameList: builds the list of ability names for the ability bits in `flags` (Ability_GetDescription
// returns one name and its bit at a time), separated by the middle dot. Returns 0. Used for the costume description.
extern "C" int Ability_BuildNameList(unsigned int flags, char *out)
{
    char text[ABILITY_NAME_SIZE];
    int any = 0;
    *out = 0;
    while (flags != 0)
    {
        unsigned int bit = Ability_GetDescription(flags, text);
        if (bit == 0)
            break;
        if (any)
            strcat(out, ABILITY_NAME_SEPARATOR);
        strcat(out, text);
        flags &= ~bit;
        any = 1;
    }
    return 0;
}
