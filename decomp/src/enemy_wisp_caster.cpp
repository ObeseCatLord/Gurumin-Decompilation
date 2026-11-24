// The wisp caster: an enemy of kind 0x10 that hides until it is hit, then stalks the player, dashes at him and
// lunges, and casts flame effects while two wisps drift about it (0x10071ee0, a per-kind behaviour of 0x100f9670).
// See the comment above Enemy_WispCaster for the states.

#include <math.h>
#include <stdlib.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "effect_manager.h"

#include "anim_model.h"

#include "char_state.h"

#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "last_position.h"
#include "map_vars.h"
#include "texture_manager.h"
#include "world.h"

#include "doppler_loops.h"
#include "enemy_chase_hop.h"
#include "hop_spit_enemy.h"
#include "target_race.h"
#include "wisp_caster_enemy.h"

Matrix4 RotationZ(float angle);   // 0x101b1b40
Matrix4 RotationX(float angle);   // 0x101b1ad0

#define WISP_SPINE_NODE "Bip01 Spine"
#define WISP_WEAPON_NODE "@EN_016ar.lwo"
#define WISP_REVEAL_MODEL "EN_016AB.it3"
#define WISP_SLOT_VARIANT_STAT 9            // CharSlot::stats[9] / 1000 is non-zero when it starts revealed
#define WISP_NODE_STATE 0x2d
#define WISP_NODE_STATE_STEP 0x2e
#define WISP_NODE_STATE_BLEND 0x2f
#define WISP_NODE_STATE_END 0x2b
#define WISP_NODE_MASK_A 0x400
#define WISP_NODE_MASK_B 0x200
#define WISP_WEAPON_SHOWN 0x20
#define WISP_SCALE 0.4f
#define WISP_PI 3.1415f
#define WISP_TWO_PI 6.283f
#define WISP_THIRTY_DEGREES 0.5236f
#define WISP_QUARTER_TURN 1.5708f
#define WISP_DEGREE 0.01745329f
#define WISP_COUNT 2                        // the wisps are DAT_10868838[index][0..1]
#define WISP_ALIVE_FROM 1.0f                // wisp._14 counts the frames it has burned (0 = not lit)
#define WISP_LIFETIME 251.0f
#define WISP_SOUND_PERIOD 120
#define WISP_SOUND_YOUNG 200.0f
#define WISP_SOUND_RESIDUE 2
#define WISP_RETARGET_ROLL 30
#define WISP_STEER_ROLL 5
#define WISP_STEER_JITTER 30
#define WISP_STEER_JITTER_HALF 15
#define WISP_STEER_SPEED (-20.0f)
#define WISP_DRIFT_STEP 0.5f
#define WISP_SMOKE_ROLL 5
#define WISP_SMOKE_RANGE 500
#define WISP_SMOKE_OFFSET 600.0f
#define WISP_SMOKE_TURN_RANGE 360
#define WISP_SMOKE_LIFT 5.0f
#define WISP_SMOKE_EFFECT 0x679
#define WISP_SOUND_FLICKER 0x2c1
#define WISP_SOUND_CRACKLE 0x2c2
#define WISP_SOUND_CRACKLE_ROLL 50
#define WISP_SOUND_CRACKLE_MATCH 5
#define WISP_SOUND_CRACKLE_CHANCE 30
#define WISP_SOUND_REVEAL 0x2c3
#define WISP_SOUND_VOLUME 1.0f
#define WISP_SLOT_PLAYER 0
#define WISP_FIRST_FAR_SLOT 0x3a            // slots below this are heard however far they are
#define WISP_HEAR_RANGE_SQUARED 6250000.0f
#define WISP_REVEAL_FRAMES 0x1f
#define WISP_REVEAL_DELAY 2
#define WISP_REVEAL_FIRST_STATE 2
#define WISP_STATE_STALK 0                  // faces the player, walks at him and picks the next state
#define WISP_STATE_SWIPE 1                  // swipes with its hands (hit frame 0x41) and shoves forward
#define WISP_STATE_DASH 2                   // dashes at the player trailing flames and may dash again
#define WISP_STATE_LUNGE 3                  // lunges forward in a burst of effect 0x655
#define WISP_STATE_SWIPE_LOW 4              // the swipe of state 1 with the other motion (hit frame 0xd2)
#define WISP_STATE_CHARGE 5                 // charges a blast, then drives back and leaves a burst of effect 0x5ea
#define WISP_STATE_RITUAL 6                 // stands and conjures bursts of effect 0x259 on frames 0xba and 0xbd
#define WISP_STATE_SUMMON 7                 // lights its wisps once it is badly hurt
#define WISP_FAR_RANGE 800.0f
#define WISP_NEAR_RANGE 400.0f
#define WISP_UNREVEALED_SWIPE_CHANCE 75
#define WISP_UNREVEALED_DASH_CHANCE 30
#define WISP_RITUAL_CHANCE 20
#define WISP_CHARGE_CHANCE 30
#define WISP_SUMMON_ROLL 1000
#define WISP_SUMMON_CHANCE 220
#define WISP_LUNGE_CHANCE 25
#define WISP_NEAR_SWIPE_CHANCE 12
#define WISP_NEAR_SWIPE_LOW_CHANCE 18
#define WISP_NEAR_DASH_CHANCE 20
#define WISP_STALK_SPEED (-6.0f)
#define WISP_SWIPE_HIT_FRAME 0x41
#define WISP_SWIPE_LOW_HIT_FRAME 0xd2
#define WISP_SWIPE_SHOVE_FROM 0x3e          // the shove covers the frames above this and below the next one
#define WISP_SWIPE_SHOVE_TO 0x42
#define WISP_SWIPE_LOW_SHOVE_FROM 0xce
#define WISP_SWIPE_LOW_SHOVE_TO 0xd3
#define WISP_SWIPE_SHOVE_SPEED (-36.0f)
#define WISP_SWIPE_LOW_SHOVE_SPEED 36.0f
#define WISP_SWIPE_DAMP 0.2f
#define WISP_SWIPE_SETTLE 0x1e
#define WISP_SWIPE_END 0x26
#define WISP_SWIPE_HIT_SPEED 35.0f
#define WISP_SWIPE_HIT_BLEND 0x12
#define WISP_SWIPE_HIT_SOUND 0x2c4
#define WISP_SWIPE_BLEND_FRAMES 0x19
#define WISP_SWIPE_LIFT_SCALE 25.0f
#define WISP_SWIPE_LIFT 35.0f
#define WISP_MOTION_IDLE 2
#define WISP_MOTION_SWIPE 4
#define WISP_MOTION_SWIPE_FOLLOW 7
#define WISP_MOTION_SWIPE_LOW 16
#define WISP_MOTION_SWIPE_LOW_FOLLOW 19
#define WISP_MOTION_DASH_WINDUP 11
#define WISP_MOTION_DASH 13
#define WISP_MOTION_LUNGE 8
#define WISP_MOTION_LUNGE_END 10
#define WISP_MOTION_RITUAL 15
#define WISP_DASH_LAUNCH_FRAME 0xaf
#define WISP_DASH_SPEED (-48.0f)
#define WISP_DASH_REACH 5.0f
#define WISP_DASH_TRAIL_SCALE 4.0f
#define WISP_DASH_RECOVER_FRAMES 13
#define WISP_DASH_FAR 100.0f
#define WISP_DASH_REPEAT_STEP 15
#define WISP_DASH_SPRAY_EFFECT 0x695
#define WISP_DASH_PUFF_EFFECT 0x699
#define WISP_DASH_PUFF_PERIOD 3
#define WISP_DASH_FLAME_EFFECT 0x60f
#define WISP_DASH_FLAME_LIFT 10.0f
#define WISP_DASH_FLAME_TILT_RANGE 60
#define WISP_DASH_FLAME_TILT_HALF 30
#define WISP_DASH_SOUND_PERIOD 5
#define WISP_DASH_SOUND 0x2c5
#define WISP_DASH_RESET (-1)
#define WISP_LUNGE_SPEED (-24.0f)
#define WISP_LUNGE_SCALE 3.0f
#define WISP_LUNGE_EFFECT 0x655
#define WISP_LUNGE_BOOST_FRAME 6
#define WISP_LUNGE_BOOST 30.0f
#define WISP_LUNGE_SOUND 0x2c6
#define WISP_LUNGE_HIT_FROM 99
#define WISP_LUNGE_HIT_TO 0x74
#define WISP_LUNGE_SLIDE_END 0x37
#define WISP_LUNGE_SLIDE_FRAMES 60.0f
#define WISP_LUNGE_PAUSE_FRAME 0x14
#define WISP_LUNGE_REPEATS 2
#define WISP_LUNGE_RESUME_FRAME 7
#define WISP_CHARGE_LAUNCH_SPEED (-48.0f)
#define WISP_CHARGE_MOTION_FRAME_STEP 0x1e
#define WISP_CHARGE_SPARK_EFFECT 0x696
#define WISP_CHARGE_HIT_FRAME 100
#define WISP_CHARGE_DAMP 0.5f
#define WISP_BURST_SCALE 2.0f
#define WISP_BURST_EFFECT 0x5ea
#define WISP_BURST_COUNT 15
#define WISP_BURST_SOUND 0x2c7
#define WISP_BURST_CAPTION "829182"
#define WISP_BURST_NODE_REACH 4.0f
#define WISP_CHARGE_FLAME_ROLL 3
#define WISP_CHARGE_FLAME_FROM 10
#define WISP_CHARGE_FLAME_LIFT 100.0f
#define WISP_CHARGE_FLAME_TILT_RANGE 60
#define WISP_CHARGE_SPLASH_SCALE 1.2f
#define WISP_CHARGE_SPLASH_EFFECT 0x2d4
#define WISP_CHARGE_SPLASH_RANGE 75
#define WISP_CHARGE_SPLASH_HALF 37.5f
#define WISP_CHARGE_SOUND_A_PERIOD 10
#define WISP_CHARGE_SOUND_A 0x2c8
#define WISP_CHARGE_SOUND_B_PERIOD 0x15
#define WISP_CHARGE_SOUND_B_OFFSET 2
#define WISP_CHARGE_SOUND_B 0x2c9
#define WISP_RECOIL_BACK (-0.2f)
#define WISP_RECOIL_FRAMES 0xf
#define WISP_RECOIL_END 0x1e
#define WISP_RECOIL_LATE 7
#define WISP_RECOIL_HIT_FRAME 0x72
#define WISP_RECOIL_NODE_REACH 3.0f
#define WISP_RITUAL_PERIOD 30
#define WISP_RITUAL_POSE_FRAMES 15
#define WISP_RITUAL_POSE_END 90
#define WISP_RITUAL_BURST_FRAME 0xba
#define WISP_RITUAL_SHOT_FRAME 0xbd
#define WISP_RITUAL_END 0x78
#define WISP_RITUAL_DISTANCE (-100.0f)
#define WISP_RITUAL_HEIGHT 100.0f
#define WISP_RITUAL_SCALE 2.5f
#define WISP_RITUAL_EFFECT 0x259
#define WISP_RITUAL_EFFECT_LAST 0x25b
#define WISP_RITUAL_EFFECT_COUNT 9
#define WISP_RITUAL_SHOT_EFFECT 0x68c
#define WISP_RITUAL_SHOT_SCALE 1.5f
#define WISP_RITUAL_SHOT_SOUND 0x2ca
#define WISP_SUMMON_END 300
#define WISP_SUMMON_SPAWN_FRAME 0xf
#define WISP_SUMMON_FLASH_FRAME 1
#define WISP_SUMMON_FLASH_EFFECT 0x3f1
#define WISP_SUMMON_FLASH_DISTANCE (-50.0f)
#define WISP_SUMMON_FLASH_HEIGHT 50.0f
#define WISP_SUMMON_SOUND 0x2cb
#define WISP_SUMMON_DISTANCE (-200.0f)
#define WISP_SUMMON_VELOCITY (-20.0f)
#define WISP_SUMMON_EFFECT 0x690
#define WISP_SUMMON_SCALE 2.0f
#define WISP_SUMMON_TWIN_DIVISOR 2
#define WISP_SLIDE_TURN_LIMIT 1.0f
#define WISP_SLIDE_TURN_SET 10.0f
#define WISP_SLIDE_SMOOTH 0.8f
#define WISP_SLIDE_NEW 0.2f
#define WISP_TRAIL_FALLOFF 2
#define WISP_MOTION_SPEED 1.0f
#define WISP_BLEND 5
#define WISP_MOTION_RESTORE_BLEND 3
#define WISP_STAT_STAGE 1

// The original expands none of math.h's inline float functions in this function.
#pragma inline_depth(0)

// 0x10071ee0 (Enemy_WispCaster). Every frame the two wisps of the slot (DAT_10868838[index][0..1], their matrices
// hold a life counter in _14, a velocity in _11/_12, the target velocity in _21/_22 and an offset in _41/_42) drift
// towards the player, and some smoke effects start at random spots. The character stays held until it is hit
// (state 0 rolls only after it has shown itself) and its weapon appears two frames after the flash. State 0 turns to
// the player and walks at 6 units a frame; at the far range it rolls the next state: unrevealed it swipes (1) or
// dashes (2), revealed it conjures (6), charges (5) or, with under three quarters of its health, summons (7); near the
// player it lunges (3), swipes (1, 4) or dashes (2). A swipe shoves forward on its hit frames and fades the shove out,
// a dash launches on frame 0xaf at 48 units a frame, trailing effects 0x695, 0x699 and 0x60f, and dashes again until
// a roll ends it, a lunge crouches, springs and fades out, a charge drives forward spraying effect 0x696 and ends in
// a burst of effect 0x5ea, the ritual conjures bursts of effect 0x259 on frames 0xba and 0xbd, and the summon lights
// one wisp (two with under a quarter of its health) after 15 frames. The move is slid along walls (the sign of
// DAT_10255160[index][0] says which way) and smoothed with the last frame's, and the motion is set last. Returns 1
// while the slot is flagged gone or nothing is wanted of it, 0 otherwise.
int Enemy_WispCaster(int index)
{
    Vec4 player;
    DAT_10db6ee8[index].root->M_10194730(WISP_NODE_STATE, WISP_NODE_MASK_A);
    DAT_10db6ee8[index].root->M_10194730(WISP_NODE_STATE, WISP_NODE_MASK_B);
    DAT_10db6ee8[index].uniformScale = WISP_SCALE;
    if (DAT_104c0110 == 0)
        DAT_104c0110 = index;
    if (DAT_1020b430[index].revealed == 0)
        DAT_10daaae8[index].held = 1;
    player.Copy(DAT_10db6ee8[WISP_SLOT_PLAYER].matrix.GetTranslation());
    DAT_1020b430[index].wispsActive = 0;
    for (int wispIndex = 0; wispIndex < WISP_COUNT; wispIndex++)
    {
        Matrix4 &wisp = DAT_10868838[index][wispIndex];
        Vec4 aim;
        if (wisp._14 >= WISP_ALIVE_FROM)
        {
            DAT_1020b430[index].wispsActive = 1;
            wisp._14 = wisp._14 + 1.0f;
            if (rand() % WISP_RETARGET_ROLL != 0)
            {
                float velocityHeading = (float)atan2(wisp._11, wisp._12) + WISP_PI;
                float playerHeading = (float)atan2(player.x - wisp._41, player.y - wisp._42) + WISP_PI;
                for (; velocityHeading < -WISP_PI; velocityHeading = velocityHeading + WISP_TWO_PI)
                {
                }
                for (; WISP_PI < velocityHeading; velocityHeading = velocityHeading - WISP_TWO_PI)
                {
                }
                for (; playerHeading < -WISP_PI; playerHeading = playerHeading + WISP_TWO_PI)
                {
                }
                for (; WISP_PI < playerHeading; playerHeading = playerHeading - WISP_TWO_PI)
                {
                }
                float weight = (float)(FUN_1013f880() % 5 + 1) / 5.0f;
                float gap = (float)fabs(velocityHeading - playerHeading);
                if (!(gap < WISP_PI))
                {
                    if (velocityHeading <= 0.0f)
                        velocityHeading = velocityHeading + WISP_TWO_PI;
                    else
                        velocityHeading = velocityHeading - WISP_TWO_PI;
                }
                velocityHeading = weight * velocityHeading + (1.0f - weight) * playerHeading;
                velocityHeading = (float)(FUN_1013f880() % WISP_STEER_JITTER - WISP_STEER_JITTER_HALF) * WISP_DEGREE +
                                  velocityHeading;
                aim.Fill(0.0f);
                aim.y = WISP_STEER_SPEED;
                aim.RotateZ(-velocityHeading);
                wisp._21 = aim.x;
                wisp._22 = aim.y;
            }
            if (wisp._14 <= WISP_LIFETIME)
            {
                if (rand() % WISP_SMOKE_ROLL == 0 && FUN_1013f880() % 2 != 0)
                {
                    Vec4 smoke;
                    Matrix4 smokeMatrix;
                    EffectOptions smokeOptions;
                    smoke.Fill(0.0f);
                    smoke.y = (float)(FUN_1013f880() % WISP_SMOKE_RANGE) - WISP_SMOKE_OFFSET;
                    smoke.RotateZ((float)(FUN_1013f880() % WISP_SMOKE_TURN_RANGE) * WISP_DEGREE);
                    smoke.Copy(smoke + wisp.GetTranslation());
                    smoke.z = smoke.z + WISP_SMOKE_LIFT;
                    smokeMatrix.SetIdentity();
                    smokeMatrix.SetTranslation(smoke);
                    smoke.Fill(0.0f);
                    smokeOptions.m_0000 = 0;
                    smokeOptions.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                    smokeOptions.m_0008 = 1.0f;
                    DAT_10daaa80.Start(WISP_SMOKE_EFFECT, smoke, -1, -1, 0, 0, 0, &smokeMatrix, &smokeOptions,
                                       &DAT_10219770[index]);
                }
                if (wisp._11 <= wisp._21)
                {
                    if (wisp._11 < wisp._21)
                    {
                        wisp._11 = wisp._11 + WISP_DRIFT_STEP;
                        if (wisp._21 < wisp._11)
                            wisp._11 = wisp._21;
                    }
                }
                else
                {
                    wisp._11 = wisp._11 - WISP_DRIFT_STEP;
                    if (wisp._11 < wisp._21)
                        wisp._11 = wisp._21;
                }
                if (wisp._12 <= wisp._22)
                {
                    if (wisp._12 < wisp._22)
                    {
                        wisp._12 = wisp._12 + WISP_DRIFT_STEP;
                        if (wisp._22 < wisp._12)
                            wisp._12 = wisp._22;
                    }
                }
                else
                {
                    wisp._12 = wisp._12 - WISP_DRIFT_STEP;
                    if (wisp._12 < wisp._22)
                        wisp._12 = wisp._22;
                }
                wisp._41 = wisp._11 + wisp._41;
                wisp._42 = wisp._12 + wisp._42;
            }
            else
                wisp.SetIdentity();
        }
    }
    if (DAT_10868838[index][0]._14 > 0.9f)
    {
        if (rand() % WISP_SOUND_PERIOD == WISP_SOUND_RESIDUE && DAT_10868838[index][0]._14 < WISP_SOUND_YOUNG)
            FUN_1003ad20(WISP_SOUND_FLICKER, &(const Vec4 &)DAT_10868838[index][0].GetTranslation(), WISP_SOUND_VOLUME);
        if (rand() % WISP_SOUND_CRACKLE_ROLL == WISP_SOUND_CRACKLE_MATCH ||
            FUN_1013f880() % WISP_SOUND_CRACKLE_CHANCE == 0)
            FUN_1003ad20(WISP_SOUND_CRACKLE, &(const Vec4 &)DAT_10868838[index][0].GetTranslation(),
                         WISP_SOUND_VOLUME);
    }
    if (DAT_10210d30[index] != 0)
    {
        if (DAT_104c0110 == index)
            DAT_104c0110 = 0;
        return 1;
    }
    if (DAT_104483e8[index] == 0)
        return 1;
    if (DAT_1020b430[index].reactFrames == 0)
    {
        DAT_10daaae8[index].m_0084 = 6;
        DAT_1021bdf8[index][1] = DAT_10db6ee8[index].root->Find(WISP_SPINE_NODE, 0);
        DAT_1020b430[index].savedEffectParam = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        DAT_1020b430[index].reactFrames = 1;
        if (DAT_10daaae8[index].stats[WISP_SLOT_VARIANT_STAT] / 1000 == 0)
            DAT_10219e40[index] = (char)0xff;
        else
        {
            DAT_1020b430[index].revealed = 1;
            DAT_10daaae8[index].held = 0;
        }
    }
    if (DAT_10210b30[index] != 0)
        DAT_10210b30[index] = 0;
    DAT_10255a88[index] = 0;
    DAT_10db6ee8[index].locked = 1;
    Vec4 place;
    Vec4 diff;
    Vec4 move;
    int motion = -1;
    int motionCut = 0;
    EffectOptions options;
    Vec4 effectPos;
    Matrix4 matrix;
    move.Fill(0.0f);
    place.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    diff.Copy(player - place);
    diff.Length();
    float flatRange = diff.Length2D();
    if (DAT_1020b430[index].weaponShowDelay != 0 && --DAT_1020b430[index].weaponShowDelay == 0)
    {
        Node *weapon = DAT_10db6ee8[index].root->Find(WISP_WEAPON_NODE, 0);
        if (weapon != 0)
            weapon->SetFlags(WISP_WEAPON_SHOWN);
    }
    unsigned char level = DAT_102550e0[index];
    if (level == 1)
    {
        DAT_104c0110 = index;
        DAT_1020b430[index].reactFrames = WISP_REVEAL_FRAMES;
    }
    if (level != 0 && DAT_1020b430[index].shielded != 0)
    {
        int savedFrame = DAT_1020b430[index].savedMotionFrame;
        DAT_102550e0[index] = 0;
        DAT_104c0110 = index;
        DAT_1020b430[index].reactFrames = WISP_REVEAL_FRAMES;
        if (savedFrame < 0)
            DAT_10db6ee8[index].PlayMotion(DAT_1020b430[index].savedMotion, 1, WISP_MOTION_RESTORE_BLEND,
                                           WISP_MOTION_SPEED);
        else
        {
            DAT_10db6ee8[index].PlayMotion(DAT_1020b430[index].savedMotion, 1, 0, WISP_MOTION_SPEED);
            DAT_10db6ee8[index].motion->frame = DAT_1020b430[index].savedMotionFrame;
            DAT_10db6ee8[index].motion->m_0004 = (float)DAT_1020b430[index].savedMotionFrame;
            DAT_10db6ee8[index].motion->Advance();
        }
    }
    DAT_1020b430[index].shielded = 0;
    if (DAT_1020b430[index].revealed == 0 && DAT_1042d9e8[index] != 0)
    {
        DAT_10daaae8[index].held = 0;
        DAT_1042d9e8[index] = 0;
        DAT_1020b430[index].revealed = 1;
        FUN_1003a7d0(WISP_REVEAL_MODEL, place.x, place.y, place.z, 0, 0, 0, DAT_10db6ee8[index].heading);
        DAT_10836c70.ResetFrameTimer();
        if ((DAT_10db6ee8[index].matrix.GetTranslation() - DAT_118e6cb8).LengthSquared3() < WISP_HEAR_RANGE_SQUARED ||
            index < WISP_FIRST_FAR_SLOT)
        {
            Vec4 revealPlace;
            revealPlace.Set(DAT_10db6ee8[index].matrix._41, DAT_10db6ee8[index].matrix._42,
                            DAT_10db6ee8[index].matrix._43, 0.0f);
            FUN_1003ad20(WISP_SOUND_REVEAL, &revealPlace, WISP_SOUND_VOLUME);
        }
        DAT_1020b430[index].weaponShowDelay = WISP_REVEAL_DELAY;
    }
    DAT_10daaae8[index].m_0141 = 0;
    DAT_1021f6c0[index] = 0;
    if (DAT_1020b430[index].state == WISP_STATE_STALK)
    {
        DAT_104483e8[index] = -1;
        if (flatRange > WISP_FAR_RANGE && DAT_10db6ee8[index].m_238c != 0)
        {
            if (DAT_1020b430[index].revealed == 0)
            {
                if (FUN_1013f880() % 100 < WISP_UNREVEALED_SWIPE_CHANCE)
                {
                    DAT_1020b430[index].state = WISP_STATE_SWIPE;
                    DAT_1020b430[index].wispFrames = 0;
                }
                else if (FUN_1013f880() % 100 < WISP_UNREVEALED_DASH_CHANCE)
                {
                    DAT_1020b430[index].state = WISP_STATE_DASH;
                    DAT_1020b430[index].wispFrames = 0;
                }
            }
            else
            {
                if (FUN_1013f880() % 100 < WISP_RITUAL_CHANCE)
                {
                    DAT_1020b430[index].state = WISP_STATE_RITUAL;
                    DAT_1020b430[index].wispFrames = 0;
                }
                else if (FUN_1013f880() % 100 < WISP_CHARGE_CHANCE)
                {
                    DAT_1020b430[index].state = WISP_STATE_CHARGE;
                    DAT_1020b430[index].wispFrames = 0;
                }
                else if (!(FUN_1013f880() % WISP_SUMMON_ROLL > WISP_SUMMON_CHANCE - 1) &&
                         DAT_10daaae8[index].stats[STAT_HEALTH] <
                             DAT_10daaae8[index].stats[STAT_MAX_HEALTH] * 3 / 4)
                {
                    DAT_1020b430[index].state = WISP_STATE_SUMMON;
                    DAT_1020b430[index].wispFrames = 0;
                }
            }
        }
        DAT_10db6ee8[index].targetHeading = (float)atan2(diff.x, diff.y) + WISP_PI;
        move.Fill(0.0f);
        move.y = WISP_STALK_SPEED;
        move.RotateZ(-DAT_10db6ee8[index].heading);
        motion = 0;
        if (flatRange < WISP_NEAR_RANGE)
        {
            if (FUN_1013f880() % 100 < WISP_LUNGE_CHANCE)
            {
                DAT_1020b430[index].state = WISP_STATE_LUNGE;
                DAT_1020b430[index].wispFrames = 0;
            }
            else if (FUN_1013f880() % 100 < WISP_NEAR_SWIPE_CHANCE)
            {
                DAT_1020b430[index].state = WISP_STATE_SWIPE;
                DAT_1020b430[index].wispFrames = 0;
            }
            else
            {
                if (DAT_1020b430[index].revealed != 0)
                {
                    if (FUN_1013f880() % 100 < WISP_NEAR_SWIPE_LOW_CHANCE)
                    {
                        DAT_1020b430[index].state = WISP_STATE_SWIPE_LOW;
                        DAT_1020b430[index].wispFrames = 0;
                    }
                }
                if (DAT_1020b430[index].revealed == 0 && FUN_1013f880() % 100 < WISP_NEAR_DASH_CHANCE)
                {
                    DAT_1020b430[index].state = WISP_STATE_DASH;
                    DAT_1020b430[index].wispFrames = 0;
                }
            }
        }
    }
    else if (DAT_1020b430[index].state == WISP_STATE_SWIPE)
    {
        int frames = DAT_1020b430[index].wispFrames;
        DAT_104483e8[index] = -1;
        if (frames < WISP_SWIPE_SETTLE || DAT_10db6ee8[index].m_238c == 0 || DAT_102550e0[index] != 0)
            motion = WISP_MOTION_SWIPE;
        else
        {
            motion = WISP_MOTION_SWIPE_FOLLOW;
            motionCut = 1;
        }
        if (DAT_10db6ee8[index].motionFrame == WISP_SWIPE_HIT_FRAME && frames != 0 &&
            DAT_10db6ee8[index].m_238c != 0)
        {
            DAT_10db6ee8[index].PushMotionSpeed(WISP_SWIPE_HIT_SPEED);
            DAT_10db6ee8[index].m_229c = WISP_SWIPE_HIT_BLEND;
            FUN_10065b10(index, WISP_SWIPE_HIT_SOUND, 0, 0);
        }
        else if (DAT_10db6ee8[index].m_229c != 0)
        {
            float blended = (float)(WISP_SWIPE_BLEND_FRAMES - DAT_10db6ee8[index].m_229c);
            DAT_10db6ee8[index].verticalSpeed =
                WISP_SWIPE_LIFT - (blended * WISP_SWIPE_LIFT_SCALE) / WISP_SWIPE_LIFT_SCALE;
        }
        if (DAT_10db6ee8[index].m_238c == 0 || (DAT_10db6ee8[index].motionFrame > WISP_SWIPE_SHOVE_FROM &&
                                                 DAT_10db6ee8[index].motionFrame < WISP_SWIPE_SHOVE_TO))
        {
            move.Fill(0.0f);
            move.y = WISP_SWIPE_SHOVE_SPEED;
            move.RotateZ(-DAT_10db6ee8[index].heading);
        }
        else if (motion == WISP_MOTION_SWIPE_FOLLOW)
        {
            DAT_1020b430[index].wispFrames++;
            move.Copy(DAT_10802bf8[index].position * WISP_SWIPE_DAMP);
        }
        if (DAT_1020b430[index].wispFrames < WISP_SWIPE_SETTLE)
            DAT_1020b430[index].wispFrames++;
        if (DAT_1020b430[index].wispFrames == WISP_SWIPE_END)
        {
            DAT_1020b430[index].state = WISP_STATE_STALK;
            DAT_1020b430[index].wispFrames = 0;
        }
        FUN_1001bec0(&place, &move);
    }
    else if (DAT_1020b430[index].state == WISP_STATE_DASH)
    {
        int frames = DAT_1020b430[index].wispFrames;
        DAT_104483e8[index] = -1;
        DAT_1020b430[index].shielded = 1;
        if (frames == 0)
        {
            motion = WISP_MOTION_DASH_WINDUP;
            if (DAT_10db6ee8[index].motionFrame == WISP_DASH_LAUNCH_FRAME)
            {
                DAT_1020b430[index].wispFrames = 1;
                DAT_10802bf8[index].step.Fill(0.0f);
                DAT_10802bf8[index].step.y = WISP_DASH_SPEED;
                DAT_10802bf8[index].step.RotateZ(WISP_PI - atan2f(diff.x, diff.y));
                DAT_1020b430[index].stepCount = 1;
            }
        }
        else if (frames == 2)
        {
            DAT_1020b430[index].shielded = 1;
            int layer = DAT_10db6ee8[index].motion->layer;
            if (layer == 2 || layer == 0xb || layer == 0xc || layer == 0xe)
                motion = WISP_MOTION_DASH_WINDUP;
            else
                motion = WISP_MOTION_DASH;
            motionCut = 1;
            if (DAT_10db6ee8[index].motionFrame == WISP_DASH_LAUNCH_FRAME)
            {
                DAT_1020b430[index].wispFrames = 1;
                DAT_10802bf8[index].step.Fill(0.0f);
                DAT_10802bf8[index].step.y = WISP_DASH_SPEED;
                DAT_10802bf8[index].step.RotateZ(WISP_PI - atan2f(diff.x, diff.y));
            }
        }
        else if (frames < 0)
        {
            motion = WISP_MOTION_DASH;
            motionCut = 1;
            DAT_1020b430[index].wispFrames = frames - 1;
            if (frames - 1 == -WISP_DASH_RECOVER_FRAMES)
            {
                DAT_1020b430[index].wispFrames = 0;
                DAT_1020b430[index].state = WISP_STATE_STALK;
            }
        }
        else if (frames == 1)
        {
            motion = WISP_DASH_RECOVER_FRAMES - 1;
            DAT_1021f6c0[index] = 1;
            DAT_1020b430[index].shielded = 1;
            motionCut = 1;
            DAT_10db6ee8[index].root->M_10194730(WISP_NODE_STATE_STEP, WISP_NODE_MASK_A);
            DAT_10db6ee8[index].root->M_10194730(WISP_NODE_STATE_STEP, WISP_NODE_MASK_B);
            int dashing = 1;
            if (DAT_10db6ee8[index].IsClearOfMapLimited(&DAT_10802bf8[index].step) != 0 &&
                DAT_10db6ee8[index].IsClearAny(&DAT_10802bf8[index].step) != 0)
            {
                DAT_10db6ee8[index].SetPosition(place.x + DAT_10802bf8[index].step.x,
                                                place.y + DAT_10802bf8[index].step.y,
                                                place.z + DAT_10802bf8[index].step.z);
                if (DAT_10db6ee8[index].IsClearOfMapLimited(0) == 0 || DAT_10db6ee8[index].IsClearAny(0) == 0)
                {
                    DAT_10db6ee8[index].SetPosition(place.x, place.y, place.z);
                    dashing = 0;
                }
            }
            else
                dashing = 0;
            DAT_10db6ee8[index].root->M_10194730(WISP_NODE_STATE_BLEND, WISP_NODE_MASK_A);
            DAT_10db6ee8[index].root->M_10194730(WISP_NODE_STATE_BLEND, WISP_NODE_MASK_B);
            if (dashing == 0)
            {
                int repeats = DAT_1020b430[index].stepCount;
                DAT_1020b430[index].sprayFrames = 0;
                if (FUN_1013f880() % 100 < repeats * WISP_DASH_REPEAT_STEP || DAT_1020b430[index].revealed != 0)
                {
                    DAT_1020b430[index].stepCount = 0;
                    DAT_1020b430[index].wispFrames = WISP_DASH_RESET;
                }
                else
                {
                    DAT_1020b430[index].stepCount = repeats + 1;
                    motion = WISP_DASH_RECOVER_FRAMES;
                    DAT_1020b430[index].wispFrames = 2;
                    DAT_10802bf8[index].step.Fill(0.0f);
                    DAT_10802bf8[index].step.y = WISP_DASH_SPEED;
                    DAT_10802bf8[index].step.RotateZ(WISP_PI - atan2f(diff.x, diff.y));
                }
            }
            else
            {
                DAT_1020b430[index].sprayFrames++;
                DAT_10868838[index][2].SetTranslation(place + DAT_10802bf8[index].step * WISP_DASH_REACH);
                EffectOptions ownedOptions;
                ownedOptions.owner = index;
                for (int spray = 0; spray < 2; spray++)
                {
                    effectPos.Copy(place);
                    effectPos.x = ((float)(FUN_1013f880() % 100) + effectPos.x - 50.0f) +
                                  DAT_10802bf8[index].step.x * WISP_DASH_TRAIL_SCALE;
                    effectPos.y = ((float)(FUN_1013f880() % 100) + effectPos.y - 50.0f) +
                                  DAT_10802bf8[index].step.y * WISP_DASH_TRAIL_SCALE;
                    matrix.SetIdentity();
                    matrix.SetTranslation(effectPos);
                    effectPos.Fill(0.0f);
                    FUN_1001d8b0(index, WISP_DASH_SPRAY_EFFECT, effectPos, -1, -1, 0, 0, 0, &matrix, &ownedOptions);
                }
                if (DAT_1020b430[index].sprayFrames % WISP_DASH_PUFF_PERIOD == 0)
                {
                    matrix.SetIdentity();
                    effectPos.Fill(0.0f);
                    FUN_1001d8b0(index, WISP_DASH_PUFF_EFFECT, effectPos, -1, -1, &DAT_10868838[index][2], 0, 0,
                                 &matrix, &ownedOptions);
                }
                if (DAT_1020b430[index].sprayFrames % WISP_DASH_PUFF_PERIOD == 0 && FUN_1013f880() % 2 != 0)
                {
                    Matrix4 turn = RotationZ(DAT_10db6ee8[index].targetHeading);
                    Matrix4 tilt = RotationX(-WISP_PI - (float)(FUN_1013f880() % WISP_DASH_FLAME_TILT_RANGE -
                                                                WISP_DASH_FLAME_TILT_HALF) *
                                                           WISP_DEGREE);
                    matrix = tilt * turn;
                    effectPos.Copy(place + DAT_10802bf8[index].step);
                    matrix.SetTranslation(effectPos);
                    matrix._43 = matrix._43 + WISP_DASH_FLAME_LIFT;
                    effectPos.Fill(0.0f);
                    FUN_1001d8b0(index, WISP_DASH_FLAME_EFFECT, effectPos, -1, -1, 0, 0, 0, &matrix, 0);
                }
                if (DAT_1020b430[index].sprayFrames % WISP_DASH_SOUND_PERIOD == 1)
                    FUN_10065b10(index, WISP_DASH_SOUND, 0, 0);
                if (DAT_10db6ee8[index].IsClearAny(0) == 0 &&
                    (place + DAT_10802bf8[index].step - player).Length2D() > WISP_DASH_FAR)
                {
                    int repeats = DAT_1020b430[index].stepCount;
                    DAT_1020b430[index].sprayFrames = 0;
                    if (FUN_1013f880() % 100 < repeats * WISP_DASH_REPEAT_STEP || DAT_1020b430[index].revealed != 0)
                    {
                        DAT_1020b430[index].stepCount = 0;
                        DAT_1020b430[index].wispFrames = WISP_DASH_RESET;
                    }
                    else
                    {
                        DAT_1020b430[index].stepCount = repeats + 1;
                        motion = WISP_DASH_RECOVER_FRAMES;
                        DAT_1020b430[index].wispFrames = 2;
                        DAT_10802bf8[index].step.Fill(0.0f);
                        DAT_10802bf8[index].step.y = WISP_DASH_SPEED;
                        DAT_10802bf8[index].step.RotateZ(WISP_PI - atan2f(diff.x, diff.y));
                    }
                }
            }
            DAT_10db6ee8[index].targetHeading = DAT_10db6ee8[index].heading + 2.8f;
        }
    }
    else if (DAT_1020b430[index].state == WISP_STATE_LUNGE)
    {
        DAT_1020b430[index].shielded = 1;
        if (DAT_1020b430[index].wispFrames == 0)
        {
            EffectOptions ownedOptions;
            ownedOptions.owner = index;
            DAT_10802bf8[index].step.Fill(0.0f);
            DAT_10802bf8[index].step.y = WISP_LUNGE_SPEED;
            float bearing = atan2f(diff.x, diff.y) + WISP_PI;
            DAT_10db6ee8[index].targetHeading = bearing;
            DAT_10802bf8[index].step.RotateZ(-bearing);
            DAT_1020b430[index].wispFrames = 1;
            motionCut = 1;
            DAT_1020b430[index].stepCount = 0;
            Matrix4 turn = RotationZ(DAT_10db6ee8[index].targetHeading);
            Matrix4 scale = Scaling(WISP_LUNGE_SCALE, WISP_LUNGE_SCALE, WISP_LUNGE_SCALE);
            matrix = scale * turn;
            effectPos.Fill(0.0f);
            FUN_1001d8b0(index, WISP_LUNGE_EFFECT, effectPos, -1, -1, &DAT_1021bdf8[index][1]->worldMatrix, 0, 0,
                         &matrix, &ownedOptions);
        }
        if (DAT_1020b430[index].wispFrames == WISP_LUNGE_BOOST_FRAME)
        {
            if (DAT_10db6ee8[index].m_238c != 0)
                DAT_10db6ee8[index].AddMotionSpeed(WISP_LUNGE_BOOST);
            DAT_10802bf8[index].step.Fill(0.0f);
            DAT_10802bf8[index].step.y = WISP_LUNGE_SPEED;
            float bearing = atan2f(diff.x, diff.y) + WISP_PI;
            DAT_10db6ee8[index].targetHeading = bearing;
            DAT_10802bf8[index].step.RotateZ(-bearing);
            DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].step * 0.5f);
            FUN_10065b10(index, WISP_LUNGE_SOUND, 0, 0);
        }
        int frames = DAT_1020b430[index].wispFrames + 1;
        DAT_1020b430[index].wispFrames = frames;
        motion = WISP_MOTION_LUNGE;
        if (DAT_10db6ee8[index].motionFrame > WISP_LUNGE_HIT_FROM &&
            DAT_10db6ee8[index].motionFrame < WISP_LUNGE_HIT_TO)
        {
            DAT_1021f6c0[index] = 1;
            move.Copy(DAT_10802bf8[index].step);
        }
        else
            move.Copy(DAT_10802bf8[index].step * ((float)(WISP_LUNGE_SLIDE_END - frames) / WISP_LUNGE_SLIDE_FRAMES));
        if (DAT_1020b430[index].wispFrames == WISP_LUNGE_PAUSE_FRAME)
        {
            DAT_1020b430[index].stepCount++;
            if (DAT_1020b430[index].stepCount == WISP_LUNGE_REPEATS)
            {
                motionCut = 1;
                motion = WISP_MOTION_LUNGE_END;
            }
            else
                DAT_1020b430[index].wispFrames = WISP_LUNGE_RESUME_FRAME;
        }
        else if (DAT_1020b430[index].wispFrames > WISP_LUNGE_PAUSE_FRAME)
            motion = WISP_MOTION_LUNGE_END;
        if (DAT_1020b430[index].wispFrames == WISP_LUNGE_SLIDE_END)
        {
            DAT_1020b430[index].state = WISP_STATE_STALK;
            DAT_1020b430[index].wispFrames = 1;
        }
    }
    else if (DAT_1020b430[index].state == WISP_STATE_SWIPE_LOW)
    {
        int frames = DAT_1020b430[index].wispFrames;
        DAT_104483e8[index] = -1;
        if (frames < WISP_SWIPE_SETTLE || DAT_10db6ee8[index].m_238c == 0)
            motion = WISP_MOTION_SWIPE_LOW;
        else
        {
            motion = WISP_MOTION_SWIPE_LOW_FOLLOW;
            motionCut = 1;
        }
        if (DAT_10db6ee8[index].motionFrame == WISP_SWIPE_LOW_HIT_FRAME && frames < WISP_SWIPE_SETTLE &&
            DAT_10db6ee8[index].m_238c != 0)
        {
            DAT_10db6ee8[index].PushMotionSpeed(WISP_SWIPE_HIT_SPEED);
            DAT_10db6ee8[index].m_229c = WISP_SWIPE_HIT_BLEND;
            FUN_10065b10(index, WISP_SWIPE_HIT_SOUND, 0, 0);
        }
        else if (DAT_10db6ee8[index].m_229c != 0)
        {
            float blended = (float)(WISP_SWIPE_BLEND_FRAMES - DAT_10db6ee8[index].m_229c);
            DAT_10db6ee8[index].verticalSpeed =
                WISP_SWIPE_LIFT - (blended * WISP_SWIPE_LIFT_SCALE) / WISP_SWIPE_LIFT_SCALE;
        }
        if (DAT_10db6ee8[index].m_238c == 0 || (DAT_10db6ee8[index].motionFrame > WISP_SWIPE_LOW_SHOVE_FROM &&
                                                 DAT_10db6ee8[index].motionFrame < WISP_SWIPE_LOW_SHOVE_TO))
        {
            move.Fill(0.0f);
            move.y = WISP_SWIPE_LOW_SHOVE_SPEED;
            move.RotateZ(-DAT_10db6ee8[index].heading);
        }
        else if (motion == WISP_MOTION_SWIPE_LOW_FOLLOW)
        {
            DAT_1020b430[index].wispFrames++;
            move.Copy(DAT_10802bf8[index].position * WISP_SWIPE_DAMP);
        }
        if (DAT_1020b430[index].wispFrames < WISP_SWIPE_SETTLE)
            DAT_1020b430[index].wispFrames++;
        if (DAT_1020b430[index].wispFrames == WISP_SWIPE_END)
        {
            DAT_1020b430[index].state = WISP_STATE_STALK;
            DAT_1020b430[index].wispFrames = 0;
        }
        FUN_1001bec0(&place, &move);
    }
    else if (DAT_1020b430[index].state == WISP_STATE_CHARGE)
    {
        DAT_104483e8[index] = -1;
        DAT_1020b430[index].shielded = 1;
        if (DAT_1020b430[index].wispFrames == 0)
        {
            DAT_1020b430[index].wispFrames = 1;
            DAT_10802bf8[index].step.Fill(0.0f);
            DAT_10802bf8[index].step.y = WISP_CHARGE_LAUNCH_SPEED;
            float bearing = atan2f(diff.x, diff.y) + WISP_PI;
            DAT_10db6ee8[index].targetHeading = bearing;
            DAT_10802bf8[index].step.RotateZ(-bearing);
            DAT_1020b430[index].stepCount = 0;
        }
        motion = WISP_MOTION_LUNGE;
        if (DAT_1020b430[index].wispFrames == 1)
        {
            move.Copy(DAT_10802bf8[index].step);
            if (DAT_1020b430[index].stepCount % WISP_CHARGE_MOTION_FRAME_STEP == 0)
            {
                EffectOptions ownedOptions;
                ownedOptions.owner = index;
                matrix = RotationZ(DAT_10db6ee8[index].targetHeading);
                effectPos.Fill(0.0f);
                FUN_1001d8b0(index, WISP_CHARGE_SPARK_EFFECT, effectPos, -1, -1, &DAT_10868838[index][2], 0, 0,
                             &matrix, &ownedOptions);
            }
            DAT_1020b430[index].stepCount++;
            if (DAT_10db6ee8[index].motion->blending == 0 && DAT_10db6ee8[index].motionFrame >= WISP_CHARGE_HIT_FRAME)
                DAT_1021f6c0[index] = 1;
            else
                move.Copy(move * WISP_CHARGE_DAMP);
            int mapClear = DAT_10db6ee8[index].IsClearOfMapLimited(&move);
            if (mapClear == 0 || DAT_10db6ee8[index].IsClearAny(&move) == 0)
            {
                DAT_1020b430[index].wispFrames = 2;
                DAT_1020b430[index].stepCount = 0;
                matrix = Scaling(WISP_BURST_SCALE, WISP_BURST_SCALE, WISP_BURST_SCALE);
                effectPos.Copy(DAT_1021bdf8[index][1]->worldMatrix.GetTranslation() + move * WISP_BURST_SCALE);
                matrix.SetTranslation(effectPos);
                effectPos.Fill(0.0f);
                for (int burst = 0; burst < WISP_BURST_COUNT; burst++)
                    FUN_1001d8b0(index, WISP_BURST_EFFECT, effectPos, -1, -1, 0, 0, 0, &matrix, 0);
                FUN_10065b10(index, WISP_BURST_SOUND, 0, 0);
                FUN_1016b230(WISP_BURST_CAPTION);
            }
            if (mapClear != 0)
            {
                DAT_10db6ee8[index].SetPosition(place.x + move.x, place.y + move.y, place.z + move.z);
                DAT_10868838[index][2] = DAT_1021bdf8[index][1]->worldMatrix;
                DAT_10868838[index][2].SetTranslation(DAT_1021bdf8[index][1]->worldMatrix.GetTranslation() +
                                                      move * WISP_BURST_NODE_REACH);
            }
            if (DAT_1020b430[index].stepCount % WISP_CHARGE_FLAME_ROLL == 0 && FUN_1013f880() % 2 != 0 &&
                DAT_1020b430[index].stepCount > WISP_CHARGE_FLAME_FROM)
            {
                Matrix4 turn = RotationZ(DAT_10db6ee8[index].targetHeading);
                Matrix4 tilt = RotationX(-WISP_QUARTER_TURN -
                                         (float)(FUN_1013f880() % WISP_CHARGE_FLAME_TILT_RANGE) * WISP_DEGREE);
                matrix = tilt * turn;
                effectPos.Copy(place + move);
                matrix.SetTranslation(effectPos);
                matrix._43 = matrix._43 + WISP_CHARGE_FLAME_LIFT;
                effectPos.Fill(0.0f);
                FUN_1001d8b0(index, WISP_DASH_FLAME_EFFECT, effectPos, -1, -1, 0, 0, 0, &matrix, 0);
            }
            if (DAT_1020b430[index].stepCount % WISP_CHARGE_FLAME_ROLL == 0 &&
                DAT_1020b430[index].stepCount > WISP_CHARGE_FLAME_FROM)
            {
                matrix = Scaling(WISP_CHARGE_SPLASH_SCALE, WISP_CHARGE_SPLASH_SCALE, WISP_CHARGE_SPLASH_SCALE);
                effectPos.Copy(place + move);
                matrix.SetTranslation(effectPos);
                matrix._41 = ((float)(FUN_1013f880() % WISP_CHARGE_SPLASH_RANGE) - WISP_CHARGE_SPLASH_HALF) +
                             matrix._41;
                matrix._42 = ((float)(FUN_1013f880() % WISP_CHARGE_SPLASH_RANGE) - WISP_CHARGE_SPLASH_HALF) +
                             matrix._42;
                effectPos.Fill(0.0f);
                FUN_1001d8b0(index, WISP_CHARGE_SPLASH_EFFECT, effectPos, -1, -1, 0, 0, 0, &matrix, 0);
            }
            if ((DAT_1020b430[index].stepCount + 1) % WISP_CHARGE_SOUND_A_PERIOD == 0)
                FUN_10065b10(index, WISP_CHARGE_SOUND_A, 0, 0);
            if ((DAT_1020b430[index].stepCount + WISP_CHARGE_SOUND_B_OFFSET) % WISP_CHARGE_SOUND_B_PERIOD == 0)
                FUN_10065b10(index, WISP_CHARGE_SOUND_B, 0, 0);
        }
        else if (DAT_1020b430[index].wispFrames == 2 || DAT_1020b430[index].wispFrames == 3)
        {
            DAT_1020b430[index].stepCount++;
            DAT_1021f6c0[index] = 1;
            move.Copy(DAT_10802bf8[index].step * WISP_RECOIL_BACK);
            move.z = (float)(WISP_RECOIL_FRAMES - DAT_1020b430[index].stepCount);
            if (DAT_10db6ee8[index].IsClearOfMapLimited(&move) != 0 && DAT_10db6ee8[index].IsClearAny(&move) != 0)
                DAT_10db6ee8[index].SetPosition(place.x + move.x, place.y + move.y, place.z + move.z);
            if ((DAT_10db6ee8[index].motionFrame == WISP_RECOIL_HIT_FRAME || DAT_1020b430[index].wispFrames == 3) &&
                WISP_RECOIL_END - DAT_1020b430[index].stepCount < WISP_RECOIL_LATE)
            {
                motion = WISP_MOTION_LUNGE_END;
                motionCut = 1;
                DAT_1020b430[index].wispFrames = 3;
            }
            if (DAT_1020b430[index].stepCount == WISP_RECOIL_END)
            {
                DAT_1020b430[index].state = WISP_STATE_STALK;
                DAT_1020b430[index].wispFrames = 0;
                DAT_1020b430[index].stepCount = 0;
            }
            DAT_10868838[index][2] = DAT_1021bdf8[index][1]->worldMatrix;
            DAT_10868838[index][2].SetTranslation(DAT_1021bdf8[index][1]->worldMatrix.GetTranslation() +
                                                  move * WISP_RECOIL_NODE_REACH);
        }
        move.Fill(0.0f);
    }
    else if (DAT_1020b430[index].state == WISP_STATE_RITUAL)
    {
        int frames = DAT_1020b430[index].wispFrames;
        int phase = frames % WISP_RITUAL_PERIOD;
        DAT_104483e8[index] = -1;
        if (phase >= 0 && phase <= WISP_RITUAL_POSE_FRAMES && frames < WISP_RITUAL_POSE_END)
            motion = WISP_MOTION_RITUAL;
        else
            motion = WISP_MOTION_IDLE;
        motionCut = 1;
        if (DAT_10db6ee8[index].motionFrame == WISP_RITUAL_BURST_FRAME)
        {
            DAT_10db6ee8[index].targetHeading = atan2f(diff.x, diff.y) + WISP_PI;
            effectPos.Fill(0.0f);
            effectPos.y = WISP_RITUAL_DISTANCE;
            effectPos.z = WISP_RITUAL_HEIGHT;
            effectPos.RotateZ(-DAT_10db6ee8[index].targetHeading);
            effectPos.Copy(effectPos + place);
            Matrix4 turn = RotationZ(DAT_10db6ee8[index].targetHeading);
            Matrix4 scale = Scaling(WISP_RITUAL_SCALE, WISP_RITUAL_SCALE, WISP_RITUAL_SCALE);
            matrix = scale * turn;
            matrix.SetTranslation(effectPos);
            effectPos.Fill(0.0f);
            for (int burst = 0; burst < WISP_RITUAL_EFFECT_COUNT; burst++)
                FUN_1001d8b0(index, WISP_RITUAL_EFFECT, effectPos, -1, -1, 0, 0, 0, &matrix, 0);
            FUN_1001d8b0(index, WISP_RITUAL_EFFECT_LAST, effectPos, -1, -1, 0, 0, 0, &matrix, 0);
        }
        else if (DAT_10db6ee8[index].motionFrame == WISP_RITUAL_SHOT_FRAME)
        {
            DAT_10db6ee8[index].targetHeading = atan2f(diff.x, diff.y) + WISP_PI;
            effectPos.Fill(0.0f);
            effectPos.y = WISP_RITUAL_DISTANCE;
            effectPos.z = WISP_RITUAL_HEIGHT;
            effectPos.RotateZ(-DAT_10db6ee8[index].targetHeading);
            effectPos.Copy(effectPos + place);
            matrix = RotationZ(DAT_10db6ee8[index].targetHeading);
            matrix.SetTranslation(effectPos);
            effectPos.Fill(0.0f);
            options.m_0000 = 0;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.m_0008 = WISP_RITUAL_SHOT_SCALE;
            FUN_1001d8b0(index, WISP_RITUAL_SHOT_EFFECT, effectPos, -1, -1, 0, 0, 0, &matrix, &options);
            FUN_10065b10(index, WISP_RITUAL_SHOT_SOUND, 0, 0);
        }
        DAT_1020b430[index].wispFrames++;
        if (DAT_1020b430[index].wispFrames == WISP_RITUAL_END)
        {
            DAT_1020b430[index].state = WISP_STATE_SWIPE;
            DAT_1020b430[index].wispFrames = 0;
        }
    }
    else if (DAT_1020b430[index].state == WISP_STATE_SUMMON)
    {
        DAT_104483e8[index] = -1;
        motion = WISP_MOTION_IDLE;
        DAT_1020b430[index].wispFrames++;
        DAT_10db6ee8[index].targetHeading = atan2f(diff.x, diff.y) + WISP_PI;
        options.owner = index;
        if (DAT_1020b430[index].wispFrames == WISP_SUMMON_FLASH_FRAME)
        {
            matrix.SetIdentity();
            effectPos.Fill(0.0f);
            effectPos.y = WISP_SUMMON_FLASH_DISTANCE;
            effectPos.z = WISP_SUMMON_FLASH_HEIGHT;
            effectPos.RotateZ(-DAT_10db6ee8[index].targetHeading);
            effectPos.Copy(effectPos + DAT_1021bdf8[index][1]->worldMatrix.GetTranslation());
            matrix.SetTranslation(effectPos);
            effectPos.Fill(0.0f);
            FUN_1001d8b0(index, WISP_SUMMON_FLASH_EFFECT, effectPos, -1, -1, 0, 0, 0, &matrix, 0);
            FUN_10065b10(index, WISP_SUMMON_SOUND, 0, 0);
        }
        if (DAT_1020b430[index].wispFrames != WISP_SUMMON_SPAWN_FRAME)
        {
            if (DAT_1020b430[index].wispFrames == WISP_SUMMON_END)
            {
                DAT_1020b430[index].state = WISP_STATE_SWIPE;
                DAT_1020b430[index].wispFrames = 0;
            }
        }
        else if (DAT_10daaae8[index].stats[STAT_HEALTH] < (DAT_10daaae8[index].stats[STAT_MAX_HEALTH] >> 2))
        {
            DAT_10868838[index][0].SetIdentity();
            DAT_10868838[index][1].SetIdentity();
            DAT_10868838[index][0]._14 = 1.0f;
            DAT_10868838[index][1]._14 = 1.0f;
            effectPos.Fill(0.0f);
            effectPos.y = WISP_SUMMON_DISTANCE;
            effectPos.RotateZ(WISP_THIRTY_DEGREES - DAT_10db6ee8[index].targetHeading);
            effectPos.Copy(effectPos + place);
            DAT_10868838[index][0].SetTranslation(effectPos);
            effectPos.Fill(0.0f);
            effectPos.y = WISP_SUMMON_VELOCITY;
            effectPos.RotateZ(WISP_THIRTY_DEGREES - DAT_10db6ee8[index].targetHeading);
            DAT_10868838[index][0]._21 = effectPos.x;
            DAT_10868838[index][0]._11 = effectPos.x;
            DAT_10868838[index][0]._22 = effectPos.y;
            DAT_10868838[index][0]._12 = effectPos.y;
            effectPos.Fill(0.0f);
            options.m_0000 = 0;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.flags |= EFFECT_FLAG_HIDDEN;
            options.m_0008 = WISP_SUMMON_SCALE;
            FUN_1001d8b0(index, WISP_SUMMON_EFFECT, effectPos, -1, -1, &DAT_10868838[index][0], 0, 0, 0, &options);
            effectPos.Fill(0.0f);
            effectPos.y = WISP_SUMMON_DISTANCE;
            effectPos.RotateZ(-DAT_10db6ee8[index].targetHeading - WISP_THIRTY_DEGREES);
            effectPos.Copy(effectPos + place);
            DAT_10868838[index][1].SetTranslation(effectPos);
            effectPos.Fill(0.0f);
            effectPos.y = WISP_SUMMON_VELOCITY;
            effectPos.RotateZ(-DAT_10db6ee8[index].targetHeading - WISP_THIRTY_DEGREES);
            DAT_10868838[index][1]._21 = effectPos.x;
            DAT_10868838[index][1]._11 = effectPos.x;
            DAT_10868838[index][1]._22 = effectPos.y;
            DAT_10868838[index][1]._12 = effectPos.y;
            effectPos.Fill(0.0f);
            FUN_1001d8b0(index, WISP_SUMMON_EFFECT, effectPos, -1, -1, &DAT_10868838[index][1], 0, 0, 0, &options);
        }
        else
        {
            DAT_10868838[index][0].SetIdentity();
            DAT_10868838[index][0]._14 = 1.0f;
            effectPos.Fill(0.0f);
            effectPos.y = WISP_SUMMON_DISTANCE;
            effectPos.RotateZ(-DAT_10db6ee8[index].targetHeading);
            effectPos.Copy(effectPos + place);
            DAT_10868838[index][0].SetTranslation(effectPos);
            effectPos.Fill(0.0f);
            effectPos.y = WISP_SUMMON_VELOCITY;
            effectPos.RotateZ(-DAT_10db6ee8[index].targetHeading);
            DAT_10868838[index][0]._21 = effectPos.x;
            DAT_10868838[index][0]._22 = effectPos.y;
            DAT_10868838[index][0]._11 = effectPos.x;
            DAT_10868838[index][0]._12 = effectPos.y;
            effectPos.Fill(0.0f);
            options.m_0000 = 0;
            options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
            options.flags |= EFFECT_FLAG_HIDDEN;
            options.m_0008 = WISP_SUMMON_SCALE;
            FUN_1001d8b0(index, WISP_SUMMON_EFFECT, effectPos, -1, -1, &DAT_10868838[index][0], 0, 0, 0, &options);
        }
    }
    if (DAT_104bfd08 == 0)
    {
        if (DAT_10db6ee8[index].IsClearOfMapLimited(&move) == 0)
        {
            if (DAT_10255160[index][0] < WISP_SLIDE_TURN_LIMIT && -WISP_SLIDE_TURN_LIMIT < DAT_10255160[index][0])
            {
                if (FUN_1013f880() % 2 == 0)
                    DAT_10255160[index][0] = -WISP_SLIDE_TURN_SET;
                else
                    DAT_10255160[index][0] = WISP_SLIDE_TURN_SET;
            }
            if (WISP_SLIDE_TURN_LIMIT < DAT_10255160[index][0])
                DAT_10255160[index][0] = WISP_SLIDE_TURN_SET;
            if (DAT_10255160[index][0] < -WISP_SLIDE_TURN_LIMIT)
                DAT_10255160[index][0] = -WISP_SLIDE_TURN_SET;
        }
        else
        {
            if (DAT_10255160[index][0] > WISP_SLIDE_TURN_LIMIT)
                DAT_10255160[index][0] = DAT_10255160[index][0] - WISP_SLIDE_TURN_LIMIT;
            else if (DAT_10255160[index][0] < -WISP_SLIDE_TURN_LIMIT)
                DAT_10255160[index][0] = DAT_10255160[index][0] + WISP_SLIDE_TURN_LIMIT;
        }
        if (DAT_10255160[index][0] > WISP_SLIDE_TURN_LIMIT)
            move.RotateZ(WISP_QUARTER_TURN);
        else if (DAT_10255160[index][0] < -WISP_SLIDE_TURN_LIMIT)
            move.RotateZ(-WISP_QUARTER_TURN);
        move.Copy(move * WISP_SLIDE_NEW + DAT_10802bf8[index].position * WISP_SLIDE_SMOOTH);
        DAT_10802bf8[index].position.Copy(move);
        FUN_100241e0(index, move.x, move.y, 0.0f);
        DAT_10db6ee8[index].matrix._43 = move.z + DAT_10db6ee8[index].matrix._43;
    }
    if (DAT_10db6ee8[index].motion->blending == 0)
    {
        DAT_1020b430[index].savedMotion = DAT_10db6ee8[index].motion->layer;
        DAT_1020b430[index].savedMotionFrame = DAT_10db6ee8[index].motion->frame;
    }
    else
    {
        DAT_1020b430[index].savedMotion = DAT_10db6ee8[index].motion->nextLayer;
        DAT_1020b430[index].savedMotionFrame = -1;
    }
    if (motion > -1)
    {
        if (motionCut != 0)
            DAT_10db6ee8[index].PlayMotion(motion, 0, 0, WISP_MOTION_SPEED);
        else
            DAT_10db6ee8[index].PlayMotion(motion, 0, WISP_BLEND, WISP_MOTION_SPEED);
    }
    if (DAT_1020b430[index].reactFrames > 1)
    {
        DAT_1021f6c0[index] = 0;
        DAT_1020b430[index].reactFrames--;
    }
    DAT_10db6ee8[index].root->M_10194730(WISP_NODE_STATE_END, WISP_NODE_MASK_A);
    DAT_10db6ee8[index].root->M_10194730(WISP_NODE_STATE_END, WISP_NODE_MASK_B);
    return 0;
}
