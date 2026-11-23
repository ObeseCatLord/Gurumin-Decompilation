// Fading of map light/effect nodes by distance from the viewer and by the brightness of their owner object.

#include <math.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#include "vec4.h"

#include "matrix4.h"

#include "node.h"



#include "render_vars.h"
#include "ui_vars.h"


// For each fading node: the alpha (tint.w) follows the owner's brightness (Node::m_1899; brightness 5 or less = 1, 20 or more = 0; no
// owner = 1), and the parent's glow is the square root of 40000 minus the squared distance to the viewer (0 from
// 200 units on). Returns 0.
extern "C" int FUN_10017fc0(void)
{
    Vec4 viewer = DAT_1086df38.GetTranslation();
    for (int i = 0; i < DAT_104c03b0; i++)
    {
        if (DAT_1021a1e0[i] == 0)
            continue;
        if (DAT_10254ac0[i] != 0)
        {
            int level = max(min(DAT_10254ac0[i]->m_1899 - 5, 15), 0);
            DAT_1021a1e0[i]->tint.w = (float)(15 - level) / 15.0f;
        }
        else
            DAT_1021a1e0[i]->tint.w = 1.0f;
        float distance2 = (viewer - DAT_1021a1e0[i]->matrix.GetTranslation()).LengthSquared3();
        float rest = 40000.0f - distance2;
        float clamped = 0.0f < rest ? rest : 0.0f;
        DAT_1021a1e0[i]->parent->matrix._43 = (float)sqrt(clamped);
    }
    return 0;
}
