// Mouse input object (the same class appears as Tracker in app_0005.cpp). It keeps the button and wheel state
// reported by the window (m_0000.. bytes), the cursor position in 640x480 game coordinates (m_0004/m_0008) and the
// window it belongs to (m_001c). m_0038 is an optional cursor callback.

#include <windows.h>
#include <math.h>

#include "app_0001.h"

// Stub that always returns 1 (an unused input hook).
extern "C" int __stdcall FUN_10001000(int, int, int)
{
    return 1;
}

// Init: binds the object to window `p` with cursor callback `q` and user value `r`; clears the position.
// Returns 0 when the window or callback is missing.
int Obj::M_10001010(int p, int q, int r)
{
    if (p == 0)
        return 0;
    if (q == 0)
        return 0;
    m_0038 = q;
    m_003c = r;
    m_001c = p;
    m_0004 = 0;
    m_0008 = 0;
    m_0040 = 0;
    return 1;
}

// 1 when point `pt` lies inside the rectangle (x, y, width, height), edges included.
extern "C" int __stdcall FUN_10001050(const Point *pt, int x, int y, int width, int height)
{
    if (pt->x < x || pt->y < y)
        return 0;
    if (pt->x > x + width || pt->y > y + height)
        return 0;
    return 1;
}

// Copies the requested parts of the mouse state (buttons, wheel, position); null pointers are skipped.
int Obj::GetState(Point *pos, char *a, char *b, char *c, int *d, char *e)
{
    if (a != 0)
        *a = m_0000;
    if (b != 0)
        *b = m_0001;
    if (c != 0)
        *c = m_0002;
    if (d != 0)
        *d = m_0014;
    if (e != 0)
        *e = m_0003;
    if (pos != 0)
    {
        pos->x = m_0004;
        pos->y = m_0008;
    }
    return 1;
}

// Sets the changed flag when the cursor moved since the last call; returns the flag.
int Obj::UpdateChanged()
{
    Point pos;
    GetState(&pos, 0, 0, 0, 0, 0);
    if (pos.x != lastX || pos.y != lastY)
        changed = 1;
    lastX = pos.x;
    lastY = pos.y;
    return changed;
}
typedef void (*CursorCallback)(int x, int y);

// 0x100014b0 (Obj::ReportCursor). When the cursor moved: converts the screen cursor position into 640x480 game coordinates (scaled by the
// client size, so it works in any window size) and passes it to the cursor callback.
int Obj::ReportCursor()
{
    POINT cursor;
    POINT origin;
    RECT client;
    if (changed != 0)
    {
        GetCursorPos(&cursor);
        origin.x = 0;
        origin.y = 0;
        ClientToScreen((HWND)m_001c, &origin);
        GetClientRect((HWND)m_001c, &client);
        if (client.right != 0 && client.bottom != 0)
        {
            cursor.x = (cursor.x - origin.x) * 640 / client.right;
            cursor.y = (cursor.y - origin.y) * 480 / client.bottom;
            if (m_0038 != 0)
                ((CursorCallback)m_0038)(cursor.x, cursor.y);
        }
    }
    return 0;
}
typedef int (*WheelCallback)();

// 0x10001170 (Obj::Update). Reads the mouse: the cursor in 640x480 game coordinates, the left, middle and right
// buttons (0 = up, 2 = just pressed, 1 = held), a double click (a second left press within 230 ms roughly in the
// same direction from the screen centre as the first) and the wheel direction (-1, 0 or 1, from the wheel callback
// m_003c). Each part is also copied to its out pointer when that is not null. Returns 1, or 0 when the window has
// no client area.
int Obj::Update(Point *pos, char *left, char *middle, char *right, int *wheel, char *doubleClick)
{
    POINT cursor;
    POINT origin;
    RECT client;
    GetCursorPos(&cursor);
    origin.x = 0;
    origin.y = 0;
    ClientToScreen((HWND)m_001c, &origin);
    GetClientRect((HWND)m_001c, &client);
    if ((GetAsyncKeyState(VK_LBUTTON) & 0xf000) == 0)
        leftHeld = 0;
    if ((GetAsyncKeyState(VK_MBUTTON) & 0xf000) == 0)
        middleHeld = 0;
    if ((GetAsyncKeyState(VK_RBUTTON) & 0xf000) == 0)
        rightHeld = 0;
    char clicked = 0;
    if (client.right == 0)
        return 0;
    if (client.bottom == 0)
        return 0;
    cursor.x = (cursor.x - origin.x) * 640 / client.right;
    cursor.y = (cursor.y - origin.y) * 480 / client.bottom;
    if ((GetAsyncKeyState(VK_LBUTTON) & 0xf000) != 0 && leftHeld == 0)
    {
        DWORD now = GetTickCount();
        float lastX = (float)(clickX - 320);
        float lastY = (float)(clickY - 240);
        float x = (float)(cursor.x - 320);
        float y = (float)(cursor.y - 240);
        float lastLength = (float)sqrt(lastX * lastX + lastY * lastY);
        float length = (float)sqrt(x * x + y * y);
        lastX = lastX / lastLength;
        lastY = lastY / lastLength;
        x = x / length;
        y = y / length;
        // a press at the centre gives no direction (NaN) and counts as a new click
        if (now >= clickTime + 230 || !(x * lastX + y * lastY > 0.5))
        {
            clickX = cursor.x;
            clickY = cursor.y;
            clickTime = now;
        }
        else
        {
            clicked = 1;
            clickTime = 0;
        }
    }
    if ((GetAsyncKeyState(VK_LBUTTON) & 0xf000) != 0)
    {
        m_0000 = (leftHeld == 0) + 1;
        leftHeld = 1;
    }
    else
        m_0000 = 0;
    if ((GetAsyncKeyState(VK_MBUTTON) & 0xf000) != 0)
    {
        m_0001 = (middleHeld == 0) + 1;
        middleHeld = 1;
    }
    else
        m_0001 = 0;
    if ((GetAsyncKeyState(VK_RBUTTON) & 0xf000) != 0)
    {
        m_0002 = (rightHeld == 0) + 1;
        rightHeld = 1;
    }
    else
        m_0002 = 0;
    if (left != 0)
        *left = m_0000;
    if (middle != 0)
        *middle = m_0001;
    if (right != 0)
        *right = m_0002;
    m_0003 = clicked;
    if (doubleClick != 0)
        *doubleClick = clicked;
    int count = 0;
    m_0014 = 0;
    if (m_003c != 0)
    {
        count = ((WheelCallback)m_003c)();
        if (m_0040 > count)
            m_0014 = -1;
        else if (m_0040 < count)
            m_0014 = 1;
    }
    m_0040 = count;
    if (wheel != 0)
        *wheel = m_0014;
    m_0004 = cursor.x;
    m_0008 = cursor.y;
    if (pos != 0)
    {
        pos->x = cursor.x;
        pos->y = cursor.y;
    }
    return 1;
}
