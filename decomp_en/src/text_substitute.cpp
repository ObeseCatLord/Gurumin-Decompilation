// Small text helpers shared by the script and message code (EN build).
#include "text_substitute.h"

// Copies the zero-terminated string `from` to `to`.
static inline void CopyString(char *to, const char *from)
{
    while ((*to++ = *from++) != 0)
    {
    }
}

// Text_CopyStripMarker: copies the message `text` to `out` including the terminator, skipping a leading '#' marker.
// Called when a message or script text is opened.
extern "C" int Text_CopyStripMarker(char *out, const char *text)
{
    if (*text == TEXT_LABEL_MARKER)
        CopyString(out, text + 1);
    else
        CopyString(out, text);
    return 1;
}
