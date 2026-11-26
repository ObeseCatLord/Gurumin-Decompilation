// Point-in-rectangle tests for the mouse (EN build, EN-only: no JP twin found).
#include "mouse.h"

// Mouse_PointInRect: 1 when the point is inside the rectangle (left, top, width, height), borders included. Called by the
// mouse state reader (EN 0x7b04e0, 0x7b0440) and by the cursor tests at EN 0x5de050 and 0x6315e0.
extern "C" int __stdcall Mouse_PointInRect(const MousePoint *point, int left, int top, int width, int height)
{
    if (point->x >= left && point->y >= top && point->x <= left + width && point->y <= top + height)
        return 1;
    return 0;
}

// TextWindow_PointInRect: the text window code's own copy of Mouse_PointInRect (same test). No direct caller.
extern "C" int __stdcall TextWindow_PointInRect(const MousePoint *point, int left, int top, int width, int height)
{
    if (point->x >= left && point->y >= top && point->x <= left + width && point->y <= top + height)
        return 1;
    return 0;
}
