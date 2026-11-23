// An NPC that turns to watch the player and greets them (0x100b7360, behaviour 0x29 of Char_RunBehaviour
// 0x100dfcf0): it follows the player with its gaze, fidgets in between and, in the areas where greetings are on,
// waves back with a spark when the player walks up to it while facing it.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

Matrix4 RotationZ(float angle);                  // 0x101b1b40

#include "node.h"

#include "effect_manager.h"

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_state.h"

#include "char_tables.h"
#include "char_vars.h"

#include "game.h"

#include "enemy_queue.h"

#define GREET_TURN_OFFSET 3.1415f
#define GREET_FULL_TURN 6.283f
#define GREET_NOTICE_ANGLE 0.05236f
#define GREET_TURN_STEP 0.349066f
#define GREET_GAZE_RANGE 1500.0f
#define GREET_UNPLACED_X (-20000.0f)
#define GREET_TALK_HOLD 300
#define GREET_TALK_MOTION 2
#define GREET_FIDGET_MOTION_A 5
#define GREET_FIDGET_MOTION_B 6
#define GREET_FIDGET_FRAMES_A 0x92
#define GREET_FIDGET_FRAMES_B 0x5d
#define GREET_FIDGET_CHOICES 3
#define GREET_FACING_ANGLE 0.34907f
#define GREET_WAVE_RANGE 250.0f
#define GREET_AREA_A 0x13
#define GREET_AREA_B 0x30
#define GREET_COSTUME_SPECIAL 3
#define GREET_FIRST_WAVE_MOTION 0x5a
#define GREET_WAVE_VARIANTS 4
#define GREET_LAST_WAVE_MOTION 0x5d
#define GREET_WAVE_SPEED 2.0f
#define GREET_SPARK_EFFECT 0x665
#define GREET_SPARK_SPREAD 0x50
#define GREET_SPARK_HALF_SPREAD 40.0f
#define GREET_WAVE_SOUND 0x337
#define GREET_FINISHED_KIND 0x16

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_10043cc0(int index);                                       // Char_ResetState

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// Absolute value of a float (an inline helper: its argument is rounded to float first).
inline float AbsFloat(float value)
{
    return (float)fabs(value);
}

// 0x100b7360 (Npc_GreetWhenFaced). Does nothing while the character is still unplaced (x below -20000). While the
// player talks to it (DAT_10c6351c) it turns to face the player at once; while the player has it held it only
// resets its state. Otherwise state 0 turns it towards the player in steps of 20 degrees (when the player is more
// than 3 degrees off and within 1500 units) and, when the gaze timer runs out, starts a fidget (state 1 or 2,
// motions 5 and 6, chosen at random). When the player faces it from within 250 units in the areas 0x13 and 0x30
// with greetings on, it plays a random wave motion (0x5a..0x5d) and a spark effect at its spine, with a sound;
// once greetings are off again the event of its talk id is raised and it is reset. Returns 0.
int FUN_100b7360(int index)
{
    Vec4 playerPosition;
    Vec4 selfPosition;
    Vec4 toPlayer;
    playerPosition.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    selfPosition.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    toPlayer.Copy(playerPosition - selfPosition);
    if (DAT_10c6351c == index)
    {
        DAT_10db6ee8[index].targetHeading = (DAT_1020b430[index].talkMotionStarted == 0
                        ? (DAT_10db6ee8[index].PlayMotion(GREET_TALK_MOTION, 0, 5, 1.0f), Atan2(toPlayer.x, toPlayer.y))
                        : (DAT_1020b430[index].talkMotionStarted = 0, Atan2(toPlayer.x, toPlayer.y))) + GREET_TURN_OFFSET;
        DAT_1020b430[index].state = 0;
        DAT_1020b430[index].gazeTimer = GREET_TALK_HOLD;
        return 0;
    }
    if (DAT_10c6351c == index + TALK_TARGET_HELD)
    {
        DAT_1020b430[index].state = 0;
        DAT_1020b430[index].gazeTimer = GREET_TALK_HOLD;
        return 0;
    }
    if (selfPosition.x < GREET_UNPLACED_X)
        return 0;
    int motion = -1;
    switch (DAT_1020b430[index].state)
    {
    case 0:
    {
        motion = GREET_TALK_MOTION;
        float turn = atan2f(toPlayer.x, toPlayer.y) + GREET_TURN_OFFSET;
        turn = turn - DAT_10db6ee8[index].heading;
        if (turn > GREET_TURN_OFFSET)
            turn = turn - GREET_FULL_TURN;
        else if (turn < -GREET_TURN_OFFSET)
            turn = turn + GREET_FULL_TURN;
        if (AbsFloat(turn) > GREET_NOTICE_ANGLE && toPlayer.Length2D() < GREET_GAZE_RANGE)
        {
            if (turn > 0.0f)
            {
                if (turn > GREET_TURN_STEP)
                    DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading + GREET_TURN_STEP;
                else
                    DAT_10db6ee8[index].targetHeading = turn + DAT_10db6ee8[index].heading;
                motion = 0;
            }
            else if (turn < 0.0f)
            {
                if (turn < -GREET_TURN_STEP)
                    DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading - GREET_TURN_STEP;
                else
                    DAT_10db6ee8[index].targetHeading = turn + DAT_10db6ee8[index].heading;
                motion = 0;
            }
        }
        if (--DAT_1020b430[index].gazeTimer < 0)
        {
            switch (FUN_1013f880() % GREET_FIDGET_CHOICES)
            {
            case 0:
                motion = GREET_FIDGET_MOTION_A;
                DAT_1020b430[index].gazeTimer = GREET_FIDGET_FRAMES_A;
                DAT_1020b430[index].state = 2;
                break;
            case 1:
                motion = GREET_FIDGET_MOTION_B;
                DAT_1020b430[index].gazeTimer = GREET_FIDGET_FRAMES_B;
                DAT_1020b430[index].state = 1;
                break;
            }
        }
        break;
    }
    case 1:
        motion = GREET_FIDGET_MOTION_A;
        if (--DAT_1020b430[index].gazeTimer < 0)
        {
            DAT_1020b430[index].state = 0;
            DAT_1020b430[index].gazeTimer = GREET_TALK_HOLD;
        }
        break;
    case 2:
        motion = GREET_FIDGET_MOTION_B;
        if (--DAT_1020b430[index].gazeTimer < 0)
        {
            DAT_1020b430[index].state = 0;
            DAT_1020b430[index].gazeTimer = GREET_TALK_HOLD;
        }
        break;
    }
    float facing = Atan2(selfPosition.x - playerPosition.x, selfPosition.y - playerPosition.y) + GREET_TURN_OFFSET;
    facing = DAT_10db9364 - facing;
    for (; facing > GREET_TURN_OFFSET; facing = facing - GREET_FULL_TURN)
        ;
    for (; facing < -GREET_TURN_OFFSET; facing = facing + GREET_FULL_TURN)
        ;
    int layer = DAT_10db6ee8[index].motion->layer;
    if (AbsFloat(facing) < GREET_FACING_ANGLE && toPlayer.Length2D() < GREET_WAVE_RANGE && DAT_104c00e0 != 0 &&
        (DAT_104c00e8 == GREET_AREA_A || DAT_104c00e8 == GREET_AREA_B))
    {
        if (DAT_1020b430[index].greeted == 0 || DAT_10daabb8 == GREET_COSTUME_SPECIAL)
            DAT_1020b430[index].greetPending = 1;
        if (layer < GREET_FIRST_WAVE_MOTION || layer > GREET_LAST_WAVE_MOTION)
        {
            int variant = FUN_1013f880() % GREET_WAVE_VARIANTS;
            DAT_10db6ee8[index].PlayMotion(variant + GREET_FIRST_WAVE_MOTION, 1, 0, GREET_WAVE_SPEED);
            motion = -1;
            Matrix4 matrix;
            Vec4 self;
            Vec4 player;
            Vec4 spot;
            if (DAT_10db6ee8[index].root->Find("Bip01 Spine", 0) != 0)
            {
                spot.Copy(DAT_10db6ee8[index].root->Find("Bip01 Spine", 0)->worldMatrix.GetTranslation());
                spot.x = ((float)(FUN_1013f880() % GREET_SPARK_SPREAD) - GREET_SPARK_HALF_SPREAD) + spot.x;
                spot.y = ((float)(FUN_1013f880() % GREET_SPARK_SPREAD) - GREET_SPARK_HALF_SPREAD) + spot.y;
                spot.z = ((float)(FUN_1013f880() % GREET_SPARK_SPREAD) - GREET_SPARK_HALF_SPREAD) + spot.z;
            }
            self.Copy(selfPosition);
            player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            matrix = RotationZ((float)atan2(player.x - self.x, player.y - self.y) + GREET_TURN_OFFSET);
            matrix.SetTranslation(spot);
            spot.Fill(0.0f);
            DAT_10daaa80.Start(GREET_SPARK_EFFECT, spot, -1, 0, 0, 0, 0, &matrix, 0, 0);
            FUN_10065b10(index, GREET_WAVE_SOUND, 1, 0);
        }
    }
    if (DAT_1020b430[index].greetPending != 0 && DAT_104c00e0 == 0)
    {
        DAT_10874848.RegisterEvent(DAT_1020b430[index].talk);
        DAT_1020b430[index].greetPending = 0;
        DAT_1030b668[index] = GREET_FINISHED_KIND;
        FUN_10043cc0(index);
    }
    if ((layer < GREET_FIRST_WAVE_MOTION || layer > GREET_LAST_WAVE_MOTION) && motion >= 0)
        DAT_10db6ee8[index].PlayMotion(motion, 0, 5, 1.0f);
    return 0;
}
