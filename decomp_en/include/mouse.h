// The mouse tracker of the EN build: polled button state, cursor position and a "moved" flag. Declared once here.
#ifndef GURUMIN_EN_MOUSE_H
#define GURUMIN_EN_MOUSE_H

// Cursor position in client pixels.
struct MousePoint
{
    int x;  // horizontal position
    int y;  // vertical position
};

// Called with the cursor position in 640x480 game coordinates whenever it is reported.
typedef void (__cdecl *MouseCursorCallback)(int x, int y);

struct Mouse
{
    char buttonLeft;          // 0x00: 1 while the left button is down
    char buttonRight;         // 0x01: right button
    char buttonMiddle;        // 0x02: middle button
    char buttonExtra1;        // 0x03: first extra button
    char buttonExtra2;        // 0x04: second extra button
    char m_0005[3];           // 0x05: padding
    MousePoint position;      // 0x08: cursor position
    int m_0010[2];            // 0x10: not recovered
    int wheel;                // 0x18: wheel movement
    int moved;                // 0x1c: 1 once the cursor has moved since the last ClearChanged
    int window;               // 0x20: window handle the mouse is bound to
    MousePoint lastPosition;  // 0x24: position at the previous UpdateMoved
    int m_002c[4];            // 0x2c: not recovered
    MouseCursorCallback cursorCallback;   // 0x3c: cursor callback given to Init (0 = none)
    int callbackUser;         // 0x40: user value passed to the callback
    int m_0044;               // 0x44: cleared by Init

    int ClearChanged();       // EN 0x7aa8b0
    int UpdateMoved();        // EN 0x7aa250
    int GetState(MousePoint *outPosition, char *outLeft, char *outRight, char *outMiddle, int *outWheel,
                 char *outExtra1, char *outExtra2);  // EN 0x7aa330
    int Init(int window, MouseCursorCallback callback, int user);    // EN 0x7aa810
    int ReportCursor();                              // EN 0x7aa2a0 (JP 0x100014b0): sends the cursor position in game coordinates to the callback
    int SetCursorGamePos(int x, int y);              // EN 0x7aa8f0 (JP 0x10001090): moves the Windows cursor to a 640x480 game position
};

// The game's mouse. EN 0x00a1d998 (JP DAT_1020d9e0).
extern "C" Mouse g_mouse;

#endif
