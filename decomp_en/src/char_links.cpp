// Beam effects between linked characters (EN build; JP twin APP.dll 1001bb20, run from the scene frame).
#include <math.h>

#include "char.h"
#include "char_links.h"
#include "effect_manager.h"
#include "effect_options.h"

// Angle added to the heading between the two ends of a link (pi as the game writes it).
const float CHAR_LINK_PI = 3.1415f;

// For each link: a link with a destroyed character is cleared; every CHAR_LINK_PERIOD frames a live link gets effect
// CHAR_LINK_END_EFFECT at both ends, turned from the first character towards the second, and CHAR_LINK_BEAMS beam
// effects between the matching node matrices of both characters. Returns 0. Called by the scene frame.
int Char_DrawLinks()
{
    Vec4 first;
    Vec4 second;
    Vec4 offset;
    EffectOptions options;
    Matrix4 matrix;
    for (int i = 0; i < CHAR_LINK_COUNT; i++)
    {
        CharLink *link = &g_charLinks[i];
        if (link->first == 0)
            continue;
        if (g_charKind[link->first] == CHAR_KIND_DESTROYED)
        {
            link->first = 0;
            link->second = 0;
        }
        else if (link->second != 0)
        {
            if (g_charKind[link->second] == CHAR_KIND_DESTROYED)
            {
                link->first = 0;
                link->second = 0;
            }
            else
            {
                if (g_charLinkTimer[i] == 0)
                {
                    options.flags = EFFECT_OPTION_FOLLOWS_MATRIX;
                    options.colour = CHAR_LINK_EFFECT_COLOUR;
                    options.m_000e = CHAR_LINK_EFFECT_LIST;
                    options.m_0000 = 0;
                    first.Copy(g_characterModels[link->first].matrix.GetTranslation());
                    second.Copy(g_characterModels[link->second].matrix.GetTranslation());
                    float dx = second.x - first.x;
                    float dy = second.y - first.y;
                    float angle = (float)atan2((double)dx, (double)dy) + CHAR_LINK_PI;
                    (first - second).Length2D();
                    matrix = RotationZ(angle);
                    matrix.SetTranslation(first);
                    g_effectManager.Start(CHAR_LINK_END_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options, 0);
                    matrix = RotationZ(angle);
                    matrix.SetTranslation(second);
                    g_effectManager.Start(CHAR_LINK_END_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options, 0);
                    for (int beam = 0; beam < CHAR_LINK_BEAMS; beam++)
                        g_effectManager.Start(CHAR_LINK_BEAM_EFFECT, offset, -1, -1, &g_characterNodeMatrices[link->first][beam],
                                              (int)&g_characterNodeMatrices[link->second][beam], 0, 0, &options, 0);
                    g_charLinkTimer[i] = CHAR_LINK_PERIOD;
                }
                g_charLinkTimer[i]--;
            }
        }
    }
    return 0;
}
