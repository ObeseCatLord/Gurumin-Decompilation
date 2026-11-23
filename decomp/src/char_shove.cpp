// Scripted shoves: every character slot has four entries in its CharState block (DAT_1020b430) that slide a
// character by a fixed step each frame, turning it and playing a walk or run motion while it moves. A negative
// target makes the player look at a character instead.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "char_state.h"

extern "C" Vec4 DAT_10802ac8;     // where the player's head looks
extern "C" int FUN_100165e0(int mode);   // starts (1) or ends head tracking

// 0x1002ee70 (Char_UpdateShoves). Runs the four shoves of slot `slot`: a look target points the player's head 75
// units above character stepX; a running shove moves its character by (stepX, stepY), turns it along the step and
// plays the run motion (walk below a step of 8), and on its last frame plays the idle motion. Returns 0.
extern "C" int FUN_1002ee70(int slot)
{
    Shove *shove = DAT_1020b430[slot].shoves;
    for (int n = 4; n != 0; n--, shove++)
    {
        if (shove->target < 0)
        {
            DAT_10802ac8.Copy(DAT_10db6ee8[shove->stepX].matrix.GetTranslation());
            DAT_10802ac8.z += 75.0f;
            FUN_100165e0(1);
        }
        else
        {
            int who = shove->target % 10000;
            if (who > -1 && shove->frames % 10000 > 0)
            {
                int frames = --shove->frames;
                DAT_10db6ee8[who].matrix.m[12] += (float)shove->stepX;
                DAT_10db6ee8[who].matrix.m[13] += (float)shove->stepY;
                if (frames % 10000 == 0)
                {
                    if (frames / 10000 == 0)
                        DAT_10db6ee8[who].PlayMotion(2, 1, 5, 1.0f);
                    shove->frames = 0;
                }
                else
                {
                    Vec4 step;
                    step.x = (float)shove->stepX;
                    step.y = (float)shove->stepY;
                    if (shove->target / 10000 == 0)
                        DAT_10db6ee8[shove->target % 10000].targetHeading = atan2f(step.x, step.y) + 3.1415f;
                    if (shove->frames / 10000 == 0)
                    {
                        if (step.Length2D() >= 8.0f)
                            DAT_10db6ee8[shove->target % 10000].PlayMotion(1, 0, 5, 1.0f);
                        else
                            DAT_10db6ee8[shove->target % 10000].PlayMotion(0, 0, 5, 1.0f);
                    }
                }
            }
        }
    }
    return 0;
}
