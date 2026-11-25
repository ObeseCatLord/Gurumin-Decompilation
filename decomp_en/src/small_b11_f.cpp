// EN-only tiny input functions without a JP twin (batch 11, section 4).
#include "small_b11.h"

InputPad *g_inputPad;

// Index of the current controller, or -1 when there is none. Called by the launcher's key assignment page.
extern "C" int Input_GetPadIndex(void)
{
    if (g_inputPad != 0)
        return g_inputPad->deviceIndex;
    return INPUT_NO_PAD;
}

// Polls every controller (pad -1) with the two values it is given. Called by the launcher's key assignment page.
extern "C" int Input_UpdateAll(int first, int second)
{
    return Input_Update(INPUT_NO_PAD, second, first, 0);
}
