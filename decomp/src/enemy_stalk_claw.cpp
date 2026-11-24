// The stalking enemy that claws at the player (0x100aa820, the normal behaviour of the enemies run from 0x100bcb30
// and from the shaga seeker 0x100d2110). It follows the stalk / circle / retreat steps of the strike enemy
// (enemy_stalk_strike.cpp) but its attacks are four claw swings (steps 3 to 6) with streaks and rings drawn from its
// finger nodes.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "effect_manager.h"

#include "anim_model.h"

typedef char check_charrecord[sizeof(AnimModel) == 0x249c ? 1 : -1];

#include "char_state.h"

#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "last_position.h"
#include "map_vars.h"
#include "world.h"

#include "claw_enemy.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40
Matrix4 RotationX(float angle);   // 0x101b1ad0

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_100241e0(int index, float x, float y, float z);            // Character_SetMove
extern "C" int FUN_1001a450(int slot, const Vec4 *point);                     // Slot_HasNoGround
extern "C" int FUN_1001a590(int slot, const Vec4 *unused, float distance);    // ground probe along the slot's angle
extern "C" int FUN_1001bfb0(int index, float maxTurn);                        // Char_LimitTurn
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            const Matrix4 *target, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

#define CLAW_NULL_NODE "@efc_Null"
#define CLAW_LEFT_HAND_NODE "@Bip01 L Finger0"
#define CLAW_RIGHT_HAND_NODE "@Bip01 R Finger0"
#define CLAW_LEFT_HAND 4               // DAT_1021bdf8 entries holding the finger nodes
#define CLAW_RIGHT_HAND 5
#define CLAW_STEP_START 0
#define CLAW_STEP_STALK 1              // faces the player and walks up to him while he is far
#define CLAW_STEP_CIRCLE 2             // keeps its distance, circling; goes back to stalking when he is far
#define CLAW_STEP_SWIPE_LEFT 3         // the four claw attacks
#define CLAW_STEP_SWIPE_RIGHT 4
#define CLAW_STEP_DOUBLE 5
#define CLAW_STEP_SLAM 6
#define CLAW_STEP_RETREAT 0xf          // jumps back a random way
#define CLAW_PI 3.1415f
#define CLAW_HALF_TURN 180.0f
#define CLAW_TURN_FULL 6.283f
#define CLAW_QUARTER_TURN 1.5708f
#define CLAW_CIRCLE_ANGLE 0.17453f     // a circling step is the offset to the player turned by this much
#define CLAW_CIRCLE_STEP 12.0f         // length of a circling step
#define CLAW_STALK_FAR 280.0f          // stalking turns into circling when the player is nearer than this
#define CLAW_CIRCLE_FAR 300.0f         // circling turns back into stalking beyond this
#define CLAW_CIRCLE_NEAR 150.0f        // circling may turn into a retreat nearer than this
#define CLAW_REACT_FAR 400.0f          // after an attack it only starts another when the player is nearer than this
#define CLAW_NOTICE_RANGE 500.0f       // the retreat roll only runs when the player is nearer than this
#define CLAW_FAR_LIMIT 300.0f          // a reaction beyond this only stalks
#define CLAW_WALK_SPEED (-20.0f)
#define CLAW_PROBE_HEIGHT 30.0f        // the move vector's height while a step is tested for ground and walls
#define CLAW_NO_GROUND_SCALE 0.1f      // a character with no ground below it moves at a tenth of the speed
#define CLAW_JUMP_SPEED 30.0f
#define CLAW_HOP_SPEED 20.0f
#define CLAW_ROLL_RANGE 10000
#define CLAW_NOTICE_ROLL_NEAR 5000     // behaviours 5 and above notice the player with this chance (of CLAW_ROLL_RANGE)
#define CLAW_NOTICE_ROLL 2500          // any behaviour notices him with this chance
#define CLAW_ALARM_ROLL 200
#define CLAW_RETREAT_ROLL 400
#define CLAW_STRAFE_MODES 3
#define CLAW_STRAFE_AFTER 0x3c
#define CLAW_STRAFE_ROLL_RANGE 1000
#define CLAW_STRAFE_ROLL 0x50
#define CLAW_STRAFE_LEFT 0
#define CLAW_STRAFE_RIGHT 1
#define CLAW_ATTACK_ROLL_COUNT 16      // an attack pick is one of 16 rolls: rolls below CLAW_ATTACK_ROLL_SWIPES give step 3 or 4
#define CLAW_ATTACK_ROLL_SWIPES 13
#define CLAW_ATTACK_SWIPE_FIRST 3
#define CLAW_ATTACK_OTHER_BASE 9       // the other rolls give roll - 9 (steps 4, 5 and 6)
#define CLAW_ATTACK_AGAIN_ODDS 0x14    // after an attack it starts another one unless a roll of this comes up 0
#define CLAW_AREA_FIRST 0x2b           // map areas in which greetings are off
#define CLAW_AREA_LAST 0x2e
#define CLAW_UNSET_TALK (-1)
#define CLAW_UNSET_RANGE (-1)
#define CLAW_PACKED_UNIT 1000000       // DAT_102548c0 digits: the pair above the millions marks a scripted character
#define CLAW_PACKED_RANGE 100
#define CLAW_LEVEL_LIMIT 10
#define CLAW_FRAME_UNSTARTED (-1)      // wait value after an attack: the next step starts over
#define CLAW_AWAY_LEFT (-150.0f)       // the player counts as in front when the offset turned by his heading lies in
#define CLAW_AWAY_RIGHT 150.0f         // x from -150 to 150 and y below 50
#define CLAW_AWAY_BEHIND 50.0f
#define CLAW_SIDE_LEFT 1
#define CLAW_SIDE_RIGHT 2
#define CLAW_MOTION_STAND 2
#define CLAW_MOTION_WALK 1
#define CLAW_MOTION_CIRCLE 1
#define CLAW_MOTION_SWIPE_LEFT 0xc
#define CLAW_MOTION_SWIPE_RIGHT 0xb
#define CLAW_MOTION_DOUBLE 4
#define CLAW_MOTION_SLAM 0x1e
#define CLAW_MOTION_RETREAT 0x11
#define CLAW_PROBE_DISTANCE 200.0f
#define CLAW_PROBE_TRIES 5
#define CLAW_RETREAT_SPREAD 0xb4
#define CLAW_RETREAT_HALF 0x5a
#define CLAW_RETREAT_SIDE_SPREAD 0x5a
#define CLAW_RETREAT_SIDE_HALF 0x2d
#define CLAW_RETREAT_HOP_FRAME 6
#define CLAW_RETREAT_SLOW (-5.0f)
#define CLAW_RETREAT_FAST (-35.0f)
#define CLAW_RETREAT_SLOW_END 5
#define CLAW_RETREAT_FAST_END 0x10
#define CLAW_RETREAT_FRAMES 0x14
#define CLAW_FAILED_RETREAT 10
#define CLAW_HOP_SOUND 0x1c6
#define CLAW_TRAIL_EFFECT 0x456        // the streak left by a finger
#define CLAW_RING_EFFECT 0x3710        // the ring drawn around a finger while it swings (turned flat)
#define CLAW_FLASH_EFFECT 0x3711
#define CLAW_FLARE_EFFECT 0x3712
#define CLAW_TRAIL_MODE 5
#define CLAW_NOTICE_NEAR_LIMIT 5       // behaviour numbers below this are the near kinds
// Swipes (steps 3 and 4)
#define CLAW_SWIPE_SLOW (-7.0f)
#define CLAW_SWIPE_SOUND 0x1cd
#define CLAW_SWIPE_LEFT_TURN_END 0x167
#define CLAW_SWIPE_LEFT_TRAIL_FROM 0x168
#define CLAW_SWIPE_LEFT_RING_FROM 0x164
#define CLAW_SWIPE_LEFT_END 0x16d
#define CLAW_SWIPE_RIGHT_TURN_END 0x15d
#define CLAW_SWIPE_RIGHT_TRAIL_FROM 0x15e
#define CLAW_SWIPE_RIGHT_RING_FROM 0x15a
#define CLAW_SWIPE_RIGHT_END 0x163
// Double swipe (step 5)
#define CLAW_DOUBLE_SOUND 0x1ce
#define CLAW_DOUBLE_TURN_LIMIT 0.34907f
#define CLAW_DOUBLE_TRAIL_FROM 0x70
#define CLAW_DOUBLE_TRAIL_TO 0x93
#define CLAW_DOUBLE_LUNGE_FROM 0x73
#define CLAW_DOUBLE_LUNGE_TO 0x91
#define CLAW_DOUBLE_LUNGE (-12.0f)
#define CLAW_DOUBLE_FLARE_FRAME 0x6f
#define CLAW_DOUBLE_END 0x95
#define CLAW_DOUBLE_WALK (-5.0f)
// Slam (step 6)
#define CLAW_SLAM_SOUND 0x1d1
#define CLAW_SLAM_TURN_END 0x17c
#define CLAW_SLAM_LUNGE_FRAME 0x185
#define CLAW_SLAM_LUNGE (-30.0f)
#define CLAW_SLAM_DECAY 0.95f
#define CLAW_SLAM_TRAIL_FROM 0x173
#define CLAW_SLAM_TRAIL_TO 0x199
#define CLAW_SLAM_FLASH_FRAME 0x172
#define CLAW_SLAM_RING_FROM 0x184
#define CLAW_SLAM_RING_TO 0x196
#define CLAW_SLAM_END 0x1a1

// The size of the streak: behaviours 1 to 4 use `strong`, 0 uses `weak`, 5 and above `normal`.
static __forceinline float ClawEffectSize(int behaviour, float strong, float normal, float weak)
{
    float size;
    if (behaviour != 0 && behaviour < 5)
        size = strong;
    else
    {
        size = normal;
        if (behaviour <= 4)
            size = weak;
    }
    return size;
}

// 0x100aa820 (Enemy_StalkAndClaw). The same skeleton as Enemy_StalkAndStrike with other tuning. The player counts
// as "in front" when his offset turned by his own heading lies in a narrow box (side 1 or 2; the original's wrap loop
// ends above pi, so side is always 1), and a behaviour number (DAT_1020b230) that notices him sends a stalking or
// circling enemy to the retreat step. A reaction request (DAT_10210b30) picks a claw attack at random. Step 1 turns to
// the player and walks up to him (a blocked step jumps, then retreats), step 2 circles him, steps 3 and 4 are
// single-hand swipes (the finger node [4] or [5] draws a streak and a ring), step 5 is the double swipe with both
// hands and step 6 the slam with a sliding lunge. Step 0xf jumps back a random way. Every step may produce a move,
// applied at the end together with the motion. `variant` is not read. Returns 0.
int Enemy_StalkAndClaw(int index, int variant)
{
    Vec4 player;
    Vec4 self;
    Vec4 move;
    int motion = -1;
    int restart = 0;
    int moving = 0;
    int blend = 5;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float amount = (player - self).Length2D();
    int side = 0;
    if ((DAT_1020b230[index] != 0 && DAT_1020b230[index] < 5) ||
        (DAT_1020b230[index] > 4 && FUN_1013f880() % CLAW_ROLL_RANGE < CLAW_NOTICE_ROLL_NEAR) ||
        FUN_1013f880() % CLAW_ROLL_RANGE < CLAW_NOTICE_ROLL)
    {
        Vec4 away;
        away.Copy(self - player);
        away.RotateZ(DAT_10db6ee8[0].heading);
        if (away.x > CLAW_AWAY_LEFT && away.x < CLAW_AWAY_RIGHT && away.y < CLAW_AWAY_BEHIND)
        {
            float angle = (float)atan2((double)away.x, (double)away.y) + CLAW_PI;
            while (angle > CLAW_PI)
                angle = angle - CLAW_TURN_FULL;
            while (angle < CLAW_PI)
                angle = angle + CLAW_TURN_FULL;
            side = CLAW_SIDE_LEFT;
            if (!(angle > 0.0f))
                side = CLAW_SIDE_RIGHT;
        }
    }
    DAT_10255a88[index] = 0;
    if (DAT_10210b30[index] != 0 && DAT_1020b430[index].state != 0)
    {
        if ((!(amount < CLAW_FAR_LIMIT) && side == 0) || DAT_104c00e0 == 0 ||
            ((unsigned int)DAT_104c00e8 >= CLAW_AREA_FIRST && (unsigned int)DAT_104c00e8 <= CLAW_AREA_LAST))
        {
            if (amount < CLAW_FAR_LIMIT)
            {
                DAT_1020b430[index].state = FUN_1013f880() % CLAW_ATTACK_ROLL_COUNT;
                if (DAT_1020b430[index].state < CLAW_ATTACK_ROLL_SWIPES)
                    DAT_1020b430[index].state = DAT_1020b430[index].state % 2 + CLAW_ATTACK_SWIPE_FIRST;
                else
                    DAT_1020b430[index].state = DAT_1020b430[index].state - CLAW_ATTACK_OTHER_BASE;
            }
            else
                DAT_1020b430[index].state = CLAW_STEP_STALK;
            DAT_1020b430[index].wait = 0;
        }
        else
            DAT_1020b430[index].state = CLAW_STEP_RETREAT;
    }
    DAT_10210b30[index] = 0;
    if (FUN_1001a450(index, &self) != 0)
        DAT_1020b430[index].stalled = 1;
    if (DAT_102550e0[index] > CLAW_LEVEL_LIMIT)
        DAT_102550e0[index] = CLAW_LEVEL_LIMIT;
    if (DAT_10468730[index] != 0)
    {
        DAT_10468730[index] = 0;
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(CLAW_NULL_NODE, 0);
    }
    if (DAT_102550e0[index] == 0)
    {
        if (DAT_1020b430[index].state == CLAW_STEP_START)
        {
            DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(CLAW_NULL_NODE, 0);
            DAT_1020b430[index].wait = 0;
            if ((DAT_102548c0[index] / CLAW_PACKED_UNIT) % CLAW_PACKED_RANGE != 0)
            {
                if (DAT_1020b430[index].talk == 0)
                    DAT_1020b430[index].talk = CLAW_UNSET_TALK;
                if (DAT_1020b430[index].range == 0)
                    DAT_1020b430[index].range = CLAW_UNSET_RANGE;
            }
            DAT_1020b430[index].state = CLAW_STEP_STALK;
            DAT_1021bdf8[index][CLAW_LEFT_HAND] = DAT_10db6ee8[index].root->Find(CLAW_LEFT_HAND_NODE, 0);
            DAT_1021bdf8[index][CLAW_RIGHT_HAND] = DAT_10db6ee8[index].root->Find(CLAW_RIGHT_HAND_NODE, 0);
        }
        else if (DAT_1020b430[index].state == CLAW_STEP_STALK)
        {
            motion = CLAW_MOTION_WALK;
            DAT_1020b430[index].wait++;
            if (side != 0)
            {
                DAT_1020b430[index].state = CLAW_STEP_RETREAT;
                DAT_1020b430[index].wait = 0;
            }
            if (DAT_1020b430[index].wait == 1 && DAT_10db6ee8[index].motion->layer != CLAW_MOTION_WALK)
                restart = 1;
            float dy = player.y - self.y;
            float dx = player.x - self.x;
            DAT_10db6ee8[index].targetHeading = (float)atan2((double)dx, (double)dy) + CLAW_PI;
            if (amount > CLAW_STALK_FAR)
            {
                move.y = CLAW_WALK_SPEED;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                moving = 1;
                motion = CLAW_MOTION_WALK;
                move.z = CLAW_PROBE_HEIGHT;
                if (DAT_10db6ee8[index].m_238c != 0)
                {
                    if (DAT_10db6ee8[index].IsClearOfMapLimited(&move) != 0 &&
                        DAT_10db6ee8[index].IsClearAny(&move) != 0 && DAT_10467920[index] == 0)
                    {
                        DAT_1020b430[index].stalled = 0;
                    }
                    else if (DAT_1020b430[index].stalled != 0)
                    {
                        DAT_1020b430[index].state = CLAW_STEP_RETREAT;
                        DAT_1020b430[index].wait = 0;
                    }
                    else
                    {
                        DAT_10db6ee8[index].AddMotionSpeed(CLAW_JUMP_SPEED);
                        DAT_1020b430[index].stalled = 1;
                    }
                }
                move.z = 0;
            }
            else
            {
                DAT_1020b430[index].state = CLAW_STEP_CIRCLE;
                DAT_1020b430[index].wait = 0;
            }
        }
        else if (DAT_1020b430[index].state == CLAW_STEP_CIRCLE)
        {
            if (side != 0)
            {
                DAT_1020b430[index].state = CLAW_STEP_RETREAT;
                DAT_1020b430[index].wait = 0;
            }
            DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CLAW_PI;
            if (DAT_1020b430[index].strafeMode == CLAW_STRAFE_LEFT)
            {
                move.Copy(self - player);
                move.RotateZ(CLAW_CIRCLE_ANGLE);
                move.Copy(move + player - self);
                move.z = 0;
                if (move.Length2D() > CLAW_CIRCLE_STEP)
                {
                    move.Normalized3();
                    move.Copy(move * CLAW_CIRCLE_STEP);
                }
                moving = 1;
                motion = CLAW_MOTION_CIRCLE;
            }
            else if (DAT_1020b430[index].strafeMode == CLAW_STRAFE_RIGHT)
            {
                move.Copy(self - player);
                move.RotateZ(-CLAW_CIRCLE_ANGLE);
                move.Copy(move + player - self);
                move.z = 0;
                if (move.Length2D() > CLAW_CIRCLE_STEP)
                {
                    move.Normalized3();
                    move.Copy(move * CLAW_CIRCLE_STEP);
                }
                moving = 1;
                motion = CLAW_MOTION_CIRCLE;
            }
            else
                motion = CLAW_MOTION_STAND;
            if (++DAT_1020b430[index].wait > CLAW_STRAFE_AFTER &&
                FUN_1013f880() % CLAW_STRAFE_ROLL_RANGE < CLAW_STRAFE_ROLL)
            {
                DAT_1020b430[index].strafeMode = FUN_1013f880() % CLAW_STRAFE_MODES;
                DAT_1020b430[index].wait = 0;
            }
            if (DAT_10467920[index] != 0 || FUN_1013f880() % CLAW_ROLL_RANGE < CLAW_ALARM_ROLL)
                DAT_10210b30[index] = 1;
            if (amount > CLAW_CIRCLE_FAR)
            {
                DAT_1020b430[index].state = CLAW_STEP_STALK;
                DAT_1020b430[index].wait = 0;
            }
            if (amount < CLAW_CIRCLE_NEAR && FUN_1013f880() % CLAW_ROLL_RANGE < CLAW_RETREAT_ROLL)
            {
                DAT_1020b430[index].state = CLAW_STEP_RETREAT;
                DAT_1020b430[index].wait = 0;
            }
        }
        else if (DAT_1020b430[index].state == CLAW_STEP_SWIPE_LEFT)
        {
            motion = CLAW_MOTION_SWIPE_LEFT;
            if (DAT_1020b430[index].previousState == CLAW_STEP_SWIPE_LEFT ||
                DAT_1020b430[index].previousState == CLAW_STEP_SWIPE_RIGHT)
                blend = 0;
            DAT_1020b430[index].wait++;
            DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading;
            if (DAT_1020b430[index].wait == 1)
            {
                restart = 1;
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CLAW_PI;
                FUN_10065b10(index, CLAW_SWIPE_SOUND, 0, 0);
            }
            if (DAT_10db6ee8[index].motionFrame < CLAW_SWIPE_LEFT_TURN_END)
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CLAW_PI;
            move.y = CLAW_SWIPE_SLOW;
            move.RotateZ(-DAT_10db6ee8[index].heading);
            moving = 1;
            if (DAT_10db6ee8[index].motionFrame > CLAW_SWIPE_LEFT_TRAIL_FROM &&
                DAT_10db6ee8[index].motionFrame < CLAW_SWIPE_LEFT_END)
            {
                EffectOptions options;
                Matrix4 matrix;
                Vec4 offset;
                options.m_0000 = 0;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0008 = ClawEffectSize(DAT_1020b230[index], 1.3f, 1.0f, 0.75f);
                options.m_000e = CLAW_TRAIL_MODE;
                if (DAT_1021bdf8[index][CLAW_LEFT_HAND] != 0)
                {
                    matrix.SetIdentity();
                    matrix.SetTranslation(DAT_1021bdf8[index][CLAW_LEFT_HAND]->worldMatrix.GetTranslation());
                    FUN_1001d8b0(index, CLAW_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
                }
            }
            if (DAT_10db6ee8[index].motionFrame > CLAW_SWIPE_LEFT_RING_FROM &&
                DAT_10db6ee8[index].motionFrame < CLAW_SWIPE_LEFT_END)
            {
                Matrix4 matrix;
                Vec4 offset;
                EffectOptions options;
                options.followOwner = 1;
                matrix = RotationX(CLAW_QUARTER_TURN) * RotationZ(DAT_10db6ee8[index].heading);
                if (DAT_1021bdf8[index][CLAW_LEFT_HAND] != 0)
                {
                    matrix.SetTranslation(DAT_1021bdf8[index][CLAW_LEFT_HAND]->worldMatrix.GetTranslation());
                    FUN_1001d8b0(index, CLAW_RING_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
                }
            }
            if (DAT_10db6ee8[index].motionFrame == CLAW_SWIPE_LEFT_END)
            {
                DAT_1020b430[index].state = CLAW_STEP_STALK;
                DAT_1020b430[index].wait = CLAW_FRAME_UNSTARTED;
            }
        }
        else if (DAT_1020b430[index].state == CLAW_STEP_SWIPE_RIGHT)
        {
            motion = CLAW_MOTION_SWIPE_RIGHT;
            if (DAT_1020b430[index].previousState == CLAW_STEP_SWIPE_LEFT ||
                DAT_1020b430[index].previousState == CLAW_STEP_SWIPE_RIGHT)
                blend = 0;
            DAT_1020b430[index].wait++;
            DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading;
            if (DAT_1020b430[index].wait == 1)
            {
                restart = 1;
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CLAW_PI;
            }
            if (DAT_10db6ee8[index].motionFrame < CLAW_SWIPE_RIGHT_TURN_END)
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CLAW_PI;
            move.y = CLAW_SWIPE_SLOW;
            move.RotateZ(-DAT_10db6ee8[index].heading);
            moving = 1;
            if (DAT_10db6ee8[index].motionFrame > CLAW_SWIPE_RIGHT_TRAIL_FROM &&
                DAT_10db6ee8[index].motionFrame < CLAW_SWIPE_RIGHT_END)
            {
                EffectOptions options;
                Matrix4 matrix;
                Vec4 offset;
                options.m_0000 = 0;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0008 = ClawEffectSize(DAT_1020b230[index], 1.3f, 1.0f, 0.75f);
                options.m_000e = CLAW_TRAIL_MODE;
                if (DAT_1021bdf8[index][CLAW_RIGHT_HAND] != 0)
                {
                    matrix.SetIdentity();
                    matrix.SetTranslation(DAT_1021bdf8[index][CLAW_RIGHT_HAND]->worldMatrix.GetTranslation());
                    FUN_1001d8b0(index, CLAW_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
                }
            }
            if (DAT_10db6ee8[index].motionFrame > CLAW_SWIPE_RIGHT_RING_FROM &&
                DAT_10db6ee8[index].motionFrame < CLAW_SWIPE_RIGHT_END)
            {
                Matrix4 matrix;
                Vec4 offset;
                EffectOptions options;
                options.followOwner = 1;
                matrix = RotationX(CLAW_QUARTER_TURN) * RotationZ(DAT_10db6ee8[index].heading);
                if (DAT_1021bdf8[index][CLAW_RIGHT_HAND] != 0)
                {
                    matrix.SetTranslation(DAT_1021bdf8[index][CLAW_RIGHT_HAND]->worldMatrix.GetTranslation());
                    FUN_1001d8b0(index, CLAW_RING_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
                }
            }
            if (DAT_10db6ee8[index].motionFrame == CLAW_SWIPE_RIGHT_END)
            {
                DAT_1020b430[index].state = CLAW_STEP_STALK;
                DAT_1020b430[index].wait = CLAW_FRAME_UNSTARTED;
            }
        }
        else if (DAT_1020b430[index].state == CLAW_STEP_DOUBLE)
        {
            motion = CLAW_MOTION_DOUBLE;
            if (++DAT_1020b430[index].wait == 1)
            {
                restart = 1;
                FUN_10065b10(index, CLAW_DOUBLE_SOUND, 0, 0);
            }
            DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CLAW_PI;
            FUN_1001bfb0(index, CLAW_DOUBLE_TURN_LIMIT);
            if (DAT_10db6ee8[index].motionFrame > CLAW_DOUBLE_TRAIL_FROM &&
                DAT_10db6ee8[index].motionFrame < CLAW_DOUBLE_TRAIL_TO)
            {
                EffectOptions options;
                Matrix4 matrix;
                Vec4 offset;
                options.m_0000 = 0;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0008 = ClawEffectSize(DAT_1020b230[index], 1.5f, 1.2f, 1.0f);
                options.m_000e = CLAW_TRAIL_MODE;
                if (DAT_1021bdf8[index][CLAW_LEFT_HAND] != 0)
                {
                    matrix.SetIdentity();
                    matrix.SetTranslation(DAT_1021bdf8[index][CLAW_LEFT_HAND]->worldMatrix.GetTranslation());
                    FUN_1001d8b0(index, CLAW_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
                }
                if (DAT_1021bdf8[index][CLAW_RIGHT_HAND] != 0)
                {
                    matrix.SetIdentity();
                    matrix.SetTranslation(DAT_1021bdf8[index][CLAW_RIGHT_HAND]->worldMatrix.GetTranslation());
                    FUN_1001d8b0(index, CLAW_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
                }
            }
            if (DAT_10db6ee8[index].motionFrame < CLAW_DOUBLE_LUNGE_FROM ||
                DAT_10db6ee8[index].motionFrame > CLAW_DOUBLE_LUNGE_TO)
                move.y = CLAW_DOUBLE_WALK;
            else
                move.y = CLAW_DOUBLE_LUNGE;
            move.RotateZ(-DAT_10db6ee8[index].heading);
            if (DAT_10db6ee8[index].motionFrame == CLAW_DOUBLE_FLARE_FRAME)
            {
                EffectOptions options;
                Vec4 offset;
                options.followOwner = 1;
                if (DAT_1021bdf8[index][CLAW_LEFT_HAND] != 0)
                    FUN_1001d8b0(index, CLAW_FLARE_EFFECT, offset, -1, -1,
                                 &DAT_1021bdf8[index][CLAW_LEFT_HAND]->worldMatrix, 0, 0, 0, &options);
                if (DAT_1021bdf8[index][CLAW_RIGHT_HAND] != 0)
                    FUN_1001d8b0(index, CLAW_FLARE_EFFECT, offset, -1, -1,
                                 &DAT_1021bdf8[index][CLAW_RIGHT_HAND]->worldMatrix, 0, 0, 0, &options);
            }
            if (DAT_10db6ee8[index].motionFrame == CLAW_DOUBLE_END)
            {
                DAT_1020b430[index].state = CLAW_STEP_STALK;
                DAT_1020b430[index].wait = CLAW_FRAME_UNSTARTED;
            }
            moving = 1;
        }
        else if (DAT_1020b430[index].state == CLAW_STEP_SLAM)
        {
            motion = CLAW_MOTION_SLAM;
            if (++DAT_1020b430[index].wait == 1)
            {
                restart = 1;
                DAT_10802bf8[index].step.Fill(0.0f);
                FUN_10065b10(index, CLAW_SLAM_SOUND, 0, 0);
            }
            if (DAT_10db6ee8[index].motionFrame < CLAW_SLAM_TURN_END)
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CLAW_PI;
            if (DAT_10db6ee8[index].motionFrame == CLAW_SLAM_LUNGE_FRAME)
            {
                DAT_10802bf8[index].step.y = CLAW_SLAM_LUNGE;
                DAT_10802bf8[index].step.RotateZ(-DAT_10db6ee8[index].heading);
            }
            move.Copy(DAT_10802bf8[index].step * CLAW_SLAM_DECAY);
            moving = 1;
            DAT_10802bf8[index].step.Copy(move);
            if (DAT_10db6ee8[index].motionFrame > CLAW_SLAM_TRAIL_FROM &&
                DAT_10db6ee8[index].motionFrame < CLAW_SLAM_TRAIL_TO)
            {
                EffectOptions options;
                Matrix4 matrix;
                Vec4 offset;
                options.m_0000 = 0;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0008 = ClawEffectSize(DAT_1020b230[index], 2.3f, 1.8f, 1.5f);
                options.m_000e = CLAW_TRAIL_MODE;
                if (DAT_1021bdf8[index][CLAW_RIGHT_HAND] != 0)
                {
                    matrix.SetIdentity();
                    matrix.SetTranslation(DAT_1021bdf8[index][CLAW_RIGHT_HAND]->worldMatrix.GetTranslation());
                    FUN_1001d8b0(index, CLAW_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
                }
            }
            if (DAT_10db6ee8[index].motionFrame == CLAW_SLAM_FLASH_FRAME && DAT_1021bdf8[index][CLAW_RIGHT_HAND] != 0)
            {
                EffectOptions options;
                Vec4 offset;
                options.followOwner = 1;
                FUN_1001d8b0(index, CLAW_FLASH_EFFECT, offset, -1, -1,
                             &DAT_1021bdf8[index][CLAW_RIGHT_HAND]->worldMatrix, 0, 0, 0, &options);
            }
            if (DAT_10db6ee8[index].motionFrame > CLAW_SLAM_RING_FROM &&
                DAT_10db6ee8[index].motionFrame < CLAW_SLAM_RING_TO)
            {
                EffectOptions options;
                Vec4 offset;
                Matrix4 matrix;
                options.followOwner = 1;
                matrix = RotationX(CLAW_QUARTER_TURN) * RotationZ(DAT_10db6ee8[index].heading);
                if (DAT_1021bdf8[index][CLAW_RIGHT_HAND] != 0)
                {
                    matrix.SetTranslation(DAT_1021bdf8[index][CLAW_RIGHT_HAND]->worldMatrix.GetTranslation());
                    FUN_1001d8b0(index, CLAW_RING_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
                }
            }
            if (DAT_10db6ee8[index].motionFrame == CLAW_SLAM_END)
            {
                DAT_1020b430[index].state = CLAW_STEP_STALK;
                DAT_1020b430[index].wait = CLAW_FRAME_UNSTARTED;
            }
        }
        else if (DAT_1020b430[index].state == CLAW_STEP_RETREAT)
        {
            if (DAT_1020b430[index].wait == 0)
            {
                int tries = 0;
                do
                {
                    if (side != 0)
                    {
                        float turn = CLAW_QUARTER_TURN;
                        if (side == CLAW_SIDE_LEFT)
                            turn = -CLAW_QUARTER_TURN;
                        move.Fill(0.0f);
                        DAT_10255160[index][0] =
                            (float)(FUN_1013f880() % CLAW_RETREAT_SIDE_SPREAD - CLAW_RETREAT_SIDE_HALF) * CLAW_PI /
                                CLAW_HALF_TURN -
                            DAT_10db6ee8[index].heading + turn;
                    }
                    else
                    {
                        DAT_10255160[index][0] =
                            (float)(FUN_1013f880() % CLAW_RETREAT_SPREAD - CLAW_RETREAT_HALF) * CLAW_PI /
                                CLAW_HALF_TURN -
                            DAT_10db6ee8[index].heading + CLAW_PI;
                    }
                    DAT_10255160[index][0] =
                        (float)(FUN_1013f880() % CLAW_RETREAT_SPREAD - CLAW_RETREAT_HALF) * CLAW_PI / CLAW_HALF_TURN +
                        CLAW_PI;
                    if (FUN_1001a590(index, &self, CLAW_PROBE_DISTANCE) != 0)
                        break;
                    tries++;
                } while (tries < CLAW_PROBE_TRIES);
                if (tries == CLAW_PROBE_TRIES)
                {
                    DAT_1020b430[index].state = CLAW_STEP_STALK;
                    DAT_1020b430[index].wait = 0;
                    DAT_1020b430[index].retreatFrames = CLAW_FAILED_RETREAT;
                    return 0;
                }
            }
            if (DAT_1020b430[index].wait == CLAW_RETREAT_HOP_FRAME)
            {
                FUN_10065b10(index, CLAW_HOP_SOUND, 0, 0);
                DAT_10db6ee8[index].AddMotionSpeed(CLAW_HOP_SPEED);
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + CLAW_PI;
            }
            if (DAT_1020b430[index].wait < CLAW_RETREAT_SLOW_END || DAT_1020b430[index].wait > 0xf)
                move.y = CLAW_RETREAT_SLOW;
            else if (DAT_1020b430[index].wait < CLAW_RETREAT_FAST_END)
                move.y = CLAW_RETREAT_FAST;
            move.RotateZ(DAT_10255160[index][0]);
            moving = 1;
            motion = CLAW_MOTION_RETREAT;
            if (++DAT_1020b430[index].wait > CLAW_RETREAT_FRAMES)
            {
                DAT_1020b430[index].state = CLAW_STEP_STALK;
                DAT_1020b430[index].wait = CLAW_FRAME_UNSTARTED;
            }
        }
    }
    if ((amount < CLAW_NOTICE_RANGE || side != 0) && DAT_1020b430[index].state < 2 && DAT_104c00e0 != 0 &&
        ((unsigned int)DAT_104c00e8 < CLAW_AREA_FIRST || (unsigned int)DAT_104c00e8 > CLAW_AREA_LAST) &&
        ((DAT_1020b230[index] != 0 && DAT_1020b230[index] < 5) ||
         (DAT_1020b230[index] > 4 && FUN_1013f880() % CLAW_ROLL_RANGE < CLAW_NOTICE_ROLL_NEAR) ||
         FUN_1013f880() % CLAW_ROLL_RANGE < CLAW_NOTICE_ROLL))
    {
        DAT_1020b430[index].state = CLAW_STEP_RETREAT;
        DAT_1020b430[index].wait = 0;
    }
    if (moving != 0)
    {
        if (DAT_10467920[index] != 0)
            move.Copy(move * CLAW_NO_GROUND_SCALE);
        FUN_100241e0(index, move.x, move.y, move.z);
        DAT_10802bf8[index].position.Copy(move);
        DAT_10802bf8[index].position.z = 0;
    }
    if (motion > -1)
        DAT_10db6ee8[index].PlayMotion(motion, restart, blend, 1.0f);
    DAT_1020b430[index].previousState = DAT_1020b430[index].state;
    if (DAT_1020b430[index].state == CLAW_STEP_STALK && DAT_1020b430[index].wait == CLAW_FRAME_UNSTARTED &&
        amount < CLAW_REACT_FAR && FUN_1013f880() % CLAW_ATTACK_AGAIN_ODDS != 0)
    {
        DAT_1020b430[index].state = FUN_1013f880() % CLAW_ATTACK_ROLL_COUNT;
        if (DAT_1020b430[index].state < CLAW_ATTACK_ROLL_SWIPES)
            DAT_1020b430[index].state = DAT_1020b430[index].state % 2 + CLAW_ATTACK_SWIPE_FIRST;
        else
            DAT_1020b430[index].state = DAT_1020b430[index].state - CLAW_ATTACK_OTHER_BASE;
        DAT_1020b430[index].wait = 0;
    }
    if (DAT_1020b430[index].retreatFrames != 0)
    {
        DAT_1020b430[index].retreatFrames--;
        if (DAT_1020b430[index].state == CLAW_STEP_RETREAT)
        {
            DAT_1020b430[index].state = CLAW_STEP_STALK;
            DAT_1020b430[index].wait = 0;
        }
    }
    return 0;
}
