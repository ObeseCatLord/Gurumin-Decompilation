// FontSet (EN build). JP twin: input_font.cpp.
#include "engine_memory.h"
#include "input_font.h"
#include "input_rumble.h"

// Frees the font buffers, deletes the three GDI fonts and frees the 32 cached glyph bitmaps. Returns 1. Called when
// the text system shuts down.
int FontSet::Release()
{
    if (buffer != 0)
        g_engineFree(buffer);
    buffer = 0;
    if (buffer2 != 0)
        g_engineFree(buffer2);
    buffer2 = 0;
    if (fonts[0] != 0)
        DeleteObject(fonts[0]);
    if (fonts[1] != 0)
        DeleteObject(fonts[1]);
    if (fonts[2] != 0)
        DeleteObject(fonts[2]);
    for (int i = 0; i < GLYPH_CACHE_COUNT; i++)
    {
        if (glyphs[i] != 0)
            g_engineFree(glyphs[i]);
        glyphs[i] = 0;
    }
    return 1;
}

// Starts the controller vibration for `milliseconds` ms at 1 / `divisor` of the full strength: on an XInput pad both
// motors, remembering when to stop; otherwise on the DirectInput force-feedback effect, if there is one. Called by
// the hit and explosion effects.
extern "C" void Input_Rumble(float divisor, int milliseconds)
{
    if (g_xinputActive != 0)
    {
        XInputVibration vibration;
        vibration.rightMotor = (unsigned short)(XINPUT_MOTOR_MAX / divisor);
        vibration.leftMotor = vibration.rightMotor;
        g_xinputSetState(g_xinputUser, &vibration);
        g_rumbleEndTime = GetTickCount() + milliseconds;
    }
    else if (g_forceFeedbackEffect != 0)
    {
        g_forceFeedbackPeriodic.magnitude = (unsigned long)(FORCE_FEEDBACK_MAGNITUDE_MAX / divisor);
        g_forceFeedbackDesc.duration = milliseconds * MICROSECONDS_PER_MILLISECOND;
        g_forceFeedbackEffect->SetParameters(&g_forceFeedbackDesc, EFFECT_UPDATE_DURATION_AND_TYPE);
        g_forceFeedbackEffect->Start(1, 0);
    }
}
