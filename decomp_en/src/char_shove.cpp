// Shoves: timed steps of characters in scripted scenes (EN build). Ported from the matched JP source
// decomp/src/char_shove.cpp.
#include <math.h>
#include "char.h"
#include "head_look.h"

// Shoves pack two values into one number: the character (or flag) in the low digits and a flag in the upper ones.
const int SHOVE_PACK = 10000;

// Height above the looked-at character that the head look target is raised by.
const float SHOVE_LOOK_LIFT = 75.0f;

// Half turn added to the step direction so the character faces away from the step.
const float SHOVE_HALF_TURN = 3.1415f;

// Step length from which the character runs instead of walking.
const float SHOVE_RUN_STEP = 8.0f;

// Motion layers and blend values of the shove motions.
const int SHOVE_MOTION_IDLE = 2;
const int SHOVE_MOTION_RUN = 1;
const int SHOVE_MOTION_WALK = 0;
const int SHOVE_MOTION_BLEND = 5;

// Char_UpdateShoves: runs the SHOVE_COUNT shoves of slot `slot`: a look target points the player's head SHOVE_LOOK_LIFT
// units above the character in stepX; a running shove moves its character by (stepX, stepY), turns it along the step and
// plays the run motion (walk below SHOVE_RUN_STEP), and on its last frame plays the idle motion. Returns 0. Called every
// frame for each slot.
extern "C" int Char_UpdateShoves(int slot)
{
    Shove *shove = g_charState[slot].shoves;
    for (int n = SHOVE_COUNT; n != 0; n--, shove++)
    {
        if (shove->target < 0)
        {
            g_headLookTarget.Copy(g_characterModels[shove->stepX].matrix.GetTranslation());
            g_headLookTarget.z += SHOVE_LOOK_LIFT;
            Player_SetHeadTracking(HEAD_TRACK_START);
        }
        else
        {
            int who = shove->target % SHOVE_PACK;
            if (who > -1 && shove->frames % SHOVE_PACK > 0)
            {
                int frames = --shove->frames;
                g_characterModels[who].matrix.m[MATRIX_TRANSLATION_X] += (float)shove->stepX;
                g_characterModels[who].matrix.m[MATRIX_TRANSLATION_Y] += (float)shove->stepY;
                if (frames % SHOVE_PACK == 0)
                {
                    if (frames / SHOVE_PACK == 0)
                        g_characterModels[who].PlayMotion(SHOVE_MOTION_IDLE, 1, SHOVE_MOTION_BLEND, 1.0f);
                    shove->frames = 0;
                }
                else
                {
                    Vec4 step;
                    step.x = (float)shove->stepX;
                    step.y = (float)shove->stepY;
                    if (shove->target / SHOVE_PACK == 0)
                        g_characterModels[shove->target % SHOVE_PACK].targetHeading = atan2f(step.x, step.y) + SHOVE_HALF_TURN;
                    if (shove->frames / SHOVE_PACK == 0)
                    {
                        if (step.Length2D() >= SHOVE_RUN_STEP)
                            g_characterModels[shove->target % SHOVE_PACK].PlayMotion(SHOVE_MOTION_RUN, 0, SHOVE_MOTION_BLEND, 1.0f);
                        else
                            g_characterModels[shove->target % SHOVE_PACK].PlayMotion(SHOVE_MOTION_WALK, 0, SHOVE_MOTION_BLEND, 1.0f);
                    }
                }
            }
        }
    }
    return 0;
}
