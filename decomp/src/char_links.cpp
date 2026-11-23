// Beam effects between linked characters (run from the scene frame 0x100f5a80).

#include <math.h>
#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "effect_manager.h"

#include "char_tables.h"

#include "char_links.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40

#define PI_APPROX 3.1415f         // pi as the game writes it

// 0x1001bb20 (Char_DrawLinks). For each link of DAT_1049f0c8: a link with a destroyed character is cleared; every
// CHAR_LINK_PERIOD frames a live link gets effect 0x778 at both ends, turned from the first character towards the
// second, and three beams 0x77a between the matching ring points (DAT_10868838) of both characters. Returns 0.
int Char_DrawLinks()
{
    Vec4 first;
    Vec4 second;
    Vec4 offset;
    EffectOptions options;
    Matrix4 matrix;
    for (int i = 0; i < CHAR_LINK_COUNT; i++)
    {
        CharLink *link = &DAT_1049f0c8[i];
        if (link->first == 0)
            continue;
        if (DAT_1021f3c0[link->first] == SLOT_KIND_DESTROYED)
        {
            link->first = 0;
            link->second = 0;
        }
        else if (link->second != 0)
        {
            if (DAT_1021f3c0[link->second] == SLOT_KIND_DESTROYED)
            {
                link->first = 0;
                link->second = 0;
            }
            else
            {
                if (DAT_1021b928[i] == 0)
                {
                    options.flags = 1;
                    options.m_0004 = CHAR_LINK_EFFECT_VALUE;
                    options.m_000e = CHAR_LINK_EFFECT_LIST;
                    options.m_0000 = 0;
                    first.Copy(DAT_10db6ee8[link->first].matrix.GetTranslation());
                    second.Copy(DAT_10db6ee8[link->second].matrix.GetTranslation());
                    float dy = second.y - first.y;
                    float dx = second.x - first.x;
                    float angle = (float)atan2((double)dx, (double)dy) + PI_APPROX;
                    (first - second).Length2D();  // computed and dropped
                    matrix = RotationZ(angle);
                    matrix.SetTranslation(first);
                    DAT_10daaa80.Start(CHAR_LINK_END_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options, 0);
                    matrix = RotationZ(angle);
                    matrix.SetTranslation(second);
                    DAT_10daaa80.Start(CHAR_LINK_END_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options, 0);
                    // a beam effect's value is the matrix its far end follows
                    for (int beam = 0; beam < CHAR_LINK_BEAMS; beam++)
                        DAT_10daaa80.Start(CHAR_LINK_BEAM_EFFECT, offset, -1, -1, &DAT_10868838[link->first][beam],
                                           (int)&DAT_10868838[link->second][beam], 0, 0, &options, 0);
                    DAT_1021b928[i] = CHAR_LINK_PERIOD;
                }
                DAT_1021b928[i]--;
            }
        }
    }
    return 0;
}
