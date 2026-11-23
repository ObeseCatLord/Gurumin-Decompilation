// Enemy_IsBusy: the per-frame check every enemy AI (enemy_ai.cpp, enemy_react.cpp) makes before it moves. An enemy
// is busy while it is being talked to, held, waiting in the enemy queue (enemy_queue.h) or fading out after it left.

#include <math.h>
#include <string.h>
#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "char_state.h"

#include "char_tables.h"

#include "char_vars.h"

#include "game_vars.h"

#include "last_position.h"

#include "enemy_queue.h"

#define PI_APPROX 3.1415f                // pi as the game writes it
#define PARKED_LIMIT_X -50000.0f         // a character at or beyond this x is parked off the map
#define LAST_POSITION_UNSET_X -20000.0f  // LastPosition::position.x below this = not recorded yet

// 0x100e1c40. Returns 1 when the enemy's AI must skip this frame:
// - the enemy being talked to (DAT_10c6351c) stands still in motion 2 (or ends its talk) and turns to the player;
// - an enemy held with DAT_10c6351c = index + TALK_TARGET_HELD does nothing;
// - an enemy in the queue moves up one place when the place ahead is free, else follows the enemy ahead; the front
//   one counts DAT_10c66cd4 down and then starts to fade out;
// - a parked enemy (x <= PARKED_LIMIT_X).
// Returns 0 otherwise, after recording the enemy's position. An enemy that is fading out (CharState::fadeOut) and
// back at its last position fades for QUEUE_FADE_FRAMES frames and is then parked.
int Enemy_IsBusy(int index)
{
    if (DAT_10c62ed0 == QUEUE_FREE_MAP)
        DAT_10db6ee8[index].mode = 0;
    Vec4 toPlayer;
    toPlayer.Copy(DAT_10db6ee8[0].matrix.GetTranslation() - DAT_10db6ee8[index].matrix.GetTranslation());
    int talking = DAT_10c6351c;
    if (talking == index)
    {
        // being talked to: stand (or end the talk) and face the player
        if (DAT_1020b430[index].wait == 0)
        {
            DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
        }
        else
        {
            DAT_1020b430[index].talk = 0;
            DAT_1020b430[index].wait = 0;
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)toPlayer.x, (double)toPlayer.y) + PI_APPROX;
        }
        return 1;
    }
    if (talking == index + TALK_TARGET_HELD)
        return 1;

    // the queue: move up when the place ahead is free, else follow the enemy there
    if (DAT_10c66cf4 == index)
    {
        FUN_100e0320(index, 0);
        if (DAT_10c66cd4 > 0 && --DAT_10c66cd4 == 0)
            DAT_1020b430[index].fadeOut = 1;
        return 1;
    }
    if (DAT_10c66cf0 == index)
    {
        if (DAT_10c66cf4 > 0)
            FUN_100e0320(index, DAT_10c66cf4);
        else
        {
            DAT_10c66cf4 = index;
            DAT_10c66cf0 = 0;
        }
        return 1;
    }
    if (DAT_10c66cec == index)
    {
        if (DAT_10c66cf0 > 0)
            FUN_100e0320(index, DAT_10c66cf0);
        else
        {
            DAT_10c66cf0 = index;
            DAT_10c66cec = 0;
        }
        return 1;
    }
    if (DAT_10c66ce8 == index)
    {
        if (DAT_10c66cec > 0)
            FUN_100e0320(index, DAT_10c66cec);
        else
        {
            DAT_10c66cec = index;
            DAT_10c66ce8 = 0;
        }
        return 1;
    }

    // the leaving enemy, once out of the queue, starts over and fades out
    if (DAT_10c66cd8 == index && DAT_10c66cf4 != DAT_10c66cd8 && DAT_10c66cf0 != DAT_10c66cd8 &&
        DAT_10c66cec != DAT_10c66cd8 && DAT_10c66ce8 != DAT_10c66cd8)
    {
        memset(&DAT_1020b430[index], 0, sizeof(CharState));
        DAT_10c66cd8 = 0;
        DAT_1020b430[index].fadeOut = 1;
    }
    if (DAT_10db6ee8[index].matrix._41 <= PARKED_LIMIT_X)
        return 1;

    if (talking != 0 && DAT_1020b430[index].sawPlayer != 0 &&
        !(DAT_10802bf8[index].position.x < LAST_POSITION_UNSET_X) &&
        !((DAT_10802bf8[index].step - DAT_10db6ee8[index].matrix.GetTranslation()).Length2D() > QUEUE_LEAVE_DISTANCE))
    {
        DAT_10802bf8[index].step.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        if (DAT_1020b430[index].fadeOut == 0)
            return 0;
        DAT_10db6ee8[index].root->M_10194630(index, QUEUE_ROOT_KIND_MASK);
        DAT_10db6ee8[index].root->M_101946b0(0xfe, QUEUE_ROOT_KIND_MASK);
        DAT_10db6ee8[index].root->M_10194730(0x80, QUEUE_ROOT_KIND_MASK);
        DAT_10db6ee8[index].root->M_10194730(0x80, QUEUE_ROOT_KIND_MASK2);
        int frame = ++DAT_1020b430[index].fadeOut;
        DAT_10db6ee8[index].alpha = (float)(QUEUE_FADE_FRAMES - frame) / (float)QUEUE_FADE_FRAMES;
        if (frame == QUEUE_FADE_FRAMES)
        {
            // gone: park it off the map
            DAT_10db6ee8[index].alpha = 1.0f;
            DAT_10db6ee8[index].SetPosition(QUEUE_PARKED_X, 0, 0);
            DAT_1020b430[index].state = 0;
            DAT_10c66cd8 = 0;
        }
        if (DAT_104bf888 == index + QUEUE_KEEP_TARGET)
        {
            DAT_104bf888 = 0;
            DAT_104bfd80 = 0;
        }
        return 0;
    }
    DAT_10802bf8[index].position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    DAT_10802bf8[index].step.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    DAT_1020b430[index].sawPlayer = 1;
    if (talking == 0)
        DAT_10c6351c = TALK_TARGET_NONE;
    return 0;
}
