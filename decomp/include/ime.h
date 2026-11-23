// StateArrays, ImeContext, ImeWindowHooks: declared here once (first used by src/ime.cpp).
#ifndef GURUMIN_IME_H
#define GURUMIN_IME_H

#include <windows.h>

typedef void (*ImeCallback)();   // an IME event callback of ImeWindowHooks

struct StateArrays
{
    char pad_0000[0x20];
    int m_0020[8];
    float m_0040[8];
    int M_1015caa0(int value, int index);
    int M_1015cac0(float value, int index);
};
struct ImeContext
{
    HWND window;
    HIMC context;
    int pad_0008;
    CANDIDATELIST *buffer;

    ImeContext();
    int Release();
    int Attach(HWND target);
    int IsOpen();
    int SetOpen(BOOL open);
    int GetConversion(DWORD *conversion, DWORD *sentence);
    int SetConversion(const DWORD *conversion, const DWORD *sentence);
    LONG GetCompositionValue();
    LONG M_1015d400();
    DWORD GetSelection();
    const char *GetCandidate(int index);
    DWORD GetPageStart();
    DWORD GetCount();
    DWORD GetPageSize();
    int GetResultString(char *out);
    int GetCompositionString(char *out);
    DWORD LoadCandidates();
    int GetModeText(char *out);
};
// IME hooks inside the game window object DAT_10f65698 (RenderWindow, window_create.cpp): WindowProc calls these
// callbacks on WM_IME_COMPOSITION and sets the two flags.
struct ImeWindowHooks
{
    void *vtable;
    HWND window;                       // 0x04
    int active;
    HINSTANCE instance;
    int compositionResult;             // 0x10: a result string arrived
    int compositionEnded;              // 0x14: the composition string changed
    ImeCallback onCompositionString;   // 0x18
    ImeCallback onCompositionResult;   // 0x1c
    ImeCallback onCursorPos;           // 0x20
    ImeCallback onAttributes;          // 0x24
};

#endif
