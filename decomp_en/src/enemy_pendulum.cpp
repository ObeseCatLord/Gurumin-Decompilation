// The pendulum enemy (EN build): hangs from a pivot above its place and swings on a line about it. JP twin:
// enemy_pendulum.cpp.
#include <math.h>

#include "anim_model.h"
#include "char.h"
#include "char_move.h"
#include "char_shots.h"
#include "collide_query.h"
#include "enemy_pendulum.h"
#include "engine.h"
#include "listbank.h"
#include "node.h"
#include "vec4.h"
#include "visibility.h"

// Fades in (over 30 frames) while dropping from 500 units above its place to 100 above it, then swings on a 400 unit
// line about that pivot, turned with its heading: the speed grows by 2 a frame up to the talk id's units, the model's
// root tilts with the line. When the player comes within 800 units and is in sight (checked every ten frames) it lets
// go (state 3) and its state is reset once its motion has settled. Being told to react skips straight to that unless
// the talk id has hundreds. Returns 0. Called by the enemy updater of the pendulum kind.
extern "C" int Enemy_Pendulum(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    float distance = (player - self).Length2D();
    int visible = 0;
    int frames;
    if (g_charState[index].values[PENDULUM_VALUE_TALK] / PENDULUM_TALK_SCALE != 0)
        g_charTalkPending[index] = 0;
    if (g_charToldToReact[index] != 0)
    {
        g_charToldToReact[index] = 0;
        if (g_charState[index].values[PENDULUM_VALUE_TALK] / PENDULUM_TALK_SCALE == 0)
        {
            g_charState[index].state = PENDULUM_STATE_RELEASED;
            g_charState[index].values[PENDULUM_VALUE_WAIT] = 0;
        }
    }
    if (g_frameCounter % PENDULUM_SIGHT_PERIOD == index % PENDULUM_SIGHT_PERIOD)
    {
        Vec4 from;
        Vec4 to;
        Vec4 hit;
        from.Copy(self);
        from.z += PENDULUM_SIGHT_HEIGHT;
        to.Copy(player);
        to.z += PENDULUM_SIGHT_HEIGHT;
        if (g_mapRootNode == 0 || g_mapRootNode->Collide(0, PENDULUM_SIGHT_MASK, &from, &to, &hit) == 0)
            visible = 1;
    }
    if (g_charState[index].state == PENDULUM_STATE_START)
    {
        g_characterModels[index].PlayMotion(PENDULUM_MOTION_HANG, 0, PENDULUM_MOTION_FRAMES, 1.0f);
        g_charRecords[index].defeated = 0;
        g_shotSlots[index].position.Copy(self);
        g_shotSlots[index].position.z += PENDULUM_PIVOT_HEIGHT;
        g_characterModels[index].SetPosition(self.x, self.y, self.z + PENDULUM_START_HEIGHT);
        g_charState[index].state = PENDULUM_STATE_DROPPING;
        g_characterModels[index].alpha = 0;
        g_characterModels[index].mode = 0;
        return 0;
    }
    if (g_charState[index].state == PENDULUM_STATE_DROPPING)
    {
        g_characterModels[index].PlayMotion(PENDULUM_MOTION_HANG, 0, PENDULUM_MOTION_FRAMES, 1.0f);
        frames = g_charState[index].values[PENDULUM_VALUE_WAIT] + 1;
        g_charState[index].values[PENDULUM_VALUE_FALL] -= PENDULUM_FALL_STEP;
        g_charState[index].values[PENDULUM_VALUE_WAIT] = frames;
        g_characterModels[index].mode = 0;
        if (frames < PENDULUM_FADE_FRAMES)
            g_characterModels[index].alpha = (float)frames / PENDULUM_FADE_FRAMES;
        else if (frames == PENDULUM_FADE_FRAMES)
            g_characterModels[index].alpha = 1.0f;
        if ((g_shotSlots[index].position - self).LengthSquared3() < PENDULUM_ARRIVE_SQUARED)
        {
            g_charState[index].values[PENDULUM_VALUE_SWING] =
                g_charState[index].values[PENDULUM_VALUE_TALK] % PENDULUM_TALK_SCALE;
            g_charState[index].values[PENDULUM_VALUE_FALL] = -(g_charState[index].values[PENDULUM_VALUE_FALL] / 2);
            g_charState[index].state = PENDULUM_STATE_SWINGING;
            g_characterModels[index].mode = 0;
            return 0;
        }
        Character_SetMove(index, 0, 0, (float)g_charState[index].values[PENDULUM_VALUE_FALL]);
        return 0;
    }
    if (g_charState[index].state == PENDULUM_STATE_SWINGING)
    {
        g_characterModels[index].PlayMotion(PENDULUM_MOTION_HANG, 0, PENDULUM_MOTION_FRAMES, 1.0f);
        frames = g_charState[index].values[PENDULUM_VALUE_WAIT] + 1;
        g_charState[index].values[PENDULUM_VALUE_WAIT] = frames;
        if (frames < PENDULUM_FADE_FRAMES)
            g_characterModels[index].alpha = (float)frames / PENDULUM_FADE_FRAMES;
        else if (frames == PENDULUM_FADE_FRAMES)
            g_characterModels[index].alpha = 1.0f;
        g_charState[index].values[PENDULUM_VALUE_ANGLE] += g_charState[index].values[PENDULUM_VALUE_SWING];
        if (g_charState[index].values[PENDULUM_VALUE_BACK] == 0)
        {
            g_charState[index].values[PENDULUM_VALUE_SWING] -= PENDULUM_SWING_STEP;
            if (g_charState[index].values[PENDULUM_VALUE_SWING] <
                -(g_charState[index].values[PENDULUM_VALUE_TALK] % PENDULUM_TALK_SCALE))
                g_charState[index].values[PENDULUM_VALUE_SWING] =
                    -(g_charState[index].values[PENDULUM_VALUE_TALK] % PENDULUM_TALK_SCALE);
        }
        else
        {
            g_charState[index].values[PENDULUM_VALUE_SWING] += PENDULUM_SWING_STEP;
            if (g_charState[index].values[PENDULUM_VALUE_SWING] >
                g_charState[index].values[PENDULUM_VALUE_TALK] % PENDULUM_TALK_SCALE)
                g_charState[index].values[PENDULUM_VALUE_SWING] =
                    g_charState[index].values[PENDULUM_VALUE_TALK] % PENDULUM_TALK_SCALE;
        }
        if (g_charState[index].values[PENDULUM_VALUE_ANGLE] <= 0 && g_charState[index].values[PENDULUM_VALUE_SWING] < 0)
            g_charState[index].values[PENDULUM_VALUE_BACK] = 1;
        else if (g_charState[index].values[PENDULUM_VALUE_ANGLE] > 0 &&
                 g_charState[index].values[PENDULUM_VALUE_SWING] >= 0)
            g_charState[index].values[PENDULUM_VALUE_BACK] = 0;
        g_characterModels[index].root->matrix =
            RotationY((float)g_charState[index].values[PENDULUM_VALUE_ANGLE] * -PENDULUM_HALF_TURN / PENDULUM_ANGLE_SCALE);
        Vec4 axis;
        Vec4 line;
        axis.y = -1.0f;
        axis.RotateZ(-g_characterModels[index].heading);
        line.z = -PENDULUM_LINE_LENGTH;
        line.RotateAxis(axis, (float)g_charState[index].values[PENDULUM_VALUE_ANGLE] * PENDULUM_HALF_TURN / PENDULUM_ANGLE_SCALE);
        axis.Copy(g_shotSlots[index].position + line);
        g_characterModels[index].SetPosition(axis.x, axis.y, axis.z + PENDULUM_LINE_LENGTH);
        if (distance < PENDULUM_LET_GO_RANGE && visible != 0 && g_gameBuiltinsRegistered != 0)
        {
            g_charState[index].state = PENDULUM_STATE_RELEASED;
            g_charState[index].values[PENDULUM_VALUE_WAIT] = 0;
            g_characterModels[index].alpha = 1.0f;
            g_characterModels[index].mode = 1;
            g_characterModels[index].idleMotionEnabled = 0;
            return 0;
        }
    }
    else if (g_charState[index].state == PENDULUM_STATE_RELEASED)
    {
        g_characterModels[index].root->matrix.SetIdentity();
        g_characterModels[index].PlayMotion(PENDULUM_MOTION_RELEASED, 0, PENDULUM_MOTION_FRAMES, 1.0f);
        g_charState[index].values[PENDULUM_VALUE_WAIT]++;
        g_characterModels[index].mode = 1;
        if (g_characterModels[index].idleMotionEnabled != 0)
            Char_ResetState(index);
    }
    return 0;
}
