// The text an IME is still composing (EN build, EN-only: no JP twin found).
#include <stdlib.h>
#include <string.h>
#include "ime.h"

// ImeContext::GetCompositionString: copies the text being composed (not yet converted) into `out`, empty when there is
// none. Returns 1 when text was copied. No direct caller.
int ImeContext::GetCompositionString(char *out)
{
    *out = 0;
    LONG length = ImmGetCompositionStringA(context, GCS_COMPSTR, 0, 0);
    if (length == 0)
        return 0;
    char *text = (char *)operator new(length + 1);
    ImmGetCompositionStringA(context, GCS_COMPSTR, text, length);
    text[length] = 0;
    strcpy(out, text);
    free(text);
    return 1;
}
