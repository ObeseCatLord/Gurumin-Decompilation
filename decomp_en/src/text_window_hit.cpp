// Mouse hit test over an array of text windows (EN build, EN-only: no JP twin found).
#include "mouse.h"
#include "text_window.h"

// TextWindows_AnyUnderMouse: 1 when the mouse of one of the first `count` windows is over that window (the window's
// rectangle plus TEXT_WINDOW_HIT_MARGIN on its width and height). Windows that are not active, or have no state
// flag set, are skipped. No direct caller recovered.
extern "C" int TextWindows_AnyUnderMouse(TextWindow *windows, int count)
{
    for (int i = 0; i < count; i++)
    {
        TextWindow *window = &windows[i];
        if ((window->m_003c || window->m_0038 || window->m_0040 || window->m_0034) && window->m_003c && window->enabled)
        {
            MousePoint cursor;
            window->mouse->GetState(&cursor, 0, 0, 0, 0, 0, 0);
            if (window->mouse->PointInRect(&cursor, window->left, window->top, window->width + TEXT_WINDOW_HIT_MARGIN,
                                           window->height + TEXT_WINDOW_HIT_MARGIN))
                return 1;
        }
    }
    return 0;
}
