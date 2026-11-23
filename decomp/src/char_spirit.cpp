// The spirit character (behaviour 0x25 of the per-kind update 0x100f9670): a ghost whose visit lasts a few
// hundred frames. It dims at first, sprinkles effects around itself while haunting, then floats on and finally
// brightens up again. While it moves it throws swarms of effects around its position.

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "effect_manager.h"

#include "char_state.h"

#include "char_tables.h"

#include "last_position.h"

#include "char_vars.h"

#include "char_spirit.h"

#include "char_script.h"

extern "C" int FUN_1013f880(void);   // the game's random number
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const EffectOptions *options);
Matrix4 RotationZ(float angle);   // 0x101b1b40

// 0x1002a740 (Char_SpiritUpdate). One frame of spirit `index`. Sets it up on the first call, runs its motion table
// step and tints it by the timer. While haunting (timer 150 or more) every 15th frame an effect appears at a random
// place around it. Afterwards, unless it is haunting, it drifts towards the motion's target and, once it has moved
// more than a few units for four frames, throws a swarm of effects in a circle around itself. Returns 0.
int Char_SpiritUpdate(int index)
{
    Vec4 start;
    Vec4 move;
    float fade = 0.0f;
    Matrix4 *world = &DAT_10db6ee8[index].matrix;
    start.Copy(world->GetTranslation());
    Vec4 spareA;
    Vec4 spareB;
    Vec4 spareC;
    DAT_10db6ee8[index].mode = 0;
    DAT_10db6ee8[index].m_0294 = 1;
    DAT_10db6ee8[index].motionFrame = 0x1e;
    DAT_10db6ee8[index].M_10161be0(0, 0);
    if (DAT_1020b430[index].spiritReady == 0)
    {
        DAT_1020b430[index].spiritReady = 1;
        DAT_1020b430[index].spiritSize =
            DAT_10db6ee8[index].root->Find("@O_047b", 0) != 0 ? SPIRIT_SMALL_SIZE : SPIRIT_LARGE_SIZE;
        DAT_10db6ee8[index].root->M_10194730(0x41, 0x200);
        DAT_10db6ee8[index].root->M_10194730(0x41, 0x400);
    }
    {
        int nextState;
        int stepA;
        int stepB;
        int stepC;
        // the route step writes its last point into the slot that later holds the fade value
        FUN_10028fa0(index, (int *)&fade, DAT_1020b430[index].spiritStep, &nextState, &start, &stepA, &stepB, &stepC);
        DAT_1020b430[index].spiritStep = nextState;
    }
    {
        float zero = 0.0f;
        DAT_10802bf8[index].step.Fill(zero);
    }
    DAT_10db6ee8[index].scale.Set(1.0f, 1.0f, 1.0f, 0.0f);
    if (DAT_1020b430[index].spiritTimer != 0)
    {
        int remaining = DAT_1020b430[index].spiritTimer - 1;
        DAT_1020b430[index].spiritTimer = remaining;
        DAT_1020b430[index].spiritHaunting = 1;
        fade = (float)remaining / SPIRIT_BRIGHTEN_FRAMES;
        if (remaining > SPIRIT_DIM_END)
        {
            fade = (float)(SPIRIT_VISIT_FRAMES - remaining) / SPIRIT_DIM_FRAMES;
            if (fade < 0.0f)
                fade = 0.0f;
            else if (fade > 1.0f)
                fade = 1.0f;
        }
        else if (fade > 1.0f)
            fade = 1.0f;
        fade = 1.0f - fade * SPIRIT_DIM_STRENGTH;
        DAT_10db6ee8[index].scale.Set(fade, fade, fade, 0.0f);
        if (DAT_1020b430[index].spiritTimer >= SPIRIT_HAUNT_FRAMES)
        {
            if (DAT_1020b430[index].spiritTimer % SPIRIT_SPARK_PERIOD == 0 &&
                DAT_1020b430[index].spiritTimer < SPIRIT_DIM_END)
            {
                Vec4 spark;
                Vec4 offset;
                spark.Copy(world->GetTranslation());
                if (DAT_1020b430[index].spiritSize == SPIRIT_SMALL_SIZE)
                {
                    offset.y = SPIRIT_SMALL_RADIUS;
                    offset.z = (float)(FUN_1013f880() % SPIRIT_SMALL_HEIGHTS);
                }
                else
                {
                    offset.y = SPIRIT_LARGE_RADIUS;
                    offset.z = (float)(FUN_1013f880() % SPIRIT_LARGE_HEIGHTS);
                }
                offset.RotateZ((float)(FUN_1013f880() % SPIRIT_DEGREES) / SPIRIT_HALF_TURN * SPIRIT_PI);
                spark.Copy(spark + offset);
                EffectOptions options;
                options.list = SPIRIT_EFFECT_LIST;
                FUN_1001d8b0(index, SPIRIT_SPARK_EFFECT, spark, -1, -1, 0, 0, 0, 0, &options);
            }
        }
        else
            DAT_1020b430[index].spiritHaunting = 0;
    }
    else
        DAT_1020b430[index].spiritHaunting = 0;
    if (DAT_1020b430[index].wait == 0 && DAT_1020b430[index].spiritHaunting == 0)
    {
        move.Copy(start - world->GetTranslation());
        move.x = 0.0f;
        move.y = 0.0f;
        if (move.z != 0.0f)
            move.Normalized3();
        float speed = SPIRIT_DRIFT_SPEED;
        move.Copy(move * speed);
        speed = (float)(SPIRIT_HAUNT_FRAMES - DAT_1020b430[index].spiritTimer) / SPIRIT_DRIFT_FRAMES;
        move.Copy(move * speed);
        if (move.z < 0.0f)
        {
            speed = SPIRIT_SINK_SCALE;
            move.Copy(move * speed);
        }
        DAT_10db6ee8[index].matrix.m[12] += move.x;
        DAT_10db6ee8[index].matrix.m[13] += move.y;
        DAT_10db6ee8[index].matrix.m[14] += move.z;
        DAT_10802bf8[index].step.Copy(move);
        DAT_1020b430[index].chaseFrames++;
        if ((DAT_10802bf8[index].position - world->GetTranslation()).LengthSquared3() > SPIRIT_MOVED_SQUARED &&
            DAT_1020b430[index].chaseFrames > SPIRIT_SWARM_PERIOD - 1 && DAT_1030b8f8[index] != 0)
        {
            DAT_1020b430[index].chaseFrames = 0;
            int used[8];
            for (int i = 0; i < DAT_1020b430[index].spiritSize / SPIRIT_SWARM_UNIT; i++)
            {
                int slot;
                int j;
                used[i] = 0;
                do
                {
                    slot = FUN_1013f880() % SPIRIT_SWARM_SLOTS;
                    for (j = 0; j < i; j++)
                        if (used[j] == slot)
                            break;
                } while (j != i);
                float angle = (float)slot / SPIRIT_SWARM_HALF * SPIRIT_PI;
                Matrix4 ring = RotationZ(angle);
                Vec4 origin;
                ring.SetTranslation(world->GetTranslation());
                ring.m[14] = 0.0f;
                EffectOptions options;
                options.list = SPIRIT_EFFECT_LIST;
                int effect = DAT_1020b430[index].spiritSize / 100 + SPIRIT_SWARM_EFFECT_BASE;
                DAT_10daaa80.Start(effect, origin, -1, -1, 0, 0, 0, &ring, &options, &DAT_10219770[index]);
            }
        }
    }
    {
        float zero = 0.0f;
        DAT_10db6ee8[index].velocity.Fill(zero);
    }
    DAT_10802bf8[index].position.Copy(world->GetTranslation());
    return 0;
}
