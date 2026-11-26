// Fog setup (EN build, EN-only: no JP twin found).
#include "fog_state.h"

// Stores the fog colour (`red`, `green`, `blue`) and the shader constants for a fog of density `density` that starts at
// distance `start` and is complete at distance `end`. Returns 1. Called by the scene script (EN 0x631890).
extern "C" int Fog_Set(float red, float green, float blue, float density, float start, float end)
{
    g_fogColour[0] = red;
    g_fogColour[1] = green;
    g_fogColour[2] = blue;
    g_fogConstants.start = start;
    g_fogConstants.zero = 0;
    g_fogConstants.inverseRange = 1.0f / (end - start);
    g_fogConstants.density = density;
    return 1;
}
