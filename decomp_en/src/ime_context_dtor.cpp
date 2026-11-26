// Destructor of the IME context wrapper (EN build, EN-only: no JP twin found).
#include <stdlib.h>

#include "ime.h"

// Frees the candidate buffer and releases the input context, the same work as Release (which the original also keeps as
// an out-of-line function with a return value).
ImeContext::~ImeContext()
{
    if (buffer != 0)
    {
        free(buffer);
        buffer = 0;
    }
    if (context != 0)
    {
        ImmReleaseContext(window, context);
        context = 0;
        window = 0;
    }
}
