// An enemy that wanders until it notices the player, then runs at him, pounces on him and breathes fire (0x100675d0,
// the per-kind behaviour 7 of 0x100f9670). See the comment above Enemy_PounceAndBreathe for the steps.

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "effect_manager.h"

#include "anim_model.h"

#include "char_state.h"

#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "last_position.h"
#include "map_vars.h"
#include "world.h"

#include "pounce_enemy.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_100241e0(int index, float x, float y, float z);            // Character_SetMove
extern "C" int FUN_1000d620(int index, int key, int frames);                  // KeyPrompt_Show
extern "C" int FUN_1001c090(int index, int force);                            // Char_PlaceOnGround
extern "C" int FUN_100509e0(int index, Vec4 *wall, Vec4 *ground, float *wallDistance, float *height, float *drop,
                            float reach);                                     // Char_ProbeSideStep
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

#define POUNCE_SPINE_NODE "Bip01 Spine"
#define POUNCE_MOUTH_NODE "@Dummy05"
#define POUNCE_NO_MOTION (-1)
#define POUNCE_NO_SHOT_POINT (-200000.0f)   // x of the shot point (DAT_10868838[slot][0]) while there is none
#define POUNCE_STATE_WANDER 0
#define POUNCE_STATE_WHIRL 1                // spins on the spot, then leaps away
#define POUNCE_STATE_ATTACK 10
#define POUNCE_STEP_ROAR 0
#define POUNCE_STEP_CHASE 1
#define POUNCE_STEP_POUNCE 2
#define POUNCE_STEP_BREATHE 3
#define POUNCE_PI 3.1415f
#define POUNCE_TWO_PI 6.283f
#define POUNCE_ALERT_PROMPT 15              // the "!" over its head
#define POUNCE_PROMPT_FRAMES 60
#define POUNCE_SIGHT_HEIGHT 150             // a sight probe that found a step lower than this still sees the player
#define POUNCE_SIGHT_FRAMES 300
#define POUNCE_PROBE_PERIOD 10
#define POUNCE_GROUND_PERIOD 30             // the ground below it is looked up once in this many frames
#define POUNCE_SHADOW_LIFT 2.0f
#define POUNCE_PIT_MARGIN 10.0f
#define POUNCE_NOTICE_HEIGHT 250.0f         // the player must be nearer than this in height to be noticed
#define POUNCE_NOTICE_NEAR 400.0f
#define POUNCE_NOTICE_FAR 1000.0f
#define POUNCE_LOSE_HEIGHT 500.0f
#define POUNCE_NOTICE_ANGLE 0.17453f        // 10 degrees: noticed when it already looks this much at the player
#define POUNCE_NOTICE_ANGLE_CLOSE 0.06981f
#define POUNCE_WAKE_FIRST 58                // the slots whose same-kind neighbours it wakes: 58 to 91
#define POUNCE_WAKE_END 92
#define POUNCE_WAKE_RANGE_SQUARED 1000000.0f
#define POUNCE_WAKE_HEIGHT 250.0f
#define POUNCE_WAKE_PERIOD 10
#define POUNCE_WANDER_SPEED 5.0f
#define POUNCE_WANDER_TURN 0.17453f
#define POUNCE_WANDER_ROLL 400
#define POUNCE_WANDER_FLIP 50
#define POUNCE_STUCK_FACTOR 1.5f
#define POUNCE_WHIRL_MIN 30
#define POUNCE_WHIRL_SPAN 30
#define POUNCE_FIXED_POINT 100000.0f        // the whirl angles are kept in 1/100000 radians
#define POUNCE_WHIRL_TURN 0.837758f         // 48 degrees a frame
#define POUNCE_WHIRL_HALF 0xf
#define POUNCE_WHIRL_END 0x1e
#define POUNCE_WHIRL_SOUND 0x2a3
#define POUNCE_WHIRL_LEAP_END 0x41
#define POUNCE_WHIRL_ARM 0x19
#define POUNCE_WHIRL_ROLL 50000
#define POUNCE_LEAP_SPEED 45.5f
#define POUNCE_BOUNCE_SPEED 18.0f
#define POUNCE_LAND_SPEED 34.5f
#define POUNCE_LEAP_LIFT 10.0f
#define POUNCE_LANDING_LIMIT 20.0f
#define POUNCE_LEAP_DRIFT (-30.0f)
#define POUNCE_ROAR_FRAMES 0x14
#define POUNCE_ROAR_SOUND 0x2a4
#define POUNCE_ROAR_SPEED 25.0f
#define POUNCE_CHASE_RANGE 150.0f
#define POUNCE_CHASE_SCALE 2.0f
#define POUNCE_HALF 0.5f
#define POUNCE_CHASE_SPEED 16.0f
#define POUNCE_CHASE_SPEED_PIT 9.0f
#define POUNCE_PIT_CLIMB 20.0f
#define POUNCE_PIT_PULL 5.0f
#define POUNCE_JUMP_LIFT 27.5f
#define POUNCE_CLOSE 500.0f
#define POUNCE_ATTACK_ROLL 100
#define POUNCE_ATTACK_CHANCE 0x1e
#define POUNCE_STALLED_FRAMES 10
#define POUNCE_GRAVITY_Z 0.25f
#define POUNCE_RISE 0.55f
#define POUNCE_POUNCE_GRAVITY_Z 0.63f
#define POUNCE_POUNCE_RISE 1.0f
#define POUNCE_POUNCE_CAST 0x28
#define POUNCE_SHOT_HEIGHT (-15.0f)
#define POUNCE_TARGET_LIFT 30.0f
#define POUNCE_SHOT_SPACING_SQUARED 2500.0f
#define POUNCE_SHOT_EFFECT 0x688
#define POUNCE_POUNCE_JUMP_SPEED 15.0f
#define POUNCE_POUNCE_MOTION 4
#define POUNCE_POUNCE_SOUND 0x2a6
#define POUNCE_DUST_EFFECT 0x655
#define POUNCE_DUST_OFFSET 0x24c
#define POUNCE_HIT_FROM 3                   // frames 4 to 34 and 44 to 74 of the pounce hit (DAT_1021f6c0)
#define POUNCE_HIT_TO 0x23
#define POUNCE_HIT2_FROM 0x2b
#define POUNCE_HIT2_TO 0x4b
#define POUNCE_POUNCE_END 0x8c
#define POUNCE_POUNCE_RECOVER 0x50
#define POUNCE_POUNCE_SPEED 24.0f
#define POUNCE_POUNCE_SPEED_GROUND 48.0f
#define POUNCE_POUNCE_SLOW 0.9f
#define POUNCE_BREATH_MOTION 7
#define POUNCE_BREATH_AIM_FRAMES 0xf
#define POUNCE_BREATH_END 0x78
#define POUNCE_BREATH_SOUND_PERIOD 8
#define POUNCE_BREATH_SOUND 0x1be
#define POUNCE_BREATH_BACK (-16.0f)
#define POUNCE_BREATH_PROBE 30.0f
#define POUNCE_BREATH_LAST 0x77
#define POUNCE_BREATH_SCALE 1.5f
#define POUNCE_BREATH_FLAG 5
#define POUNCE_FIRE_EFFECT 0x3707
#define POUNCE_FIRE_PERIOD 3
#define POUNCE_SMOKE_EFFECT 0x456
#define POUNCE_BREATH_RELEASE 0x96
#define POUNCE_FALL_DAMP 0.6f
#define POUNCE_FALL_PULL_LOW 8.0f
#define POUNCE_FALL_PULL_NEAR 5.0f
#define POUNCE_GROUND_DAMP 0.5f
#define POUNCE_MOTION_SPEED 1.0f
#define POUNCE_BLEND 5

// 1 when `position` is less than `margin` above the pit floor and inside the pit rectangle (a pit on the map).
static __forceinline int IsOverPit(const Vec4 &position, float margin)
{
    if (DAT_104bfe24 + margin > position.z)
    {
        int y = (int)position.y;
        int x = (int)position.x;
        if (x >= (int)DAT_104d29e8.x && y >= (int)DAT_104d29e8.y && x <= (int)DAT_104d29f8.x &&
            y <= (int)DAT_104d29f8.y && DAT_104bfe28 != 0)
            return 1;
    }
    return 0;
}

// 0x100675d0 (Enemy_PounceAndBreathe). Does the common upkeep first: it looks up its spine and mouth nodes, wakes up
// at a reaction request or when its health changes, keeps the shadow height, settles its mode (2 over the pit area)
// and probes the sight line to the player every tenth frame. State 0 wanders (a step along its heading, turning on a
// blocked way) and notices the player nearer than 400 units, hit or when it faces him; state 1 whirls on the spot and
// leaps away; state 10 is the attack: step 0 roars, step 1 runs at the player (circling away on a blocked way) and
// picks the pounce (step 2: a leap with a dust effect) or the fire breath (step 3: a stream of fire effects from the
// mouth node) at random, and gives up when the player is far away. Awake ones wake their same-kind neighbours (slots
// 58 to 91). Every state ends in the common move: falling over the pit, the damping and the motion speed. Returns 1
// while the slot is flagged gone, 0 otherwise.
int Enemy_PounceAndBreathe(int index)
{
    int motion = POUNCE_NO_MOTION;
    int lineClear;
    if (DAT_1020b430[index].probeResult == 0 ||
        (DAT_1020b430[index].probeResult == 2 && DAT_1020b430[index].probeHeight < POUNCE_SIGHT_HEIGHT))
        lineClear = 1;
    else
        lineClear = 0;
    if (DAT_10210d30[index] != 0)
        return 1;
    if (DAT_104483e8[index] == 0)
        return 0;
    if (DAT_1020b430[index].nodesFound == 0)
    {
        DAT_1020b430[index].nodesFound = 1;
        DAT_1021bdf8[index][1] = DAT_10db6ee8[index].root->Find(POUNCE_SPINE_NODE, 0);
        DAT_1021bdf8[index][2] = DAT_10db6ee8[index].root->Find(POUNCE_MOUTH_NODE, 0);
        DAT_10868838[index][0].SetIdentity();
        DAT_10868838[index][0]._41 = POUNCE_NO_SHOT_POINT;
    }
    if (DAT_10210b30[index] != 0)
    {
        DAT_10210b30[index] = 0;
        if (DAT_1020b430[index].state != 0)
        {
            DAT_1020b430[index].state = 0;
            DAT_1020b430[index].pounceStep = 0;
            DAT_1020b430[index].stepFrames = 0;
        }
        if (DAT_1020b430[index].noticed == 0)
        {
            DAT_1020b430[index].noticed = 1;
            FUN_1000d620(index, POUNCE_ALERT_PROMPT, POUNCE_PROMPT_FRAMES);
            DAT_1020b430[index].seenHealth = DAT_10daaae8[index].stats[STAT_HEALTH];
        }
    }
    int lastHealth = DAT_1020b430[index].seenHealth;
    int health = DAT_10daaae8[index].stats[STAT_HEALTH];
    if (lastHealth != health)
    {
        if (lastHealth != 0)
        {
            DAT_1020b430[index].noticed = 1;
            FUN_1000d620(index, POUNCE_ALERT_PROMPT, POUNCE_PROMPT_FRAMES);
            DAT_104483e8[index] = -1;
        }
        DAT_1020b430[index].seenHealth = health;
    }
    DAT_10218fc0[index] = 0;
    if (DAT_10db6ee8[index].m_238c != 0)
        DAT_10255160[index][0] = DAT_10db6ee8[index].matrix._43 + POUNCE_SHADOW_LIFT;
    else
    {
        if (DAT_104c0380 % POUNCE_GROUND_PERIOD == index % POUNCE_GROUND_PERIOD)
        {
            Vec4 unused;
            DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
            DAT_10db6ee8[index].m_0012 = 2;
            FUN_1001c090(index, 1);
            DAT_10255160[index][0] = DAT_10db6ee8[index].shadowMatrix._43;
            DAT_11b002c0 = 0;
        }
        DAT_10db6ee8[index].shadowMatrix._43 = DAT_10255160[index][0];
    }
    Vec4 player;
    Vec4 self;
    Vec4 diff;
    int state = DAT_1020b430[index].state;
    int step = DAT_1020b430[index].pounceStep;
    player.Copy(DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    diff.Copy(player - self);
    float amount = (self - player).Length2D();
    float distance = (self - player).Length();
    int overPit = 0;
    if (DAT_104bfe24 + POUNCE_PIT_MARGIN > self.z)
    {
        int y = (int)self.y;
        int x = (int)self.x;
        if (x >= (int)DAT_104d29e8.x && y >= (int)DAT_104d29e8.y && x <= (int)DAT_104d29f8.x &&
            y <= (int)DAT_104d29f8.y && DAT_104bfe28 != 0)
            overPit = 1;
    }
    if (overPit == 1)
    {
        DAT_1020b430[index].overPit = 1;
        DAT_10db6ee8[index].mode = 2;
        DAT_10db6ee8[index].m_238c = 1;
    }
    else
    {
        DAT_1020b430[index].overPit = 0;
        DAT_10db6ee8[index].mode = 1;
    }
    float heightGap = (float)floor(diff.z);
    if (!(heightGap < POUNCE_NOTICE_HEIGHT) || DAT_1020b430[index].noticed != 0 ||
        (!(distance < POUNCE_NOTICE_NEAR) && (DAT_1030b8f8[index] == 0 || !(distance < POUNCE_NOTICE_FAR))) ||
        state == POUNCE_STATE_ATTACK || lineClear == 0)
    {
        if (DAT_1020b430[index].noticed == 1 && DAT_1020b430[index].sightFrames == 0 &&
            (floorf(diff.z) > POUNCE_LOSE_HEIGHT ||
             (DAT_1030b8f8[index] == 0 && distance > POUNCE_NOTICE_FAR && state != POUNCE_STATE_ATTACK &&
              step != POUNCE_STEP_POUNCE)))
        {
            DAT_1020b430[index].noticed = 0;
            DAT_1020b430[index].state = 0;
            DAT_1020b430[index].pounceStep = 0;
            DAT_1020b430[index].stepFrames = 0;
        }
    }
    else
    {
        float turn = atan2f(diff.x, diff.y) + POUNCE_PI - DAT_10db6ee8[index].heading;
        while (turn > POUNCE_PI)
            turn -= POUNCE_TWO_PI;
        while (turn < -POUNCE_PI)
            turn += POUNCE_TWO_PI;
        if ((DAT_10c66710 == 0 && floorf(turn) < POUNCE_NOTICE_ANGLE) || floorf(turn) < POUNCE_NOTICE_ANGLE_CLOSE)
        {
            DAT_1020b430[index].noticed = 1;
            FUN_1000d620(index, POUNCE_ALERT_PROMPT, POUNCE_PROMPT_FRAMES);
            DAT_104483e8[index] = -1;
        }
    }
    if (lineClear != 0)
        DAT_1020b430[index].sightFrames = POUNCE_SIGHT_FRAMES;
    else if (DAT_1020b430[index].sightFrames > 0)
        DAT_1020b430[index].sightFrames--;
    if (DAT_1020b430[index].noticed != 0 && DAT_10db6ee8[index].motionFrame % POUNCE_WAKE_PERIOD == 0 &&
        DAT_10c66710 == 0)
    {
        Vec4 otherPosition;
        for (int other = POUNCE_WAKE_FIRST; other < POUNCE_WAKE_END; other++)
        {
            if (DAT_10daaae8[index].kind == DAT_10daaae8[other].kind)
            {
                otherPosition.Copy(DAT_10db6ee8[other].matrix.GetTranslation());
                int otherClear;
                if (DAT_1020b430[other].probeResult == 0 ||
                    (DAT_1020b430[other].probeResult == 2 && DAT_1020b430[other].probeHeight < POUNCE_SIGHT_HEIGHT))
                    otherClear = 1;
                else
                    otherClear = 0;
                if (DAT_1030b8f8[other] == 0 &&
                    !((player - otherPosition).LengthSquared3() < POUNCE_WAKE_RANGE_SQUARED))
                    continue;
                if (DAT_1020b430[other].noticed == 0 && floorf(player.z - otherPosition.z) < POUNCE_WAKE_HEIGHT &&
                    otherClear != 0)
                    DAT_1020b430[other].seenHealth = -1;
            }
        }
    }
    if (DAT_1020b430[index].probeDelay == 0)
    {
        float savedHeading = DAT_10db6ee8[index].heading;
        Vec4 ground;
        float angle = (float)atan2((double)diff.x, (double)diff.y);
        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
        DAT_10db6ee8[index].heading = angle + POUNCE_PI;
        DAT_1020b430[index].probeResult = FUN_100509e0(index, 0, &ground, 0, &amount, &angle, amount);
        DAT_11b002c0 = 0;
        DAT_10db6ee8[index].heading = savedHeading;
        DAT_1020b430[index].probeHeight = (int)amount;
        DAT_1020b430[index].probeDelay = POUNCE_PROBE_PERIOD;
    }
    else
        DAT_1020b430[index].probeDelay--;
    if (state == POUNCE_STATE_WANDER)
    {
        if (step == 0)
        {
            Vec4 move;
            amount = 0.0f;
            move.Fill(amount);
            move.y = POUNCE_WANDER_SPEED;
            move.RotateZ(POUNCE_PI - DAT_10db6ee8[index].heading);
            if (DAT_104bfd08 == 0)
                FUN_100241e0(index, move.x, move.y, 0.0f);
            Vec4 spareA;
            Vec4 spareB;
            Vec4 spareC;
            amount = POUNCE_CHASE_SCALE;
            move.Copy(move * amount);
            int blocked;
            if (DAT_10db6ee8[index].IsClearOfMapLimited(&move) == 0 || DAT_10db6ee8[index].IsClearAny(&move) == 0)
                blocked = 1;
            else
                blocked = 0;
            amount = POUNCE_HALF;
            move.Copy(move * amount);
            int roll = FUN_1013f880() % POUNCE_WANDER_ROLL;
            if (blocked == 1)
            {
                if (DAT_1020b430[index].turnRate == 0)
                    DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].targetHeading + POUNCE_WANDER_TURN;
                else
                    DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].targetHeading - POUNCE_WANDER_TURN;
            }
            else if (roll != 0 && !((self - DAT_10802bf8[index].position).Length() <
                                           DAT_10db6ee8[index].speedScale * POUNCE_STUCK_FACTOR))
            {
                if (roll < POUNCE_WANDER_FLIP)
                    DAT_1020b430[index].turnRate = (DAT_1020b430[index].turnRate == 0);
            }
            else
            {
                DAT_1020b430[index].state = POUNCE_STATE_WHIRL;
                DAT_1020b430[index].pounceStep = FUN_1013f880() % POUNCE_WHIRL_SPAN + POUNCE_WHIRL_MIN;
                DAT_1020b430[index].stepFrames = 0;
                DAT_1020b430[index].turnRate = (int)(DAT_10db6ee8[index].heading * POUNCE_FIXED_POINT);
            }
        }
        motion = 0;
        if (DAT_1020b430[index].noticed == 1 && (DAT_1030b8f8[index] != 0 || distance < POUNCE_NOTICE_FAR))
        {
            DAT_1020b430[index].pounceStep = 0;
            DAT_1020b430[index].stepFrames = 0;
            DAT_1020b430[index].state = POUNCE_STATE_ATTACK;
        }
    }
    else if (state == POUNCE_STATE_WHIRL)
    {
        DAT_10218fc0[index] = 1;
        int whirlStart = DAT_1020b430[index].pounceStep;
        int frame = DAT_1020b430[index].stepFrames + 1;
        DAT_104483e8[index] = -1;
        DAT_1020b430[index].stepFrames = frame;
        if (frame < whirlStart + POUNCE_WHIRL_ARM)
            motion = 2;
        if (frame >= whirlStart)
        {
            if (frame == whirlStart || frame == whirlStart + POUNCE_WHIRL_HALF)
            {
                DAT_10db6ee8[index].AddMotionSpeed(POUNCE_BOUNCE_SPEED);
                FUN_10065b10(index, POUNCE_WHIRL_SOUND, 0, 0);
            }
            else if (frame < whirlStart + POUNCE_WHIRL_HALF)
                DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].targetHeading + POUNCE_WHIRL_TURN;
            else if (frame < whirlStart + POUNCE_WHIRL_END)
            {
                DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].targetHeading - POUNCE_WHIRL_TURN;
                if (frame == whirlStart + POUNCE_WHIRL_ARM)
                    motion = 4;
            }
            else
            {
                if (frame == whirlStart + POUNCE_WHIRL_END)
                {
                    Vec4 launch;
                    launch.z = POUNCE_SHOT_HEIGHT;
                    DAT_10db6ee8[index].AddMotionSpeed(POUNCE_LEAP_SPEED);
                    DAT_1020b430[index].turnRate = FUN_1013f880() % POUNCE_WHIRL_ROLL;
                    DAT_1020b430[index].leapAngle =
                        (int)((POUNCE_PI - DAT_10db6ee8[index].targetHeading) * POUNCE_FIXED_POINT);
                    FUN_10065b10(index, POUNCE_WHIRL_SOUND, 0, 0);
                }
                else if (frame < whirlStart + POUNCE_WHIRL_LEAP_END)
                {
                    Vec4 move;
                    amount = 0.0f;
                    move.Fill(amount);
                    amount = (float)DAT_1020b430[index].leapAngle;
                    move.y = POUNCE_LEAP_DRIFT;
                    move.RotateZ(amount / POUNCE_FIXED_POINT);
                    amount = (float)DAT_1020b430[index].turnRate;
                    DAT_10db6ee8[index].targetHeading = amount / POUNCE_FIXED_POINT + DAT_10db6ee8[index].targetHeading;
                    if (DAT_1020b430[index].overPit == 1)
                    {
                        float lift = DAT_10db6ee8[index].move.z + POUNCE_LEAP_LIFT;
                        DAT_10db6ee8[index].move.x = move.x;
                        DAT_10db6ee8[index].move.y = move.y;
                        DAT_10db6ee8[index].move.z = lift;
                        if (lift + self.z > DAT_104bfe24 - POUNCE_LANDING_LIMIT)
                        {
                            DAT_10db6ee8[index].AddMotionSpeed(POUNCE_LAND_SPEED);
                            DAT_1020b430[index].overPit = 0;
                            DAT_10db6ee8[index].mode = 1;
                            FUN_10065b10(index, POUNCE_WHIRL_SOUND, 0, 0);
                        }
                    }
                    else if (DAT_104bfd08 == 0 && DAT_10db6ee8[index].m_238c == 0)
                    {
                        FUN_100241e0(index, move.x, move.y, 0.0f);
                        if (DAT_10db6ee8[index].verticalSpeed > 0.0f)
                            DAT_10db6ee8[index].verticalSpeed += POUNCE_RISE;
                        else
                        {
                            DAT_10db6ee8[index].m_237c -= POUNCE_GRAVITY_Z;
                            DAT_10db6ee8[index].verticalSpeed += POUNCE_RISE;
                        }
                    }
                }
                else
                {
                    amount = (float)DAT_1020b430[index].turnRate;
                    DAT_1020b430[index].state = 0;
                    DAT_1020b430[index].pounceStep = 0;
                    DAT_1020b430[index].stepFrames = 0;
                    DAT_1020b430[index].leapAngle = 0;
                    DAT_1020b430[index].turnRate = 0;
                    motion = 2;
                    DAT_10db6ee8[index].targetHeading = amount / POUNCE_FIXED_POINT;
                }
            }
        }
    }
    else if (state == POUNCE_STATE_ATTACK)
    {
        DAT_104483e8[index] = -1;
        if (step == POUNCE_STEP_ROAR)
        {
            if (DAT_1020b430[index].stepFrames == POUNCE_ROAR_FRAMES)
            {
                DAT_1020b430[index].pounceStep = POUNCE_STEP_CHASE;
                DAT_1020b430[index].stepFrames = 0;
            }
            else if (DAT_1020b430[index].stepFrames == 0)
            {
                FUN_10065b10(index, POUNCE_ROAR_SOUND, 0, 0);
                FUN_10065b10(index, POUNCE_WHIRL_SOUND, 0, 0);
            }
            DAT_1020b430[index].stepFrames++;
            amount = 0.0f;
            DAT_10db6ee8[index].velocity.Fill(amount);
            DAT_10db6ee8[index].PushMotionSpeed(POUNCE_ROAR_SPEED);
            motion = 1;
        }
        else if (step == POUNCE_STEP_CHASE)
        {
            Vec4 move;
            amount = (float)atan2((double)diff.x, (double)diff.y);
            int frame = DAT_1020b430[index].stepFrames;
            int overGround = DAT_10db6ee8[index].m_238c;
            DAT_10db6ee8[index].targetHeading = amount + POUNCE_PI;
            DAT_10218fc0[index] = 1;
            DAT_1020b430[index].stepFrames = frame + 1;
            if (overGround == 1)
            {
                Vec4 spareA;
                Vec4 spareB;
                Vec4 spareC;
                amount = 0.0f;
                spareA.Fill(amount);
                spareA.z = spareA.z + POUNCE_PIT_CLIMB;
                amount = 0.0f;
                move.Fill(amount);
                if (distance > POUNCE_CHASE_RANGE)
                    move.y = POUNCE_CHASE_RANGE;
                else
                    move.y = distance;
                amount = POUNCE_CHASE_SCALE;
                move.Copy(move * amount);
                int blocked;
                if (DAT_10db6ee8[index].IsClearOfMapLimited(&move) == 0 || DAT_10db6ee8[index].IsClearAny(&move) == 0)
                    blocked = 1;
                else
                    blocked = 0;
                amount = POUNCE_HALF;
                move.Copy(move * amount);
                if (blocked == 1)
                {
                    if (DAT_1020b430[index].overPit == 0 || DAT_104bfe24 - POUNCE_PIT_CLIMB < self.z)
                    {
                        Vec4 launch;
                        launch.z = POUNCE_SHOT_HEIGHT;
                        DAT_10db6ee8[index].mode = 1;
                        DAT_10db6ee8[index].AddMotionSpeed(POUNCE_JUMP_LIFT);
                        FUN_10065b10(index, POUNCE_WHIRL_SOUND, 0, 0);
                    }
                    else if (DAT_1020b430[index].overPit == 1)
                        DAT_10db6ee8[index].move.z = DAT_10db6ee8[index].move.z + POUNCE_PIT_CLIMB;
                }
            }
            amount = 0.0f;
            move.Fill(amount);
            move.y = POUNCE_CHASE_SPEED;
            if (DAT_10db6ee8[index].m_238c != 0)
                move.y = POUNCE_CHASE_SPEED_PIT;
            move.RotateZ(POUNCE_PI - DAT_10db6ee8[index].heading);
            DAT_10db6ee8[index].move.x = move.x + DAT_10db6ee8[index].move.x;
            DAT_10db6ee8[index].move.y = move.y + DAT_10db6ee8[index].move.y;
            if (DAT_1020b430[index].overPit == 1)
                DAT_10db6ee8[index].move.z = DAT_10db6ee8[index].move.z - POUNCE_PIT_PULL;
            motion = 1;
            if (!((DAT_10802bf8[index].position - self).Length() < DAT_10db6ee8[index].speedScale) ||
                DAT_1020b430[index].sightFrames >= POUNCE_STALLED_FRAMES)
            {
                if (distance < POUNCE_CLOSE && FUN_1013f880() % POUNCE_ATTACK_ROLL < POUNCE_ATTACK_CHANCE)
                {
                    DAT_1020b430[index].pounceStep = POUNCE_STEP_POUNCE;
                    DAT_1020b430[index].stepFrames = 0;
                }
                else if (distance < POUNCE_CLOSE && FUN_1013f880() % POUNCE_ATTACK_ROLL < POUNCE_ATTACK_CHANCE)
                {
                    DAT_1020b430[index].pounceStep = POUNCE_STEP_BREATHE;
                    DAT_1020b430[index].stepFrames = 0;
                }
            }
            else
            {
                DAT_1020b430[index].state = 0;
                DAT_1020b430[index].pounceStep = 0;
                DAT_1020b430[index].stepFrames = 0;
                DAT_1020b430[index].noticed = 0;
            }
        }
        else if (step == POUNCE_STEP_POUNCE)
        {
            if (DAT_10db6ee8[index].m_238c == 0)
            {
                if (DAT_10db6ee8[index].verticalSpeed <= 0.0f)
                    DAT_10db6ee8[index].m_237c -= POUNCE_POUNCE_GRAVITY_Z;
                DAT_10db6ee8[index].verticalSpeed += POUNCE_POUNCE_RISE;
            }
            DAT_10218fc0[index] = 1;
            if (DAT_1020b430[index].stepFrames == 0 || DAT_1020b430[index].stepFrames == POUNCE_POUNCE_CAST)
            {
                Vec4 probe;
                probe.z = POUNCE_SHOT_HEIGHT;
                if (DAT_10db6ee8[index].IsClearOfMapLimited(&probe) == 0)
                {
                    Vec4 target = DAT_10db6ee8[PLAYER_SLOT].matrix.GetTranslation();
                    target.z = target.z + POUNCE_TARGET_LIFT;
                    if ((DAT_10868838[index][0].GetTranslation() - target).LengthSquared3() > POUNCE_SHOT_SPACING_SQUARED)
                    {
                        EffectOptions options;
                        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                        options.m_0000 = 0;
                        FUN_1001d8b0(index, POUNCE_SHOT_EFFECT, target, -1, -1, 0, 0, 0, 0, &options);
                        DAT_10868838[index][0]._41 = target.x;
                        DAT_10868838[index][0]._42 = target.y;
                        DAT_10868838[index][0]._43 = target.z;
                    }
                }
                amount = atan2f(diff.x, diff.y) + POUNCE_PI;
                DAT_10db6ee8[index].heading = amount;
                DAT_10db6ee8[index].targetHeading = amount;
                DAT_10db6ee8[index].mode = 1;
                DAT_10db6ee8[index].AddMotionSpeed(POUNCE_POUNCE_JUMP_SPEED);
                motion = POUNCE_POUNCE_MOTION;
                amount = 0.0f;
                DAT_10db6ee8[index].velocity.Fill(amount);
                FUN_10065b10(index, POUNCE_POUNCE_SOUND, 0, 0);
                Matrix4 dustMatrix;
                Vec4 dustPosition;
                dustMatrix = RotationZ(DAT_10db6ee8[index].heading);
                EffectOptions dustOptions;
                dustOptions.owner = index;
                FUN_1001d8b0(index, POUNCE_DUST_EFFECT, dustPosition, -1, -1,
                             &DAT_1021bdf8[index][1]->worldMatrix, 0, 0, &dustMatrix, &dustOptions);
            }
            int frame = DAT_1020b430[index].stepFrames;
            if ((frame > POUNCE_HIT_FROM && frame < POUNCE_HIT_TO) || (frame > POUNCE_HIT2_FROM && frame < POUNCE_HIT2_TO))
                DAT_1021f6c0[index] = 1;
            frame++;
            DAT_1020b430[index].stepFrames = frame;
            if (frame == POUNCE_POUNCE_END)
            {
                if (DAT_1030b8f8[index] != 0 || diff.LengthSquared3() < POUNCE_WAKE_RANGE_SQUARED)
                {
                    DAT_1020b430[index].state = POUNCE_STATE_ATTACK;
                    DAT_1020b430[index].pounceStep = 0;
                    DAT_1020b430[index].stepFrames = 0;
                }
                else
                {
                    DAT_1020b430[index].state = 0;
                    DAT_1020b430[index].pounceStep = 0;
                    DAT_1020b430[index].stepFrames = 0;
                    DAT_1020b430[index].noticed = 0;
                }
            }
            else if (frame == POUNCE_POUNCE_RECOVER)
                motion = 2;
            if (DAT_1020b430[index].stepFrames < POUNCE_POUNCE_RECOVER)
            {
                Vec4 move;
                amount = 0.0f;
                move.Fill(amount);
                if (DAT_10db6ee8[index].m_238c == 1)
                {
                    move.y = POUNCE_POUNCE_SPEED;
                    move.RotateZ(POUNCE_PI - DAT_10db6ee8[index].heading);
                    DAT_10802bf8[index].step.Copy(move);
                }
                else
                {
                    move.y = POUNCE_POUNCE_SPEED_GROUND;
                    move.RotateZ(POUNCE_PI - DAT_10db6ee8[index].heading);
                    amount = POUNCE_HALF;
                    DAT_10802bf8[index].step.Copy(move * amount);
                }
                DAT_10802bf8[index].step.z = 0.0f;
                if (DAT_104bfd08 == 0)
                    FUN_100241e0(index, move.x, move.y, move.z);
            }
            else
            {
                DAT_10218fc0[index] = 0;
                amount = POUNCE_POUNCE_SLOW;
                DAT_10802bf8[index].step.Copy(DAT_10802bf8[index].step * amount);
                if (DAT_104bfd08 == 0)
                    FUN_100241e0(index, DAT_10802bf8[index].step.x, DAT_10802bf8[index].step.y,
                                 DAT_10802bf8[index].step.z);
                motion = 2;
            }
        }
        else if (step == POUNCE_STEP_BREATHE)
        {
            int frame = ++DAT_1020b430[index].stepFrames;
            if (frame == 1)
                DAT_10db6ee8[index].PlayMotion(POUNCE_BREATH_MOTION, 1, 15, POUNCE_MOTION_SPEED);
            else if (frame < POUNCE_BREATH_END)
                DAT_10db6ee8[index].PlayMotion(POUNCE_BREATH_MOTION, 0, POUNCE_BLEND, POUNCE_MOTION_SPEED);
            if (DAT_1020b430[index].stepFrames < POUNCE_BREATH_AIM_FRAMES)
                DAT_10db6ee8[index].targetHeading =
                    atan2f(player.x - self.x, player.y - self.y) + POUNCE_PI;
            frame = DAT_1020b430[index].stepFrames;
            if (frame > POUNCE_BREATH_AIM_FRAMES && frame < POUNCE_BREATH_END)
            {
                if (frame % POUNCE_BREATH_SOUND_PERIOD == 0)
                    FUN_10065b10(index, POUNCE_BREATH_SOUND, 0, 0);
                Vec4 move;
                move.y = POUNCE_BREATH_BACK;
                move.RotateZ(-DAT_10db6ee8[index].heading);
                move.z = POUNCE_BREATH_PROBE;
                if (DAT_10db6ee8[index].m_238c == 0 || DAT_10db6ee8[index].IsClearOfMapLimited(&move) == 0)
                    DAT_1020b430[index].stepFrames = POUNCE_BREATH_LAST;
                move.z = 0.0f;
                if (DAT_104bfd08 == 0)
                    FUN_100241e0(index, move.x, move.y, 0.0f);
                EffectOptions options;
                Matrix4 mouthMatrix;
                Vec4 mouthPosition;
                options.flags |= 2;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0008 = POUNCE_BREATH_SCALE;
                options.m_000e = POUNCE_BREATH_FLAG;
                mouthMatrix = RotationZ(DAT_10db6ee8[index].heading);
                if (DAT_1021bdf8[index][2] != 0)
                {
                    mouthMatrix.SetTranslation(DAT_1021bdf8[index][2]->worldMatrix.GetTranslation());
                    if (DAT_1020b430[index].stepFrames % POUNCE_FIRE_PERIOD == 0)
                        FUN_1001d8b0(index, POUNCE_FIRE_EFFECT, mouthPosition, -1, -1, 0, 0, 0, &mouthMatrix,
                                     &options);
                    FUN_1001d8b0(index, POUNCE_SMOKE_EFFECT, mouthPosition, -1, -1, 0, 0, 0, &mouthMatrix, &options);
                }
            }
            else if (frame >= POUNCE_BREATH_END)
            {
                DAT_10db6ee8[index].PlayMotion(2, 0, POUNCE_BLEND, POUNCE_MOTION_SPEED);
                if (DAT_1020b430[index].stepFrames > POUNCE_BREATH_RELEASE)
                {
                    if (DAT_1030b8f8[index] != 0 || diff.LengthSquared3() < POUNCE_WAKE_RANGE_SQUARED)
                    {
                        DAT_1020b430[index].pounceStep = 0;
                        DAT_1020b430[index].stepFrames = 0;
                        DAT_1020b430[index].state = POUNCE_STATE_ATTACK;
                    }
                    else
                    {
                        DAT_1020b430[index].state = 0;
                        DAT_1020b430[index].pounceStep = 0;
                        DAT_1020b430[index].stepFrames = 0;
                        DAT_1020b430[index].noticed = 0;
                    }
                }
            }
        }
    }
    if (DAT_1020b430[index].overPit == 1)
    {
        DAT_10db6ee8[index].move.x = DAT_10db6ee8[index].move.x * POUNCE_FALL_DAMP;
        DAT_10db6ee8[index].move.y = DAT_10db6ee8[index].move.y * POUNCE_FALL_DAMP;
        if (DAT_104bfe24 > self.z)
            DAT_10db6ee8[index].move.z = DAT_10db6ee8[index].move.z + POUNCE_FALL_PULL_LOW;
        else if (DAT_104bfe24 + POUNCE_PIT_MARGIN > self.z)
            DAT_10db6ee8[index].move.z = DAT_10db6ee8[index].move.z + POUNCE_FALL_PULL_NEAR;
    }
    if (DAT_10db6ee8[index].m_238c == 0)
    {
        DAT_10db6ee8[index].move.x = DAT_10db6ee8[index].move.x * POUNCE_GROUND_DAMP;
        DAT_10db6ee8[index].move.y = DAT_10db6ee8[index].move.y * POUNCE_GROUND_DAMP;
    }
    DAT_10802bf8[index].position.Copy(self);
    DAT_10db6ee8[index].move.x = DAT_10db6ee8[index].move.x * DAT_10db6ee8[index].speedScale;
    DAT_10db6ee8[index].move.y = DAT_10db6ee8[index].speedScale * DAT_10db6ee8[index].move.y;
    if (motion >= 0)
        DAT_10db6ee8[index].PlayMotion(motion, 0, POUNCE_BLEND, POUNCE_MOTION_SPEED);
    return 0;
}
