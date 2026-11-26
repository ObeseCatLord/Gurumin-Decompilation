// Fog values the renderer reads (EN build): the fog colour and the vertex shader constants of the fog.
// This is the one declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_FOG_STATE_H
#define GURUMIN_EN_FOG_STATE_H

// The four floats uploaded as one shader constant: where the fog starts, how fast it grows and its density.
struct FogShaderConstants
{
    float start;          // 0x00: distance the fog starts at
    float inverseRange;   // 0x04: 1 / (end distance - start distance)
    float zero;           // 0x08: always 0
    float density;        // 0x0c: fog density of the scene
};

// Fog colour red, green and blue (0..1). EN 0x021ffd48.
extern "C" float g_fogColour[3];
// The fog shader constants. EN 0x02200f68.
extern "C" FogShaderConstants g_fogConstants;

// Sets the fog colour and the shader constants for a fog of density `density` that starts at distance `start` and is
// complete at distance `end`. Returns 1. EN 0x76b270 (fog_state.cpp). Called by the scene script (EN 0x631890).
extern "C" int Fog_Set(float red, float green, float blue, float density, float start, float end);

#endif
