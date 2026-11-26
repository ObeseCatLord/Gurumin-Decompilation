// IME globals and context of the EN build (declared once here; JP equivalents in decomp/include/ime.h, ui_vars.h).
#ifndef GURUMIN_EN_IME_H
#define GURUMIN_EN_IME_H

#include <windows.h>
#include <imm.h>

#include "window.h"

// Wrapper of the Imm32 input context used by the name entry screen. Only the members EN code has used so far.
struct ImeContext
{
    HWND window;              // 0x00: window the context was taken from
    HIMC context;             // 0x04: the input context (Imm32 handle)
    int m_0008;               // 0x08: not yet named
    CANDIDATELIST *buffer;    // 0x0c: candidate list loaded by LoadCandidates (0 = none)
    int m_0010;               // 0x10: zeroed by the constructor, not used by other ported code yet

    ImeContext();                                                       // EN 0x7aab10 (EN-only, ime_context_ctor.cpp): no window, no context, no candidate list
    ~ImeContext();                                                     // EN 0x7aab30: releases the context like Release
    int Release();                                                      // EN 0x7aaf70 (JP 0x1015d0a0)
    int Attach(HWND target);                                            // EN 0x7aaff0 (JP 0x1015d0e0)
    int SetOpen(BOOL open);                                             // EN 0x7aafb0 (JP 0x1015d120)
    int GetConversion(DWORD *conversion, DWORD *sentence);              // EN 0x7aad50 (JP 0x1015d140)
    int SetConversion(const DWORD *conversion, const DWORD *sentence);  // EN 0x7aafd0 (JP 0x1015d160)
    int GetResultString(char *out);                                     // EN 0x7aacc0 (JP ImeContext::GetResultString): copies the finished text into `out`
    int GetCompositionString(char *out);                                // EN 0x7aaee0 (EN-only, ime_composition.cpp): copies the text still being composed into `out`
    void QueryDeltaStartAndCursor();                                    // EN 0x7aac90 (ime_context_queries.cpp, EN-only): asks Imm32 for the composition change start and the cursor position and drops both answers
    int GetModeText(char *out);                                        // EN 0x7aad70 (JP 0x1015d180): writes the input mode label into `out`
    DWORD GetReadingClauseEnd();                                        // EN 0x7aabd0 (EN-only, ime_clauses.cpp): end of the first reading clause of the composition, 0 when the IME gives none
    DWORD GetClauseEnd();                                               // EN 0x7aac00 (EN-only, ime_clauses.cpp): end of the first clause of the composition, 0 when the IME gives none
    DWORD LoadCandidates();                                          // EN 0x7aac30 (JP 0x1015d430)
};

extern "C" ImeContext g_imeContext;     // EN 0x022280b8 (JP DAT_10f68124): the game's IME context
extern "C" int g_textInputAllowed;      // EN 0x009094f4 (JP DAT_10203428): 1 = text input allowed
extern "C" int g_textInputState;        // EN 0x022280b0 (JP DAT_10f68110): cleared when text input starts
extern "C" int g_textInputCursor;       // EN 0x022280a0 (JP DAT_10f66998): cleared when text input starts
extern "C" char *g_textInputBuffer;     // EN 0x02226944 (JP DAT_10f68118): buffer that receives the typed text
extern "C" int g_textInputActive;   // EN 0x02228074 (JP DAT_10f67000): 1 while text input (IME composition) is active
typedef void (*KeyDownCallback)(WPARAM keyCode, LPARAM keyData);   // called by the window procedure on a key press
extern "C" KeyDownCallback g_textInputFlag;     // EN 0x021e4834: key press callback of the name entry (0 = none), cleared when input ends

// Text input callbacks installed in the game window.
extern "C" void Ime_ReadCompositionString();   // EN 0x7ab050 (JP 0x1015f3e0): reads the composition string
extern "C" void Ime_ReadResultString();        // EN 0x7ab070 (JP 0x1015f400): reads the result string
extern "C" void Ime_ReadCursorPosition();      // EN 0x7ab0f0 (JP 0x1015f490): reads the cursor position
extern "C" void Ime_ReadAttributes();          // EN 0x7ab110 (JP 0x1015f4b0): reads the clause attributes

// Starts text input into `buffer` (name entry). EN 0x7ad410 (JP 0x1015fda0). TextInput_End finishes it.
extern "C" int TextInput_Begin(char *buffer);
// Ends text input (src/ime.cpp). EN 0x7ab1e0.
extern "C" int TextInput_End(void);
// 1 while text input is active (src/ime.cpp). EN 0x7ad4a0.
extern "C" int Ime_IsOpen(void);

#endif
