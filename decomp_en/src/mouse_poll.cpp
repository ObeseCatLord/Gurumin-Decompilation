// Polling the Windows mouse (EN build). Ported from the matched JP source decomp/src/app_0001.cpp (Obj::Update); the EN
// version also detects a double click of the right button.
#include <windows.h>
#include <math.h>

#include "mouse.h"

// The mouse position is reported in 640x480 game coordinates; the centre is where a double click's direction is measured from.
const int MOUSE_GAME_WIDTH = 640;
const int MOUSE_GAME_HEIGHT = 480;
const int MOUSE_GAME_CENTRE_X = 320;
const int MOUSE_GAME_CENTRE_Y = 240;
// A second press counts as a double click within this many milliseconds of the first.
const int MOUSE_DOUBLE_CLICK_MS = 230;
// ... and when it lies in roughly the same direction from the screen centre (dot product of the unit directions above this).
const double MOUSE_DOUBLE_CLICK_DIRECTION = 0.5;
// GetAsyncKeyState bits that tell the key is down.
const int MOUSE_KEY_DOWN = 0xf000;
// Wheel callback of the window code: returns the wheel's running count.
typedef int (__cdecl *WheelCallback)(void);
// State values of a button: not pressed, just pressed, held.
const int MOUSE_BUTTON_UP = 0;

// Mouse::Poll: reads the cursor (in 640x480 game coordinates), the left, middle and right buttons (0 = up, 2 = just
// pressed, 1 = held), double clicks of the left and right buttons and the wheel direction (-1, 0 or 1, from the wheel
// callback in callbackUser) into the tracker; each part is also copied to its out pointer when that is not null.
// Returns 1, or 0 when the window has no client area. Called once per frame by the input update.
int Mouse::Poll(MousePoint *outPosition, char *outLeft, char *outMiddle, char *outRight, int *outWheel,
                char *outDoubleClick)
{
    POINT cursor;
    POINT origin;
    RECT client;
    GetCursorPos(&cursor);
    origin.x = 0;
    origin.y = 0;
    ClientToScreen((HWND)window, &origin);
    GetClientRect((HWND)window, &client);
    if ((GetAsyncKeyState(VK_LBUTTON) & MOUSE_KEY_DOWN) == 0)
        leftHeld = 0;
    if ((GetAsyncKeyState(VK_MBUTTON) & MOUSE_KEY_DOWN) == 0)
        middleHeld = 0;
    if ((GetAsyncKeyState(VK_RBUTTON) & MOUSE_KEY_DOWN) == 0)
        rightHeld = 0;
    char rightDoubleClick = 0;
    char leftDoubleClick = 0;
    if (client.right == 0)
        return 0;
    if (client.bottom == 0)
        return 0;
    cursor.x = (cursor.x - origin.x) * MOUSE_GAME_WIDTH / client.right;
    cursor.y = (cursor.y - origin.y) * MOUSE_GAME_HEIGHT / client.bottom;
    if ((GetAsyncKeyState(VK_LBUTTON) & MOUSE_KEY_DOWN) != 0 && leftHeld == 0)
    {
        DWORD now = GetTickCount();
        float lastX = (float)(clickPosition.x - MOUSE_GAME_CENTRE_X);
        float lastY = (float)(clickPosition.y - MOUSE_GAME_CENTRE_Y);
        float x = (float)(cursor.x - MOUSE_GAME_CENTRE_X);
        float y = (float)(cursor.y - MOUSE_GAME_CENTRE_Y);
        float lastLength = (float)sqrt(lastX * lastX + lastY * lastY);
        float length = (float)sqrt(x * x + y * y);
        lastX = lastX / lastLength;
        lastY = lastY / lastLength;
        x = x / length;
        y = y / length;
        // a press at the centre gives no direction (NaN) and counts as a new click
        if (now >= clickTime + MOUSE_DOUBLE_CLICK_MS || !(x * lastX + y * lastY > MOUSE_DOUBLE_CLICK_DIRECTION))
        {
            clickPosition.x = cursor.x;
            clickPosition.y = cursor.y;
            clickTime = now;
        }
        else
        {
            leftDoubleClick = 1;
            clickTime = 0;
        }
    }
    if ((GetAsyncKeyState(VK_RBUTTON) & MOUSE_KEY_DOWN) != 0 && rightHeld == 0)
    {
        DWORD now = GetTickCount();
        float lastX = (float)(clickPosition.x - MOUSE_GAME_CENTRE_X);
        float lastY = (float)(clickPosition.y - MOUSE_GAME_CENTRE_Y);
        float x = (float)(cursor.x - MOUSE_GAME_CENTRE_X);
        float y = (float)(cursor.y - MOUSE_GAME_CENTRE_Y);
        float lastLength = (float)sqrt(lastX * lastX + lastY * lastY);
        float length = (float)sqrt(x * x + y * y);
        lastX = lastX / lastLength;
        lastY = lastY / lastLength;
        x = x / length;
        y = y / length;
        // a press at the centre gives no direction (NaN) and counts as a new click
        if (now >= clickTime + MOUSE_DOUBLE_CLICK_MS || !(x * lastX + y * lastY > MOUSE_DOUBLE_CLICK_DIRECTION))
        {
            clickPosition.x = cursor.x;
            clickPosition.y = cursor.y;
            clickTime = now;
        }
        else
        {
            rightDoubleClick = 1;
            clickTime = 0;
        }
    }
    // Note: in the tracker, byte 0x01 (buttonRight in mouse.h) holds the middle button and byte 0x02 (buttonMiddle) the right one.
    if ((GetAsyncKeyState(VK_LBUTTON) & MOUSE_KEY_DOWN) != 0)
    {
        buttonLeft = (leftHeld == 0) + 1;
        leftHeld = 1;
    }
    else
        buttonLeft = MOUSE_BUTTON_UP;
    if ((GetAsyncKeyState(VK_MBUTTON) & MOUSE_KEY_DOWN) != 0)
    {
        buttonRight = (middleHeld == 0) + 1;
        middleHeld = 1;
    }
    else
        buttonRight = MOUSE_BUTTON_UP;
    if ((GetAsyncKeyState(VK_RBUTTON) & MOUSE_KEY_DOWN) != 0)
    {
        buttonMiddle = (rightHeld == 0) + 1;
        rightHeld = 1;
    }
    else
        buttonMiddle = MOUSE_BUTTON_UP;
    if (outLeft != 0)
        *outLeft = buttonLeft;
    if (outMiddle != 0)
        *outMiddle = buttonRight;
    if (outRight != 0)
        *outRight = buttonMiddle;
    buttonExtra2 = rightDoubleClick;
    buttonExtra1 = leftDoubleClick;
    if (outDoubleClick != 0)
        *outDoubleClick = leftDoubleClick;
    wheel = 0;
    int count = 0;
    if (callbackUser != 0)
    {
        count = ((WheelCallback)callbackUser)();
        if (m_0044 > count)
            wheel = -1;
        else if (m_0044 < count)
            wheel = 1;
    }
    m_0044 = count;
    if (outWheel != 0)
        *outWheel = wheel;
    position.x = cursor.x;
    position.y = cursor.y;
    if (outPosition != 0)
    {
        outPosition->x = cursor.x;
        outPosition->y = cursor.y;
    }
    return 1;
}
