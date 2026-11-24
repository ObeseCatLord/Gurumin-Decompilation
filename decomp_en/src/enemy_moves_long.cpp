// Enemy turning behaviour (EN build). JP twin: enemy_moves.cpp (Enemy_TurnToPlayer).
#include <math.h>

#include "anim_model.h"
#include "engine.h"
#include "enemy_moves_long.h"
#include "node.h"

// Turns the enemy towards the player, at most 0.349 radians (20 degrees) per frame, playing walk-in-place motion 0
// while it turns and idle motion 2 otherwise. Facing the player (within 0.05236 radians) and closer than 1500 units it
// keeps motion 0 for the remaining turn frames. The first call sets up the model's node flags. The behaviour ends with a
// chance of `endChance` in 400 per frame; returns 1 then, else 0.
extern "C" int Enemy_TurnToPlayer(int index, int endChance)
{
    Vec4 player;
    Vec4 self;
    Vec4 toPlayer;
    g_charState[index].values[TURN_VALUE_ACTIVE] = 0;
    if (g_charState[index].behaviour == 0)
    {
        g_charState[index].behaviour = 1;
        g_characterModels[index].root->SetColourC((char)index, TURN_NODE_MASK_A);
        g_characterModels[index].root->SetColourA(TURN_COLOUR_A, TURN_NODE_MASK_A);
        g_characterModels[index].root->SetColourB(TURN_COLOUR_B, TURN_NODE_MASK_A);
        g_characterModels[index].root->SetColourB(TURN_COLOUR_B, TURN_NODE_MASK_B);
        g_enemyTurnFlag[index] = 0;
    }
    player.Copy(g_characterModels[0].matrix.GetTranslation());
    self.Copy(g_characterModels[index].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    if (g_charState[index].values[TURN_VALUE_LOCKED] == 0)
    {
        float turn = (float)atan2(toPlayer.x, toPlayer.y);
        turn = turn + TURN_HALF;
        turn = turn - g_characterModels[index].heading;
        if (turn > TURN_HALF)
            turn = -(TURN_FULL - turn);
        else if (turn < -TURN_HALF)
            turn = turn + TURN_FULL;
        if ((float)fabs(turn) < TURN_FACING_ANGLE && toPlayer.Length2D() < TURN_ATTENTION_RANGE)
        {
            if (g_charState[index].values[TURN_VALUE_COOLDOWN] == 0)
                g_charState[index].values[TURN_VALUE_COOLDOWN] = TURN_COOLDOWN_FRAMES;
            g_characterModels[index].targetHeading = g_characterModels[index].heading;
            if (g_charState[index].values[TURN_VALUE_FRAMES] > 0)
            {
                g_characterModels[index].PlayMotion(TURN_MOTION_WALK, 0, TURN_MOTION_BLEND, 1.0f);
                g_charState[index].values[TURN_VALUE_FRAMES]--;
            }
            else
            {
                g_characterModels[index].PlayMotion(TURN_MOTION_IDLE, 0, TURN_MOTION_BLEND, 1.0f);
            }
        }
        else if (((float)fabs(turn) < TURN_AHEAD_ANGLE && toPlayer.Length2D() < TURN_ATTENTION_RANGE) ||
                 g_charState[index].values[TURN_VALUE_COOLDOWN] > 0)
        {
            if (g_charState[index].values[TURN_VALUE_COOLDOWN] == 0)
                g_charState[index].values[TURN_VALUE_COOLDOWN] = TURN_COOLDOWN_FRAMES;
            g_charState[index].values[TURN_VALUE_FRAMES] = TURN_WALK_FRAMES;
            if (turn > 0.0f)
            {
                if (turn > TURN_STEP)
                    g_characterModels[index].targetHeading = g_characterModels[index].heading + TURN_STEP;
                else
                    g_characterModels[index].targetHeading = turn + g_characterModels[index].heading;
            }
            else if (turn < 0.0f)
            {
                if (turn < -TURN_STEP)
                    g_characterModels[index].targetHeading = g_characterModels[index].heading - TURN_STEP;
                else
                    g_characterModels[index].targetHeading = turn + g_characterModels[index].heading;
            }
            g_characterModels[index].PlayMotion(TURN_MOTION_WALK, 0, TURN_MOTION_BLEND, 1.0f);
        }
        else
        {
            g_characterModels[index].PlayMotion(TURN_MOTION_IDLE, 0, TURN_MOTION_BLEND, 1.0f);
        }
        if (g_charState[index].values[TURN_VALUE_COOLDOWN] > 0)
            g_charState[index].values[TURN_VALUE_COOLDOWN]--;
        if (Random() % TURN_END_RANGE < endChance)
        {
            g_charState[index].values[TURN_VALUE_COOLDOWN] = 0;
            g_charState[index].values[TURN_VALUE_FRAMES] = 0;
            return 1;
        }
    }
    else
    {
        g_characterModels[index].PlayMotion(TURN_MOTION_IDLE, 0, TURN_MOTION_BLEND, 1.0f);
    }
    return 0;
}
