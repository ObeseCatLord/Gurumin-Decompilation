// EN-only tiny functions without a JP twin (batch 11, section 4): Steam text input callback, scratch block.
#include <stdlib.h>

#include "small_b11.h"
#include "steam.h"

void *g_scratchBlock;

// Steam calls this when the on-screen keyboard closes: stores whether the text was submitted and marks the input as
// dismissed so the name entry reads the text.
void TextInputReceiver::OnDismissed(const GamepadTextInputDismissed *result)
{
    *submittedOut = result->submitted;
    g_steamTextInputShown = STEAM_TEXT_INPUT_DISMISSED;
}

// Frees the scratch block (when there is one) and forgets it. Returns 0. Called at shutdown.
extern "C" int ScratchBlock_Free(void)
{
    if (g_scratchBlock != 0)
        free(g_scratchBlock);
    g_scratchBlock = 0;
    return 0;
}

// Points the view at a block. Returns 1. Called wherever a caller hands over a buffer for the program to fill.
int BufferView::Set(void *newData, int newSize)
{
    data = newData;
    size = newSize;
    return 1;
}
