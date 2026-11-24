// IME globals and context of the EN build (declared once here; JP equivalents in decomp/include/ime.h, ui_vars.h).
#ifndef GURUMIN_EN_IME_H
#define GURUMIN_EN_IME_H

#include <windows.h>
#include <imm.h>

// Wrapper of the Imm32 input context used by the name entry screen. Only the members EN code has used so far.
struct ImeContext
{
    HWND window;              // 0x00: window the context was taken from
    HIMC context;             // 0x04: the input context (Imm32 handle)
    int m_0008;               // 0x08: not yet named
    CANDIDATELIST *buffer;    // 0x0c: candidate list loaded by LoadCandidates (0 = none)

    int Release();                                                      // EN 0x7aaf70 (JP 0x1015d0a0)
    int Attach(HWND target);                                            // EN 0x7aaff0 (JP 0x1015d0e0)
    int SetOpen(BOOL open);                                             // EN 0x7aafb0 (JP 0x1015d120)
    int GetConversion(DWORD *conversion, DWORD *sentence);              // EN 0x7aad50 (JP 0x1015d140)
    int SetConversion(const DWORD *conversion, const DWORD *sentence);  // EN 0x7aafd0 (JP 0x1015d160)
    DWORD LoadCandidates();                                             // EN 0x7aac30 (JP 0x1015d430)
};

extern "C" int g_textInputActive;   // EN 0x02228074 (JP DAT_10f67000): 1 while text input (IME composition) is active
extern "C" int g_textInputFlag;     // EN 0x021e4834: second text input flag, cleared when input ends

#endif
