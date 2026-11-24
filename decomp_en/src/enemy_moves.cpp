// Enemy movement behaviours (EN build). JP twin: enemy_moves.cpp.
#include "anim_model.h"
#include "char.h"
#include "engine.h"
#include <math.h>

// Motion layers, blend numbers and frame counts of the two idle motions.
const int IDLE_MOTION_A = 5;
const int IDLE_MOTION_B = 6;
const int IDLE_MOTION_BLEND = 5;
const int IDLE_MOTION_A_FRAMES = 146;
const int IDLE_MOTION_B_FRAMES = 93;
// Idle motions to choose between.
const int IDLE_MOTION_CHOICES = 2;
// CharState::values indexes used by the idle behaviour.
const int IDLE_VALUE_ACTIVE = 0;     // set to 1 while the behaviour runs
const int IDLE_VALUE_PHASE = 1;      // 0 = choose a motion, 1 = wait for it to end
const int IDLE_VALUE_FRAMES = 2;     // frames of the chosen motion left

// Enemy_IdleMotion: plays one of two idle motions, chosen at random (motion 5 for 146 frames or motion 6 for 93
// frames), and waits for it to end. Returns 1 when it has ended, else 0. Called every frame by enemy behaviours.
extern "C" int Enemy_IdleMotion(int index)
{
    g_charState[index].values[IDLE_VALUE_ACTIVE] = 1;
    if (g_charState[index].values[IDLE_VALUE_PHASE] == 0)
    {
        g_charState[index].values[IDLE_VALUE_FRAMES] = 0;
        switch (Random() % IDLE_MOTION_CHOICES)
        {
        case 0:
            g_characterModels[index].PlayMotion(IDLE_MOTION_A, 1, IDLE_MOTION_BLEND, 1.0f);
            g_charState[index].values[IDLE_VALUE_FRAMES] = IDLE_MOTION_A_FRAMES;
            g_charState[index].values[IDLE_VALUE_PHASE] = 1;
            return 0;
        case 1:
            g_characterModels[index].PlayMotion(IDLE_MOTION_B, 1, IDLE_MOTION_BLEND, 1.0f);
            g_charState[index].values[IDLE_VALUE_FRAMES] = IDLE_MOTION_B_FRAMES;
            g_charState[index].values[IDLE_VALUE_PHASE] = 1;
            break;
        }
    }
    else if (g_charState[index].values[IDLE_VALUE_PHASE] == 1)
    {
        g_charState[index].values[IDLE_VALUE_FRAMES]--;
        if (g_charState[index].values[IDLE_VALUE_FRAMES] == 0)
        {
            g_charState[index].values[IDLE_VALUE_PHASE] = 0;
            return 1;
        }
    }
    return 0;
}

// Motion layer, blend frames and frame count of the watching motion; chance (per WATCH_END_RANGE) per frame that it ends.
const int WATCH_MOTION = 7;
const int WATCH_MOTION_BLEND = 5;
const int WATCH_MOTION_FRAMES = 150;
const int WATCH_END_CHANCE = 15;
const int WATCH_END_RANGE = 3000;
// CharState::values indexes used by the watching behaviour.
const int WATCH_VALUE_ACTIVE = 0;    // set to 1 while the behaviour runs
const int WATCH_VALUE_PHASE = 1;     // 0 = start the motion, 1 = keep facing the player
const int WATCH_VALUE_FRAMES = 2;    // frames of the watching motion

// Enemy_WatchPlayer: starts motion 7 (unless it already plays or is being blended into) and then keeps the enemy turned
// towards the player until a 15-in-3000 chance per frame ends the behaviour. Returns 1 on the frame it ends, else 0.
// Called every frame by enemy behaviours.
extern "C" int Enemy_WatchPlayer(int index)
{
    Vec4 move;
    Vec4 player;
    Vec4 self;
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    g_charState[index].values[WATCH_VALUE_ACTIVE] = 1;
    move.Fill(0);
    if (g_charState[index].values[WATCH_VALUE_PHASE] == 0)
    {
        Playback *motion = g_characterModels[index].playback;
        g_charState[index].values[WATCH_VALUE_FRAMES] = WATCH_MOTION_FRAMES;
        if (motion->layer != WATCH_MOTION && (motion->blending == 0 || motion->nextLayer != WATCH_MOTION))
            g_characterModels[index].PlayMotion(WATCH_MOTION, 1, WATCH_MOTION_BLEND, 1.0f);
        g_charState[index].values[WATCH_VALUE_PHASE] = 1;
    }
    else if (g_charState[index].values[WATCH_VALUE_PHASE] == 1)
    {
        g_characterModels[index].targetHeading = (float)atan2(player.x - self.x, player.y - self.y) + ANGLE_HALF_TURN;
        if (Random() % WATCH_END_RANGE < WATCH_END_CHANCE)
        {
            g_charState[index].values[WATCH_VALUE_PHASE] = 0;
            return 1;
        }
    }
    return 0;
}
