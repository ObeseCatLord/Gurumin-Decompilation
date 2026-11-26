// A character standing on a link between two characters (0x100460d0, behaviour 0x17 of Char_RunBehaviour 0x100dfcf0):
// the link's beams shock it. It shakes in place, throwing sparks from its spine, then is reset.

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "effect_manager.h"

#include "char_state.h"

#include "char_slot.h"

#include "char_tables.h"

#include "last_position.h"

#include "char_vars.h"

#include "char_links.h"

#define SHOCK_LINK_RANGE_SQUARED 10000.0f   // the character must be within 100 units of the link's line
#define SHOCK_FRAMES 60                     // frames of shaking before the character is reset
#define SHOCK_SPARK_EFFECT 0x868
#define SHOCK_SPARK_PERIOD 5                // frames between two chances of a spark
#define SHOCK_ODDS_PERIOD 120               // a random number below (frames left) * 2 of it lets a shake step or spark happen
#define SHOCK_SPARK_SPREAD 100              // sparks land within this range (centred by SHOCK_SPARK_CENTRE)
#define SHOCK_SPARK_CENTRE 50.0f
#define SHOCK_AXIS_RANGE 6                  // a restarted shake counter is random below this
#define SHOCK_AXIS_SCALE 3.0f               // units of shake per counter step
#define SHOCK_SHAKE_MOTION 2
#define SHOCK_UNSET_LEVEL 100               // shake level when no link was found
#define SHOCK_UNSET_BEHAVIOUR 0x12          // behaviour the character falls back to when no link was found

extern "C" int FUN_10043cc0(int index);                    // Char_ResetState
extern "C" int FUN_1013f880(void);                         // the game's random number
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const EffectOptions *options);

// Moves one shake counter a step: a level below 3 swings it up to a limit, a higher level swings it down. When it
// leaves its range or reaches 0 it restarts at a random value.
__forceinline void StepShakeAxis(int &axis, int level)
{
    if (level < 3)
    {
        axis += level * 2 + 2;
        if (axis > (level * 3 + 3) * 2 || axis == 0)
            axis = FUN_1013f880() % SHOCK_AXIS_RANGE;
    }
    else
    {
        axis += (2 - level) * 2;
        if (axis < (-1 - level) * 6 || axis == 0)
            axis = FUN_1013f880() % SHOCK_AXIS_RANGE;
    }
}

// 0x100460d0 (Npc_ShockByLink). The talk id's tens give the release distance (talk / 10 * 10). In state 0 it looks
// for a link of DAT_1049f0c8 whose line passes within 100 units of it (at its own height); with none it falls back
// to behaviour 0x12. Otherwise it freezes (mode 0, shock motion) and enters state 1. In state 1 a random number
// below (60 - frames) * 2 (of 120) lets the three shake counters step (their levels are the digits of shakeLevel)
// and the model is put on its home position (LastPosition) plus 3 units per counter, as far as the map allows;
// every 5th frame a spark effect appears at its spine (or at its feet). When the link is gone, or the player is
// closer than the release distance and the talk id's last digit is not 0, it counts 60 frames and is reset.
// A character told to react (or woken) records its position as the home position. Returns 0.
int FUN_100460d0(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float distance = (player - self).Length2D();
    if (DAT_102550e0[index] != 0 || DAT_10210b30[index] != 0)
        DAT_10802bf8[index].position.Copy(self);
    if (DAT_1020b430[index].state == 0)
    {
        int i = 0;
        DAT_1020b430[index].linkIndex = -1;
        do
        {
            if (DAT_1049f0c8[i].first != 0)
            {
                Vec4 end1;
                Vec4 end2;
                Vec4 here;
                end1.Copy(DAT_10db6ee8[DAT_1049f0c8[i].first].matrix.GetTranslation());
                end2.Copy(DAT_10db6ee8[DAT_1049f0c8[i].second].matrix.GetTranslation());
                here.Copy(self);
                end1.z = here.z;
                end2.z = here.z;
                if (self.DistanceSquaredToSegment(end1, end2) < SHOCK_LINK_RANGE_SQUARED)
                {
                    DAT_1020b430[index].linkIndex = i;
                    break;
                }
            }
            i++;
        } while (i < CHAR_LINK_COUNT);
        if (DAT_1020b430[index].linkIndex < 0)
        {
            DAT_1020b430[index].linkIndex = 0;
            DAT_1020b430[index].behaviour = SHOCK_UNSET_BEHAVIOUR;
            DAT_1020b430[index].shakeLevel = SHOCK_UNSET_LEVEL;
            return 0;
        }
        DAT_10db6ee8[index].mode = 0;
        DAT_1020b430[index].shockFrames = 0;
        DAT_1020b430[index].releaseDistance = DAT_1020b430[index].talk / 10 * 10;
        DAT_1020b430[index].state = 1;
        DAT_10daaae8[index].m_0141 = 1;
        DAT_10802bf8[index].position.Copy(self);
        return 0;
    }
    DAT_10210b30[index] = 0;
    DAT_10db6ee8[index].PlayMotion(SHOCK_SHAKE_MOTION, 0, 5, 1.0f);
    if (DAT_1020b430[index].shockFrames != 0)
        DAT_1020b430[index].shockFrames++;
    if (FUN_1013f880() % SHOCK_ODDS_PERIOD < (SHOCK_FRAMES - DAT_1020b430[index].shockFrames) * 2)
    {
        StepShakeAxis(DAT_1020b430[index].shakeX, DAT_1020b430[index].shakeLevel / 100);
        StepShakeAxis(DAT_1020b430[index].shakeY, DAT_1020b430[index].shakeLevel / 10 % 10);
        StepShakeAxis(DAT_1020b430[index].shakeZ, DAT_1020b430[index].shakeLevel % 10);
    }
    if (DAT_1020b430[index].shockFrames != 0)
    {
        Vec4 shake;
        Vec4 step;
        shake.x = (float)DAT_1020b430[index].shakeX * SHOCK_AXIS_SCALE;
        shake.y = (float)DAT_1020b430[index].shakeY * SHOCK_AXIS_SCALE;
        step.Copy(self);
        step.Copy(DAT_10802bf8[index].position - step + shake);
        step.z = 0.0f;
        if (DAT_10db6ee8[index].IsClearOfMapLimited(&step) != 0)
        {
            DAT_10db6ee8[index].matrix._41 = shake.x + DAT_10802bf8[index].position.x;
            DAT_10db6ee8[index].matrix._42 = shake.y + DAT_10802bf8[index].position.y;
        }
    }
    else
    {
        DAT_10db6ee8[index].SetPosition((float)DAT_1020b430[index].shakeX * SHOCK_AXIS_SCALE + DAT_10802bf8[index].position.x,
                                        (float)DAT_1020b430[index].shakeY * SHOCK_AXIS_SCALE + DAT_10802bf8[index].position.y,
                                        (float)DAT_1020b430[index].shakeZ * SHOCK_AXIS_SCALE + DAT_10802bf8[index].position.z);
    }
    if (DAT_104c0380 % SHOCK_SPARK_PERIOD == 0)
    {
        if (FUN_1013f880() % SHOCK_ODDS_PERIOD < (SHOCK_FRAMES - DAT_1020b430[index].shockFrames) * 2)
        {
            if (DAT_10db6ee8[index].spineNode != 0)
            {
                Vec4 spark;
                Matrix4 matrix;
                matrix.SetIdentity();
                spark.Copy(DAT_10db6ee8[index].spineNode->worldMatrix.GetTranslation());
                spark.x = (float)(FUN_1013f880() % SHOCK_SPARK_SPREAD) + spark.x - SHOCK_SPARK_CENTRE;
                spark.y = (float)(FUN_1013f880() % SHOCK_SPARK_SPREAD) + spark.y - SHOCK_SPARK_CENTRE;
                spark.z = (float)(FUN_1013f880() % SHOCK_SPARK_SPREAD) + spark.z - SHOCK_SPARK_CENTRE;
                matrix.SetTranslation(spark);
                float zero = 0.0f;
                spark.Fill(zero);
                FUN_1001d8b0(index, SHOCK_SPARK_EFFECT, spark, -1, -1, 0, 0, 0, &matrix, 0);
            }
            else
            {
                Vec4 spark;
                Matrix4 matrix;
                matrix.SetIdentity();
                spark.Copy(self);
                spark.x = (float)(FUN_1013f880() % SHOCK_SPARK_SPREAD) + spark.x - SHOCK_SPARK_CENTRE;
                spark.y = (float)(FUN_1013f880() % SHOCK_SPARK_SPREAD) + spark.y - SHOCK_SPARK_CENTRE;
                spark.z = (float)(FUN_1013f880() % SHOCK_SPARK_SPREAD) + spark.z;
                matrix.SetTranslation(spark);
                float zero = 0.0f;
                spark.Fill(zero);
                FUN_1001d8b0(index, SHOCK_SPARK_EFFECT, spark, -1, -1, 0, 0, 0, &matrix, 0);
            }
        }
    }
    if (DAT_1020b430[index].shockFrames == 0)
    {
        int linkFirst = DAT_1049f0c8[DAT_1020b430[index].linkIndex].first;
        if (linkFirst == 0)
        {
            DAT_10db6ee8[index].mode = 1;
            DAT_10daaae8[index].m_0141 = (char)linkFirst;
            DAT_1020b430[index].shockFrames = 1;
        }
        if (DAT_1049f0c8[DAT_1020b430[index].linkIndex].first != 0 || DAT_1020b430[index].talk % 10 == 0)
        {
            if (DAT_1020b430[index].talk % 10 == 0)
                return 0;
            if (!(distance < (float)DAT_1020b430[index].releaseDistance))
                return 0;
        }
        DAT_10db6ee8[index].mode = 1;
        DAT_10daaae8[index].m_0141 = 0;
    }
    else if (DAT_1020b430[index].shockFrames != SHOCK_FRAMES)
        return 0;
    FUN_10043cc0(index);
    return 0;
}
