// Composition queries of the IME context whose answers are not used (EN build, EN-only: no JP twin found).
#include "ime.h"

// Asks Imm32 for the start of the last composition change and for the cursor position inside the composition; neither
// answer is kept. No direct caller.
void ImeContext::QueryDeltaStartAndCursor()
{
    ImmGetCompositionStringA(context, GCS_DELTASTART, 0, 0);
    ImmGetCompositionStringA(context, GCS_CURSORPOS, 0, 0);
}
