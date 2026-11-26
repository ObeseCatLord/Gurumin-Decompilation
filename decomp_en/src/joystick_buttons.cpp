// Game pad button state as a bit mask (EN build, EN-only: no JP twin found).
#include "joystick_device.h"

// Returns the held pad buttons as a bit mask (bit n set when button n is held); 0 when game pad input is off. Called
// by the input code at EN 0x62cb70.
extern "C" unsigned int Pad_GetButtonMask(void)
{
    unsigned int mask = 0;
    if (g_padEnabled != 0)
    {
        for (int button = 0; button < PAD_BUTTON_COUNT; button++)
        {
            if (g_padButtons[button] != 0)
                mask |= 1u << button;
        }
    }
    return mask;
}
