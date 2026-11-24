// Fading nodes (JP twin: node_fade.cpp).

#include <math.h>
#include <windows.h>

#include "node_fade.h"

// The owner's brightness (Node::brightness) maps 5 or less to fully visible and 20 or more to invisible.
const int FADE_BRIGHTNESS_OFFSET = 5;
const int FADE_LEVEL_MAX = 15;
// A node's parent glows within this squared distance (200 units) of the viewer, falling to 0 at its edge.
const float GLOW_RANGE_SQUARED = 40000.0f;

// For each fading node: the alpha (tint.w) follows the owner's brightness (brightness 5 or less = 1, 20 or more = 0;
// no owner = 1), and the parent's glow (matrix translation z) is the square root of 40000 minus the squared distance
// to the viewer (0 from 200 units on). Called every frame. Returns 0.
extern "C" int FadeNodes_Update(void)
{
    Vec4 viewer = g_viewerMatrix.GetTranslation();
    for (int i = 0; i < g_fadeNodeCount; i++)
    {
        if (g_fadeNodes[i] == 0)
            continue;
        if (g_fadeNodeOwners[i] != 0)
        {
            int level = max(min(g_fadeNodeOwners[i]->brightness - FADE_BRIGHTNESS_OFFSET, FADE_LEVEL_MAX), 0);
            g_fadeNodes[i]->tint.w = (float)(FADE_LEVEL_MAX - level) / (float)FADE_LEVEL_MAX;
        }
        else
            g_fadeNodes[i]->tint.w = 1.0f;
        float distance2 = (viewer - g_fadeNodes[i]->matrix.GetTranslation()).LengthSquared3();
        float clamped = max(GLOW_RANGE_SQUARED - distance2, 0.0f);
        g_fadeNodes[i]->parent->matrix.m[MATRIX_TRANSLATION_Z] = (float)sqrt(clamped);
    }
    return 0;
}
