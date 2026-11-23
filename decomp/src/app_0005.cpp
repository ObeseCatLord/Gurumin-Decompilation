// Mouse input object (see app_0001.cpp) and message text.

#include <windows.h>

#include "game_vars.h"

#include "tracker.h"

// Moves the Windows cursor to game position (x, y) in 640x480 coordinates, scaled to the client area.
int Tracker::M_10001090(int x, int y)
{
    POINT origin;
    origin.x = 0;
    origin.y = 0;
    ClientToScreen(window, &origin);
    RECT client;
    GetClientRect(window, &client);
    SetCursorPos(client.right * x / 640 + origin.x, client.bottom * y / 480 + origin.y);
    return 0;
}

// Same as Obj::GetState: copies the requested parts of the mouse state.
int Tracker::M_10001110(int *pos, char *a, char *b, char *c, int *d, char *e)
{
    if (a)
        *a = m_0000;
    if (b)
        *b = m_0001;
    if (c)
        *c = m_0002;
    if (d)
        *d = m_0014;
    if (e)
        *e = m_0003;
    if (pos)
    {
        pos[0] = m_0004;
        pos[1] = m_0008;
    }
    return 1;
}

// Returns the changed flag.
int Tracker::M_10001440()
{
    return m_0018;
}

// Same as Obj::UpdateChanged.
int Tracker::M_10001450()
{
    int pos[2];
    M_10001110(pos, 0, 0, 0, 0, 0);
    if (pos[0] != m_0020 || pos[1] != m_0024)
        m_0018 = 1;
    m_0020 = pos[0];
    m_0024 = pos[1];
    return m_0018;
}

// Clears the changed flag.
int Tracker::M_100014a0()
{
    m_0018 = 0;
    return 0;
}

// Sets the message window body text (DAT_1020e0f8); a non-empty text reopens the window from the start.
// Passing 0 clears the text.
extern "C" int FUN_1000cb90(const char *text)
{
    if (text != 0)
    {
        const char *src = text;
        char c;
        do
        {
            c = *src;
            DAT_1020e0f8[src - text] = c;
            src++;
        } while (c != 0);
        if (*text != 0)
        {
            DAT_104bf8d4 = 1;
            DAT_104bf8d8 = 0;
            return 0;
        }
    }
    else
        DAT_1020e0f8[0] = 0;
    return 0;
}
