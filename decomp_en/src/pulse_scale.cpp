// Pulsing character scale (JP twin: pulse_scale.cpp).

#include <stdlib.h>

#include "char.h"
#include "node.h"
#include "vec4.h"

// The pulse step runs through 0..PULSE_STEP_LAST and back; steps above PULSE_STEP_TURN are mirrored.
const int PULSE_STEP_TURN = 3;
const int PULSE_STEP_LAST = 7;
// CharState::values index of the pulse step.
const int CHARSTATE_PULSE_STEP = 1;
// Scale arithmetic: t = |3 - step| / 3; x/y scale 2 - t, z scale t * 0.7 + 0.3.
const float PULSE_STEP_DIVISOR = 3.0f;
const float PULSE_XY_MAX = 2.0f;
const float PULSE_Z_RANGE = 0.7f;
const float PULSE_Z_MIN = 0.3f;

// Advances the pulse step (a 0..7 triangle wave in values[1]) and sets the character's pose node matrix to a scale
// matrix that widens x/y while flattening z. The step advances by two per call. Called by the pulsing character
// behaviours. Returns 0.
extern "C" int Char_PulseScale(int index)
{
    if (g_charState[index].values[CHARSTATE_PULSE_STEP] > PULSE_STEP_TURN)
        g_charState[index].values[CHARSTATE_PULSE_STEP] = PULSE_STEP_LAST - g_charState[index].values[CHARSTATE_PULSE_STEP];
    float t = (float)abs(PULSE_STEP_TURN - ++g_charState[index].values[CHARSTATE_PULSE_STEP]) / PULSE_STEP_DIVISOR;
    if (g_charNodes[index][0] != 0)
    {
        float s = PULSE_XY_MAX - t;
        g_charNodes[index][0]->matrix = ScaleMatrix(s, s, t * PULSE_Z_RANGE + PULSE_Z_MIN);
    }
    g_charState[index].values[CHARSTATE_PULSE_STEP]++;
    return 0;
}
