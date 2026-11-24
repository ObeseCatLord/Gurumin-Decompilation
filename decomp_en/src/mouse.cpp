// EN mouse tracker (JP twin: app_0001.cpp / app_0005.cpp).

#include "mouse.h"

// Clears the moved flag. Called once per frame after the game has looked at it.
int Mouse::ClearChanged()
{
    moved = 0;
    return 0;
}

// Sets the moved flag when the cursor moved since the last call; returns the flag.
int Mouse::UpdateMoved()
{
    MousePoint now = position;
    if (now.x != lastPosition.x || now.y != lastPosition.y)
        moved = 1;
    lastPosition.y = now.y;
    lastPosition.x = now.x;
    return moved;
}

// Copies the requested parts of the mouse state (buttons, wheel, position); null pointers are skipped.
int Mouse::GetState(MousePoint *outPosition, char *outLeft, char *outRight, char *outMiddle, int *outWheel,
                    char *outExtra1, char *outExtra2)
{
    if (outLeft != 0)
        *outLeft = buttonLeft;
    if (outRight != 0)
        *outRight = buttonRight;
    if (outMiddle != 0)
        *outMiddle = buttonMiddle;
    if (outWheel != 0)
        *outWheel = wheel;
    if (outExtra1 != 0)
        *outExtra1 = buttonExtra1;
    if (outExtra2 != 0)
        *outExtra2 = buttonExtra2;
    if (outPosition != 0)
    {
        outPosition->x = position.x;
        outPosition->y = position.y;
    }
    return 1;
}

// Binds the tracker to `windowHandle` with cursor callback `callback` and user value `user`; clears the position.
// Returns 0 when the window or callback is missing.
int Mouse::Init(int windowHandle, int callback, int user)
{
    if (windowHandle == 0)
        return 0;
    if (callback == 0)
        return 0;
    cursorCallback = callback;
    callbackUser = user;
    window = windowHandle;
    position.x = 0;
    position.y = 0;
    m_0044 = 0;
    return 1;
}
