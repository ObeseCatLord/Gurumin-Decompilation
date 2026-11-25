// EN-only tiny functions without a JP twin (batch 11, section 4): window procedure slot and an identity hook.
#include <windows.h>

#include "small_b11.h"

// Slot of the window object's virtual table (EN 0x854fb4): hands the message to the default window procedure and
// ignores the object. Called by the window dispatch.
LRESULT __stdcall WindowObject_DefaultProc(void *self, HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProcA(window, message, wParam, lParam);
}

// Returns its argument unchanged. Nothing calls it in the exe; it is a leftover hook of the sound code.
extern "C" int Hook_ReturnArgument(int value)
{
    return value;
}
