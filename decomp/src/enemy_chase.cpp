// An enemy chasing the player (0x10027630, an enemy move like those in enemy_moves.cpp, used by the AI updaters
// 0x100e2110 and 0x100e2560): it runs towards the player with a speed that eases towards its wanted speed.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];

// Per-character AI state (world.h CharState).
struct NpcState
{
    int state;
    int wait;                 // 0x04: frames in the move (cleared here)
    int pause;                // 0x08: 1 = pausing instead of chasing
    int pauseFrames;          // 0x0c
    int running;              // 0x10: 1 = running (faster) after the player was far away
    char pad_0014[0x2c];
};

extern "C" NpcState DAT_1020b430[0x80];
extern "C" float DAT_10255160[0x80][4];       // [0]: the enemy's current speed
extern "C" int DAT_104bfd08;                  // non-zero = enemies do not move
extern "C" int FUN_1013f880(void);            // the game's random number
extern "C" int FUN_10024e80(int index, Vec4 move);           // Char_OutsideLimit
extern "C" int FUN_100241e0(int index, float x, float y, float z);   // Character_SetMove

// 0x10027630 (Enemy_ChasePlayer). Runs towards the player at a speed easing by 0.1 a frame towards 2 (8 once the
// player was more than 300 units away, back to 2 within 200), facing the player (motion 1, or 0 when barely
// moving). Returns 1 to end the move when the way 20 units up is blocked (unless `wander`), the player is more than
// 500 units away, at random with `wander` (1 in 400) or when the step leaves its range; else 0. While pausing it
// only counts down.
extern "C" int FUN_10027630(int index, int wander)
{
    Vec4 step;
    Vec4 player;
    Vec4 self;
    Vec4 toPlayer;
    int blocked = 0;
    DAT_1020b430[index].wait = 0;
    step.Fill(0.0f);
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    float distance = toPlayer.Length2D();
    if (DAT_1020b430[index].pause == 0)
    {
        step.y = -2.0f;
        if (DAT_1020b430[index].running != 0)
            step.y = -8.0f;
        step.RotateZ(-DAT_10db6ee8[index].heading);
        float *speed = &DAT_10255160[index][0];
        if (step.Length() > *speed)
            *speed += 0.1f;
        else if (step.Length() < *speed)
            *speed -= 0.1f;
        step.Normalized3();
        step.Copy(step * *speed);
        DAT_10db6ee8[index].matrix.m[12] += step.x;
        DAT_10db6ee8[index].matrix.m[13] += step.y;
        DAT_10db6ee8[index].matrix.m[14] += 20.0f;
        if (DAT_10db6ee8[index].IsClearAny(0) == 0 || DAT_10db6ee8[index].IsClearOfMapLimited(0) == 0)
            blocked = 1;
        DAT_10db6ee8[index].matrix.m[12] -= step.x;
        DAT_10db6ee8[index].matrix.m[13] -= step.y;
        DAT_10db6ee8[index].matrix.m[14] -= 20.0f;
        if ((blocked != 0 && wander == 0) || distance > 500.0f || (wander != 0 && FUN_1013f880() % 400 == 0) ||
            FUN_10024e80(index, step) != 0)
            return 1;
        DAT_10db6ee8[index].targetHeading = atan2f(toPlayer.x, toPlayer.y) + 3.1415f;
        if (DAT_104bfd08 == 0)
        {
            FUN_100241e0(index, step.x, step.y, 0);
            DAT_10db6ee8[index].move.x *= DAT_10db6ee8[index].speedScale;
            DAT_10db6ee8[index].move.y = DAT_10db6ee8[index].speedScale * DAT_10db6ee8[index].move.y;
        }
        if (distance < 200.0f)
            DAT_1020b430[index].running = 0;
        else if (distance > 300.0f)
            DAT_1020b430[index].running = 1;
        if (step.Length() < 5.0f)
        {
            DAT_10db6ee8[index].PlayMotion(0, 0, 5, 1.0f);
            return 0;
        }
        DAT_10db6ee8[index].PlayMotion(1, 0, 5, 1.0f);
        return 0;
    }
    if (DAT_1020b430[index].pause == 1)
    {
        if (DAT_1020b430[index].pauseFrames > 0)
            DAT_1020b430[index].pauseFrames--;
        if (DAT_1020b430[index].pauseFrames == 0)
            DAT_1020b430[index].pause = 0;
    }
    return 0;
}
