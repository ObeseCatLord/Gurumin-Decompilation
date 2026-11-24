// IME support of the name entry screen (EN build). Ported from the matched JP source decomp/src/ime.cpp.
#include <stdlib.h>
#include <new>
#include <string.h>

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

// Copies the finished (converted) text into `out` (empty when nothing was converted). Returns 1 when text was copied.
int ImeContext::GetResultString(char *out)
{
    *out = 0;
    LONG length = ImmGetCompositionStringA(context, GCS_RESULTSTR, 0, 0);
    if (length == 0)
        return 0;
    char *text = (char *)operator new(length + 1);
    ImmGetCompositionStringA(context, GCS_RESULTSTR, text, length);
    text[length] = 0;
    strcpy(out, text);
    free(text);
    return 1;
}

// Writes the IME input mode as a short Japanese label into `out`, built from the conversion and sentence modes: "R"
// (romaji input), full or half width, katakana or hiragana, then the sentence-mode marks (renbunsetsu, fukugo,
// tanbunsetsu, jidou). Shift-JIS bytes are spelled out in the strings. Returns 1.
int ImeContext::GetModeText(char *out)
{
    DWORD conversion;
    DWORD sentence;
    out[0] = 0;
    GetConversion(&conversion, &sentence);
    if (conversion & IME_CMODE_ROMAN)
        strcat(out, "\x82\x71");                          // "R" (full width)
    if (conversion & IME_CMODE_FULLSHAPE)
        strcat(out, "\x91\x53");                          // "zen" (full width)
    else
        strcat(out, "\x94\xbc");                          // "han" (half width)
    if (conversion & IME_CMODE_KATAKANA)
        strcat(out, "\x83\x4a\x83\x69");                  // "kana" in katakana
    else
        strcat(out, "\x82\xa9\x82\xc8");                  // "kana" in hiragana
    if (sentence & IME_SMODE_PHRASEPREDICT)
        strcat(out, "\x98\x41");                          // "ren"
    if (sentence & IME_SMODE_PLAURALCLAUSE)
        strcat(out, "\x95\xa1");                          // "fuku"
    if (sentence & IME_SMODE_SINGLECONVERT)
        strcat(out, "\x92\x50");                          // "tan"
    if (sentence & IME_SMODE_AUTOMATIC)
        strcat(out, "\x8e\xa9");                          // "ji"
    return 1;
}
