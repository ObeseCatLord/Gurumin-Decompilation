// Message pump that runs while Escape or Enter is held down (EN build).
#include <windows.h>

#include "render.h"

// Key state bits that mean "key is down".
const unsigned short KEY_DOWN_MASK = 0xf000;
// Messages handled per pass before the key state is read again.
const int KEY_WAIT_MESSAGES_PER_PASS = 30;

// Keeps dispatching window messages for as long as Escape or Enter is held down, so the key press that closed a screen
// is not seen by the next one (F10 is dropped; WM_QUIT sets the quit flag). Called by the screen close paths.
extern "C" void KeyRelease_PumpWhileHeld()
{
    MSG msg;
    for (;;)
    {
        if ((GetKeyState(VK_ESCAPE) & KEY_DOWN_MASK) == 0 && (GetKeyState(VK_RETURN) & KEY_DOWN_MASK) == 0)
            return;
        int count = 0;
        BOOL pending = PeekMessageA(&msg, 0, 0, 0, PM_REMOVE);
        if (pending == 0)
            continue;
        do
        {
            if (count >= KEY_WAIT_MESSAGES_PER_PASS)
                break;
            count++;
            if (msg.message == WM_QUIT)
            {
                g_quitRequested = 1;
                break;
            }
            if (msg.message != WM_SYSKEYDOWN || msg.wParam != VK_F10)
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
            pending = PeekMessageA(&msg, 0, 0, 0, PM_REMOVE);
        } while (pending != 0);
    }
}
