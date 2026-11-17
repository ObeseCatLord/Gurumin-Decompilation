// Mouse input object (the same class appears as Tracker in app_0005.cpp). It keeps the button and wheel state
// reported by the window (m_0000.. bytes), the cursor position in 640x480 game coordinates (m_0004/m_0008) and the
// window it belongs to (m_001c). m_0038 is an optional cursor callback.

#include <windows.h>

struct Point
{
    int x;
    int y;
};

// Stub that always returns 1 (an unused input hook).
extern "C" int __stdcall FUN_10001000(int, int, int)
{
    return 1;
}

struct Obj
{
    char m_0000;
    char m_0001;
    char m_0002;
    char m_0003;
    int m_0004;
    int m_0008;
    int pad_000c[2];
    int m_0014;
    int changed;
    int m_001c;
    int lastX;
    int lastY;
    int pad_0028[4];
    int m_0038;
    int m_003c;
    int m_0040;
    int M_10001010(int p, int q, int r);
    int GetState(Point *pos, char *a, char *b, char *c, int *d, char *e);
    int UpdateChanged();
    int ReportCursor();
};

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

// When the cursor moved: converts the screen cursor position into 640x480 game coordinates (scaled by the
// client size, so it works in any window size) and passes it to the cursor callback. Not byte-identical yet
// (see open.tsv).
int Obj::ReportCursor()
{
    POINT cursor;
    POINT origin;
    RECT client;
    if (changed == 0)
        return 0;
    GetCursorPos(&cursor);
    origin.x = 0;
    origin.y = 0;
    ClientToScreen((HWND)m_001c, &origin);
    GetClientRect((HWND)m_001c, &client);
    if (client.right == 0 || client.bottom == 0)
        return 0;
    cursor.x = (cursor.x - origin.x) * 640 / client.right;
    cursor.y = (cursor.y - origin.y) * 480 / client.bottom;
    if (m_0038 != 0)
        ((CursorCallback)m_0038)(cursor.x, cursor.y);
    return 0;
}