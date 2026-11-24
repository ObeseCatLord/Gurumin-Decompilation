// IME support of the name entry screen (EN build). Ported from the matched JP source decomp/src/ime.cpp.
#include <stdlib.h>
#include <new>

#include "ime.h"

// Ime_IsOpen: 1 while text input is active. Called by the text input and menu code.
extern "C" int Ime_IsOpen(void)
{
    return g_textInputActive;
}

// TextInput_End: ends text input: clears the active flag and the second input flag. Called when name entry finishes.
extern "C" int TextInput_End(void)
{
    g_textInputActive = 0;
    g_textInputFlag = 0;
    return 0;
}

// Switches the IME on or off. Returns 1 on success.
int ImeContext::SetOpen(BOOL open)
{
    return ImmSetOpenStatus(context, open) != 0;
}

// Reads the conversion and sentence modes. Returns 1 on success.
int ImeContext::GetConversion(DWORD *conversion, DWORD *sentence)
{
    return ImmGetConversionStatus(context, conversion, sentence) != 0;
}

// Sets the conversion and sentence modes. Returns 1 on success.
int ImeContext::SetConversion(const DWORD *conversion, const DWORD *sentence)
{
    return ImmSetConversionStatus(context, *conversion, *sentence) != 0;
}

// Releases the input context and frees the candidate buffer.
int ImeContext::Release()
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
    return 1;
}

// Takes the input context of window `target`.
int ImeContext::Attach(HWND target)
{
    Release();
    if (target != 0)
    {
        window = target;
        context = ImmGetContext(target);
    }
    return 1;
}

// Loads the candidate list into the context's buffer and returns the number of candidates.
DWORD ImeContext::LoadCandidates()
{
    if (buffer != 0)
    {
        free(buffer);
        buffer = 0;
    }
    DWORD size = ImmGetCandidateListA(context, 0, 0, 0);
    if (size == 0)
        return 0;
    buffer = (CANDIDATELIST *)operator new(size);
    ImmGetCandidateListA(context, 0, buffer, size);
    return buffer->dwCount;
}
