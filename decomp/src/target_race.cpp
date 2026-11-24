// The map 0x15 mini game: the player races two rival characters to destroy 50 targets that drop into the arena. C++
// rather than extern "C": the function keeps an exception frame for its Matrix4 locals.

#include <math.h>
#include "vec4.h"
#include "matrix4.h"
#include "anim_model.h"
#include "node.h"
#include "char_slot.h"
#include "char_state.h"
#include "char_tables.h"
#include "char_vars.h"
#include "char_hikari.h"
#include "char_links.h"
#include "dodge_game.h"
#include "doppler_loops.h"
#include "effect_manager.h"
#include "game.h"
#include "game_vars.h"
#include "hikari_button.h"
#include "map_records.h"
#include "map_vars.h"
#include "minigame_hud.h"
#include "mole_game.h"
#include "path_walk.h"
#include "target_race.h"

// The game's atan2 wrapper (float arguments, so each difference is rounded to a float first).
__forceinline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

#define RIVAL_FIRST 0x3a
#define RIVAL_SECOND 0x3b
#define RIVAL_END 0x3c
#define MARKER_FIRST 0x3c
#define MARKER_END 0x44
#define TARGET_SLOT_FIRST 0x5c
#define TARGET_SLOT_END 0x7c
#define TARGET_TOTAL 0x32
#define TARGET_MARKER_KIND 'Q'

// Banner sounds, as in the other mini games.
#define COUNTDOWN_SOUND 0x221
#define START_SOUND 0x222
#define COUNTDOWN_FRAMES 0x5a
#define BANNER_END_FRAMES 0x78
#define COUNTDOWN_SOUND_PERIOD 0x1e
#define SOUND_VOLUME 1.0f
#define SCORE_POP_FRAMES 0xf

// Rivals' speed factor: 1 (1.35 once more than TARGET_TOTAL targets were counted), times 0.75 on easy and 1.5 on hard,
// and 1.25 more for the second rival. They run 12 units a frame (before the factor).
#define LATE_SPEED 1.35f
#define SPEED_SCALE_EASY 0.75f
#define SPEED_SCALE_HARD 1.5f
#define SECOND_RIVAL_SPEED 1.25f
#define RUN_STEP -12.0f
#define BLOCKED_TURN 1.57075f
#define FACING_OFFSET 3.1415f

// The arena is the map record's first two points, 150 units in from each side.
#define ARENA_MARGIN 150.0f
#define UNDERGROUND_HEIGHT -150.0f
#define OFF_WORLD_X -20000.0f

// Targets: at most spawned / 8 + 2 (up to 8) on screen, one more every 30 frames, at a random point at least 75 units
// from the others. They drop from 900 by 30 a frame and land at -150 with four effects.
#define SPAWN_PERIOD 0x1e
#define MAX_LIVE_BASE 2
#define MAX_LIVE_DIVISOR 8
#define MAX_LIVE 8
#define SPAWN_SPACING 75.0f
#define TARGET_HEALTH 4
#define DROP_START_HEIGHT 900
#define DROP_STEP 0x1e
#define LAND_HEIGHT -150
#define LAND_TRIGGER -0x94
#define LAND_TRIGGER_EXACT -0x95
#define SPAWN_SOUND 0x1de
#define LAND_SOUND 0x1dd
#define LAND_DUST_EFFECT 0x2de
#define LAND_FLASH_EFFECT 0x839
#define LAND_DUST_HEIGHT_A -150.0f
#define LAND_DUST_HEIGHT_B -180.0f
#define LAND_DUST_HEIGHT_C -165.0f
#define LAND_FLASH_HEIGHT -145.0f
#define CRACK_NODE "hibi2"
#define LAND_CAPTION "4545"

// Rival states (CharState::rivalState) and motions.
#define RIVAL_APPEAR 0
#define RIVAL_RUN 1
#define RIVAL_LOOK 2
#define RIVAL_POUNCE 3
#define RIVAL_STUNNED 4
#define MOTION_STAND 2
#define MOTION_RUN 1
#define MOTION_WINDUP 8
#define MOTION_LEAP 0xc
#define MOTION_STUN 10
#define MOTION_BLEND 5

// Rival behaviour: looks for a target after LOOK_FRAMES, runs at it until within POUNCE_RANGE, gives up after
// STUCK_FRAMES blocked frames, picks only targets at most MAX_HEIGHT_GAP above or below it.
#define LOOK_FRAMES 30
#define POUNCE_RANGE 200.0f
#define STUCK_FRAMES 5
#define MAX_HEIGHT_GAP 50.0f

// The pounce lasts 0x2d frames: wind-up, leap (steps of the table below), strike at frame 0x1e, landing, then it
// sinks out of sight.
#define POUNCE_AIM_FRAME 1
#define POUNCE_WINDUP_END 0xf
#define POUNCE_SKIP_STEP 10
#define POUNCE_STRIKE_FRAME 0x1e
#define POUNCE_END_FRAME 0x2d
#define STRIKE_RANGE_PLAYER_SIDE 100.0f
#define STRIKE_RANGE 200.0f
#define STRIKE_MAX_HEIGHT -75.0f

// Stun length by difficulty (frames).
#define STUN_FRAMES_NORMAL 0x10e
#define STUN_FRAMES_EASY 0x1c2
#define STUN_FRAMES_HARD 0x87

// A rival hit by the player (DAT_102550e0 above 0 on easy, above 0xf otherwise; 0x1c = a strong hit) is stunned with
// a chance (percent) by difficulty.
#define HIT_LEVEL_STRONG 0x1c
#define HIT_LEVEL_MIN 0xf
#define STUN_CHANCE_NORMAL 20
#define STUN_CHANCE_EASY 40
#define STUN_CHANCE_HARD 10

// 0x1007dab0 (Map15_UpdateTargetRace). Runs the count-down banner sounds, sets up on the first frame (arena from the
// map record, slot kinds 3, markers cleared), then every frame: every 30th frame drops a new target (up to
// spawned / 8 + 2 at once, 50 in all) at a random point of the arena, lets every target fall and land, scores the
// player's destroyed targets in DAT_10c65060 and the rivals' in DAT_1020c324, and ends the timer when all 50 were
// spent. Each rival (slots 0x3a and 0x3b) appears, looks for the nearest target within 50 units of its height, runs
// at it, pounces (leaping and striking at frame 0x1e, destroying the target in reach) and sinks out of sight; a
// strong hit stuns it. Returns 0.
int Map15_UpdateTargetRace()
{
    if (DAT_1020c2e8 == 0 && DAT_10c66d14 == 4)
    {
        DAT_102028cc = -1;
        DAT_1020c2e8 = 1;
        DAT_102540ca = 0;
        DAT_102540cb = 0;
        FUN_1003ad20(COUNTDOWN_SOUND, 0, SOUND_VOLUME);
    }
    if (DAT_10c66d14 == 4 && DAT_1020c2e8 < BANNER_END_FRAMES)
    {
        if (DAT_1020c2e8 == COUNTDOWN_FRAMES)
            FUN_1003ad20(START_SOUND, 0, SOUND_VOLUME);
        else if (DAT_1020c2e8 % COUNTDOWN_SOUND_PERIOD == 0)
            FUN_1003ad20(COUNTDOWN_SOUND, 0, SOUND_VOLUME);
        DAT_1020c2e8++;
    }
    if (DAT_104c0135 == 0 && DAT_104bfe1c == 0)
    {
        if (DAT_1020c2ec != 0)
            DAT_1020c2ec--;
        if (DAT_1020c2e4 != 0)
            DAT_1020c2e4--;
    }
    if (DAT_1032d69c != 0 && DAT_104c0135 == 0 && DAT_104bfe1c == 0)
    {
        int slot;
        float speed = 1.0f;
        if (DAT_1020c2b8 > TARGET_TOTAL)
            speed = LATE_SPEED;
        if (DAT_10c635f8 == 1)
            speed *= SPEED_SCALE_EASY;
        else if (DAT_10c635f8 == 2)
            speed *= SPEED_SCALE_HARD;
        if (DAT_1020c2b0 == DODGE_STATE_SETUP)
        {
            DAT_1020c2b0 = DODGE_STATE_RUNNING;
            DAT_1020c2b4 = 0;
            DAT_1021f3c0[RIVAL_FIRST] = 3;
            DAT_1021f3c0[RIVAL_SECOND] = 3;
            for (int marker = MARKER_FIRST; marker < MARKER_END; marker++)
            {
                DAT_1020b430[marker].targetSlot = 0;
                DAT_1020b430[marker].dropHeight = 0;
            }
            DAT_10803a78.minX = (float)DAT_10210fc0[0].points[0].x + ARENA_MARGIN;
            DAT_10803a78.spawnY = (float)DAT_10210fc0[0].points[0].y + ARENA_MARGIN;
            DAT_10803a78.minZ = (float)DAT_10210fc0[0].points[0].z;
            DAT_10803a78.maxX = (float)DAT_10210fc0[0].points[1].x - ARENA_MARGIN;
            DAT_10803a78.endY = (float)DAT_10210fc0[0].points[1].y - ARENA_MARGIN;
            DAT_10803a78.maxZ = (float)DAT_10210fc0[0].points[1].z;
        }
        else if (DAT_1020c2b0 == DODGE_STATE_RUNNING)
        {
            if (DAT_10db6ee8[0].matrix.translation.z < UNDERGROUND_HEIGHT)
            {
                DAT_10db6ee8[0].matrix.translation.z = UNDERGROUND_HEIGHT;
                DAT_10db6ee8[0].m_238c = 1;
            }
            DAT_1020c2b4++;
            if (DAT_1020c2b4 % SPAWN_PERIOD == 0 && DAT_1020c2b8 < TARGET_TOTAL)
            {
                int live = 0;
                if (DAT_1020b430[MARKER_FIRST].targetSlot != 0)
                    live++;
                if (DAT_1020b430[MARKER_FIRST + 1].targetSlot != 0)
                    live++;
                if (DAT_1020b430[MARKER_FIRST + 2].targetSlot != 0)
                    live++;
                if (DAT_1020b430[MARKER_FIRST + 3].targetSlot != 0)
                    live++;
                if (DAT_1020b430[MARKER_FIRST + 4].targetSlot != 0)
                    live++;
                if (DAT_1020b430[MARKER_FIRST + 5].targetSlot != 0)
                    live++;
                if (DAT_1020b430[MARKER_FIRST + 6].targetSlot != 0)
                    live++;
                if (DAT_1020b430[MARKER_FIRST + 7].targetSlot != 0)
                    live++;
                int maxLive = DAT_1020c2b8 / MAX_LIVE_DIVISOR + MAX_LIVE_BASE;
                if (maxLive >= MAX_LIVE)
                    maxLive = MAX_LIVE;
                if (maxLive > live)
                {
                    for (int marker = MARKER_FIRST; marker < MARKER_END; marker++)
                    {
                        if (DAT_1020b430[marker].targetSlot != 0)
                            continue;
                        for (slot = TARGET_SLOT_FIRST; slot < TARGET_SLOT_END; slot++)
                        {
                            if ((DAT_1021f3c0[slot] == TARGET_MARKER_KIND &&
                                 DAT_10db6ee8[slot].matrix.translation.x < OFF_WORLD_X) ||
                                DAT_1021f3c0[slot] == SLOT_KIND_DESTROYED)
                            {
                                DAT_1020c2b8 = DAT_1020c2b8 + 1;
                                DAT_10db6ee8[slot].m_0288 = 0;
                                DAT_10db6ee8[slot].m_028c = TARGET_HEALTH;
                                DAT_1021f3c0[slot] = TARGET_MARKER_KIND;
                                DAT_10db6ee8[slot].alpha = 1.0f;
                                DAT_1020b430[marker].targetSlot = slot;
                                DAT_1020b430[marker].dropHeight = DROP_START_HEIGHT;
                                DAT_1020b430[marker].rivalHit = 0;
                                Vec4 spawnPosition;
                                Vec4 otherPosition;
                            retry:
                                DAT_10db6ee8[slot].matrix.translation.x =
                                    (float)(FUN_1013f880() % (int)(DAT_10803a78.maxX - DAT_10803a78.minX)) +
                                    DAT_10803a78.minX;
                                DAT_10db6ee8[slot].matrix.translation.y =
                                    (float)(FUN_1013f880() % (int)(DAT_10803a78.endY - DAT_10803a78.spawnY)) +
                                    DAT_10803a78.spawnY;
                                spawnPosition.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
                                int other = MARKER_FIRST;
                                do
                                {
                                    int otherSlot = DAT_1020b430[other].targetSlot;
                                    if (otherSlot != 0 && slot != otherSlot)
                                    {
                                        otherPosition.Copy(DAT_10db6ee8[otherSlot].matrix.GetTranslation());
                                        if ((spawnPosition - otherPosition).Length2D() < SPAWN_SPACING)
                                            goto retry;
                                    }
                                } while (++other < MARKER_END);
                                Node *crack = DAT_10db6ee8[slot].root->Find(CRACK_NODE, 0);
                                if (crack != 0)
                                    crack->SetFlags(NODE_FLAG_HIDDEN);
                                DAT_10db6ee8[slot].SetPosition(spawnPosition.x, spawnPosition.y,
                                                               (float)DAT_1020b430[marker].dropHeight);
                                (DAT_10db6ee8[slot].matrix.GetTranslation() - DAT_118e6cb8).LengthSquared3();
                                Vec4 soundPosition;
                                soundPosition.Set(DAT_10db6ee8[slot].matrix.translation.x,
                                                  DAT_10db6ee8[slot].matrix.translation.y,
                                                  DAT_10db6ee8[slot].matrix.translation.z, 0);
                                FUN_1003ad20(SPAWN_SOUND, &soundPosition, SOUND_VOLUME);
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            int anyFalling = 0;
            for (int marker = MARKER_FIRST; marker < MARKER_END; marker++)
            {
                if (DAT_1020b430[marker].targetSlot != 0)
                {
                    if (DAT_1021f3c0[DAT_1020b430[marker].targetSlot] == SLOT_KIND_DESTROYED)
                    {
                        if (DAT_1020b430[marker].rivalHit == 0)
                        {
                            DAT_1020c32c++;
                            DAT_1020c2ec = SCORE_POP_FRAMES;
                            DAT_10c65060 = DAT_1020c32c;
                        }
                        else
                        {
                            DAT_1020c324++;
                            DAT_1020c2e4 = SCORE_POP_FRAMES;
                        }
                        DAT_1020b430[marker].targetSlot = 0;
                    }
                    else
                    {
                        anyFalling = 1;
                        if (DAT_1020b430[marker].dropHeight > LAND_TRIGGER_EXACT)
                        {
                            DAT_1020b430[marker].dropHeight -= DROP_STEP;
                            if (DAT_1020b430[marker].dropHeight < LAND_TRIGGER)
                            {
                                DAT_1020b430[marker].dropHeight = LAND_HEIGHT;
                                {
                        Vec4 offset;
                        Matrix4 matrix;
                        matrix.SetIdentity();
                        matrix.SetTranslation(DAT_10db6ee8[DAT_1020b430[marker].targetSlot].matrix.GetTranslation());
                        matrix.translation.z = LAND_DUST_HEIGHT_A;
                        DAT_10daaa80.Start(LAND_DUST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, 0);
                        matrix.translation.z = LAND_DUST_HEIGHT_B;
                        DAT_10daaa80.Start(LAND_DUST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, 0);
                        matrix.translation.z = LAND_DUST_HEIGHT_C;
                        DAT_10daaa80.Start(LAND_DUST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, 0);
                        matrix.translation.z = LAND_FLASH_HEIGHT;
                        DAT_10daaa80.Start(LAND_FLASH_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, 0);
                        FUN_1016b230(LAND_CAPTION);
                        FUN_10065b10(DAT_1020b430[marker].targetSlot, LAND_SOUND, 1, 0);
                                }
                            }
                            DAT_10db6ee8[DAT_1020b430[marker].targetSlot].SetPosition(
                                DAT_10db6ee8[DAT_1020b430[marker].targetSlot].matrix.translation.x,
                                DAT_10db6ee8[DAT_1020b430[marker].targetSlot].matrix.translation.y,
                                (float)DAT_1020b430[marker].dropHeight);
                        }
                        else if (DAT_1020b430[marker].dropHeight == LAND_TRIGGER_EXACT)
                        {
                            DAT_1020b430[marker].dropHeight = LAND_HEIGHT;
                            {
                    Vec4 offset;
                    Matrix4 matrix;
                    matrix.SetIdentity();
                    matrix.SetTranslation(DAT_10db6ee8[DAT_1020b430[marker].targetSlot].matrix.GetTranslation());
                    matrix.translation.z = LAND_DUST_HEIGHT_A;
                    DAT_10daaa80.Start(LAND_DUST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, 0);
                    matrix.translation.z = LAND_DUST_HEIGHT_B;
                    DAT_10daaa80.Start(LAND_DUST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, 0);
                    matrix.translation.z = LAND_DUST_HEIGHT_C;
                    DAT_10daaa80.Start(LAND_DUST_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, 0);
                    matrix.translation.z = LAND_FLASH_HEIGHT;
                    DAT_10daaa80.Start(LAND_FLASH_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, 0, 0);
                    FUN_1016b230(LAND_CAPTION);
                    FUN_10065b10(DAT_1020b430[marker].targetSlot, LAND_SOUND, 1, 0);
                            }
                            DAT_10db6ee8[DAT_1020b430[marker].targetSlot].SetPosition(
                                DAT_10db6ee8[DAT_1020b430[marker].targetSlot].matrix.translation.x,
                                DAT_10db6ee8[DAT_1020b430[marker].targetSlot].matrix.translation.y,
                                (float)DAT_1020b430[marker].dropHeight);
                        }
                    }
                }
            }
            if (!anyFalling && DAT_1020c2b8 == TARGET_TOTAL)
            {
                DAT_10c6505c = 0;
                DAT_1020c2b0 = DODGE_STATE_OVER;
            }
            slot = RIVAL_FIRST;
            do
            {
                if (!(DAT_10db6ee8[slot].matrix.translation.x < OFF_WORLD_X))
                {
                    if (slot == RIVAL_SECOND)
                        speed *= SECOND_RIVAL_SPEED;
                    if (DAT_10db6ee8[slot].matrix.translation.z < UNDERGROUND_HEIGHT)
                        DAT_10db6ee8[slot].matrix.translation.z = UNDERGROUND_HEIGHT;
                    if ((DAT_10c635f8 < 2 && DAT_102550e0[slot] > 0) || DAT_102550e0[slot] > HIT_LEVEL_MIN)
                    {
                        if (DAT_102550e0[slot] == HIT_LEVEL_STRONG &&
                            ((DAT_10c635f8 == 0 && FUN_1013f880() % 100 < STUN_CHANCE_NORMAL) ||
                             (DAT_10c635f8 == 1 && FUN_1013f880() % 100 < STUN_CHANCE_EASY) ||
                             (DAT_10c635f8 == 2 && FUN_1013f880() % 100 < STUN_CHANCE_HARD)))
                        {
                            DAT_1020b430[slot].rivalState = RIVAL_STUNNED;
                            DAT_1020b430[slot].rivalFrames = 0;
                            DAT_1020b430[slot].rivalTarget = 0;
                            DAT_1020b430[slot].rivalStuckFrames = 0;
                            DAT_1020b430[slot].rivalSkipTarget = 0;
                            DAT_102550e0[slot] = 0;
                        }
                        if (DAT_1020b430[slot].rivalState == RIVAL_STUNNED)
                            DAT_102550e0[slot] = 0;
                    }
                    else if (DAT_1020b430[slot].rivalState == RIVAL_APPEAR)
                    {
                        DAT_1020b430[slot].rivalFrames = 0;
                        DAT_1020b430[slot].rivalState = RIVAL_LOOK;
                        DAT_1021bdf8[slot][0] = DAT_10db6ee8[slot].root->Find("Bip01", 0);
                        DAT_10daaae8[slot].m_0141 = 1;
                    }
                    else if (DAT_1020b430[slot].rivalState == RIVAL_RUN)
                    {
                        Vec4 step;
                        Vec4 position;
                        Vec4 targetPosition;
                        position.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
                        targetPosition.Copy(
                            DAT_10db6ee8[DAT_1020b430[DAT_1020b430[slot].rivalTarget].targetSlot].matrix.GetTranslation());
                        DAT_10db6ee8[slot].targetHeading =
                            Atan2(targetPosition.x - position.x, targetPosition.y - position.y) + FACING_OFFSET;
                        step.y = speed * RUN_STEP;
                        step.RotateZ(-DAT_10db6ee8[slot].heading);
                        if (DAT_10db6ee8[slot].IsClearOfMapLimited(&step) && DAT_10db6ee8[slot].IsClearAny(&step))
                            DAT_10db6ee8[slot].SetPosition(step.x + position.x, step.y + position.y, position.z);
                        else
                        {
                            step.RotateZ(BLOCKED_TURN);
                            if (DAT_10db6ee8[slot].IsClearOfMapLimited(&step) && DAT_10db6ee8[slot].IsClearAny(&step))
                                DAT_10db6ee8[slot].SetPosition(step.x + position.x, step.y + position.y, position.z);
                            if (++DAT_1020b430[slot].rivalStuckFrames > STUCK_FRAMES)
                            {
                                DAT_1020b430[slot].rivalStuckFrames = 0;
                                DAT_1020b430[slot].rivalState = RIVAL_LOOK;
                                DAT_1020b430[slot].rivalFrames = 0;
                                DAT_1020b430[slot].rivalSkipTarget = DAT_1020b430[slot].rivalTarget;
                                DAT_1020b430[slot].rivalTarget = 0;
                            }
                        }
                        DAT_10db6ee8[slot].PlayMotion(MOTION_RUN, 0, MOTION_BLEND, 1.0f);
                        if ((position - targetPosition).Length2D() < POUNCE_RANGE)
                        {
                            DAT_1020b430[slot].rivalState = RIVAL_POUNCE;
                            DAT_1020b430[slot].rivalFrames = 0;
                        }
                        if (targetPosition.x < OFF_WORLD_X)
                        {
                            DAT_1020b430[slot].rivalFrames = 0;
                            DAT_1020b430[slot].rivalState = RIVAL_LOOK;
                        }
                    }
                    else if (DAT_1020b430[slot].rivalState == RIVAL_LOOK)
                    {
                        DAT_10db6ee8[slot].PlayMotion(MOTION_STAND, 0, MOTION_BLEND, 1.0f);
                        if (++DAT_1020b430[slot].rivalFrames > LOOK_FRAMES || slot == RIVAL_SECOND)
                        {
                            int best = 0;
                            int bestDistance = 0;
                            int other = RIVAL_FIRST;
                            if (slot == RIVAL_FIRST)
                                other = RIVAL_SECOND;
                            Vec4 position;
                            Vec4 targetPosition;
                            position.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
                            for (int marker = MARKER_FIRST; marker < MARKER_END; marker++)
                            {
                                if (DAT_1020b430[marker].targetSlot != 0 && DAT_1020b430[slot].rivalSkipTarget != marker &&
                                    DAT_1020b430[other].rivalTarget != marker)
                                {
                                    targetPosition.Copy(DAT_10db6ee8[DAT_1020b430[marker].targetSlot].matrix.GetTranslation());
                                    if (best != 0)
                                    {
                                        float limit = (float)bestDistance;
                                        if (!((targetPosition - position).Length2D() < limit))
                                            continue;
                                    }
                                    float heightDifference = targetPosition.z - position.z;
                                    float heightGap = (float)fabs(heightDifference);
                                    if (heightGap < MAX_HEIGHT_GAP)
                                    {
                                        best = marker;
                                        bestDistance = (int)(targetPosition - position).Length2D();
                                    }
                                }
                            }
                            if (best != 0)
                            {
                                DAT_1020b430[slot].rivalSkipTarget = 0;
                                DAT_1020b430[slot].rivalTarget = best;
                                DAT_1020b430[slot].rivalState = RIVAL_RUN;
                                DAT_1020b430[slot].rivalFrames = 0;
                            }
                        }
                    }
                    else if (DAT_1020b430[slot].rivalState == RIVAL_POUNCE)
                    {
                        int frames = DAT_1020b430[slot].rivalFrames + 1;
                        DAT_1020b430[slot].rivalFrames = frames;
                        Vec4 position;
                        Vec4 targetPosition;
                        int motion = MOTION_WINDUP;
                        position.Copy(DAT_10db6ee8[slot].matrix.GetTranslation());
                        targetPosition.Copy(
                            DAT_10db6ee8[DAT_1020b430[DAT_1020b430[slot].rivalTarget].targetSlot].matrix.GetTranslation());
                        if (DAT_1020b430[slot].rivalFrames == POUNCE_AIM_FRAME)
                        {
                            motion = MOTION_STAND;
                            DAT_10db6ee8[slot].targetHeading =
                                Atan2(targetPosition.x - position.x, targetPosition.y - position.y) + FACING_OFFSET;
                        }
                        else if (DAT_1020b430[slot].rivalFrames < POUNCE_WINDUP_END)
                        {
                            motion = MOTION_STAND;
                            if (DAT_10c635f8 == 2 || (slot == RIVAL_SECOND && DAT_1020c308 == POUNCE_SKIP_STEP))
                                DAT_1020b430[slot].rivalFrames = POUNCE_WINDUP_END;
                        }
                        else if (DAT_1020b430[slot].rivalFrames == POUNCE_WINDUP_END)
                            motion = MOTION_WINDUP;
                        else if (DAT_1020b430[slot].rivalFrames < 0x14)
                        {
                            motion = MOTION_WINDUP;
                            Vec4 leap;
                            leap.y = -5.0f;
                            leap.z = 8.0f;
                            leap.RotateZ(-DAT_10db6ee8[slot].targetHeading);
                            if (DAT_10db6ee8[slot].IsClearOfMapLimited(&leap))
                                DAT_10db6ee8[slot].SetPosition(leap.x + position.x, leap.y + position.y,
                                                               position.z + leap.z);
                        }
                        else if (DAT_1020b430[slot].rivalFrames < 0x19)
                        {
                            motion = MOTION_LEAP;
                            Vec4 leap;
                            leap.y = -8.0f;
                            leap.z = 5.0f;
                            leap.RotateZ(-DAT_10db6ee8[slot].targetHeading);
                            if (DAT_10db6ee8[slot].IsClearOfMapLimited(&leap))
                                DAT_10db6ee8[slot].SetPosition(leap.x + position.x, leap.y + position.y,
                                                               leap.z + position.z);
                        }
                        else if (DAT_1020b430[slot].rivalFrames < 0x1e)
                        {
                            motion = MOTION_LEAP;
                            Vec4 leap;
                            leap.y = -10.0f;
                            leap.z = 2.0f;
                            leap.RotateZ(-DAT_10db6ee8[slot].targetHeading);
                            if (DAT_10db6ee8[slot].IsClearOfMapLimited(&leap))
                                DAT_10db6ee8[slot].SetPosition(leap.x + position.x, leap.y + position.y,
                                                               leap.z + position.z);
                        }
                        else if (DAT_1020b430[slot].rivalFrames == POUNCE_STRIKE_FRAME)
                        {
                            motion = MOTION_LEAP;
                            Vec4 leap;
                            leap.y = -12.0f;
                            leap.z = 0.0f;
                            leap.RotateZ(-DAT_10db6ee8[slot].targetHeading);
                            if (DAT_10db6ee8[slot].IsClearOfMapLimited(&leap))
                                DAT_10db6ee8[slot].SetPosition(leap.x + position.x, leap.y + position.y,
                                                               leap.z + position.z);
                            if (slot == RIVAL_SECOND)
                            {
                                int marker = MARKER_FIRST;
                                do
                                {
                                    if (DAT_1021f3c0[DAT_1020b430[marker].targetSlot] == TARGET_MARKER_KIND)
                                    {
                                        targetPosition.Copy(
                                            DAT_10db6ee8[DAT_1020b430[marker].targetSlot].matrix.GetTranslation());
                                        if ((position - targetPosition).Length2D() < STRIKE_RANGE &&
                                            DAT_10db6ee8[DAT_1020b430[marker].targetSlot].m_028c > 0 &&
                                            targetPosition.z < STRIKE_MAX_HEIGHT)
                                        {
                                            FUN_100541f0(DAT_1020b430[marker].targetSlot);
                                            DAT_1020b430[marker].rivalHit = 1;
                                        }
                                    }
                                } while (++marker < MARKER_END);
                            }
                            else if ((position - targetPosition).Length2D() < STRIKE_RANGE_PLAYER_SIDE)
                            {
                                int victim = DAT_1020b430[DAT_1020b430[slot].rivalTarget].targetSlot;
                                if (DAT_10db6ee8[victim].m_028c > 0 && DAT_1021f3c0[victim] == TARGET_MARKER_KIND &&
                                    targetPosition.z < STRIKE_MAX_HEIGHT)
                                {
                                    FUN_100541f0(victim);
                                    DAT_1020b430[DAT_1020b430[slot].rivalTarget].rivalHit = 1;
                                }
                            }
                        }
                        else if (DAT_1020b430[slot].rivalFrames < 0x23)
                        {
                            motion = MOTION_LEAP;
                            Vec4 leap;
                            leap.y = -10.0f;
                            leap.z = -2.0f;
                            leap.RotateZ(-DAT_10db6ee8[slot].targetHeading);
                            if (DAT_10db6ee8[slot].IsClearOfMapLimited(&leap))
                                DAT_10db6ee8[slot].SetPosition(leap.x + position.x, leap.y + position.y,
                                                               leap.z + position.z);
                        }
                        else if (DAT_1020b430[slot].rivalFrames < 0x28)
                        {
                            motion = MOTION_LEAP;
                            Vec4 leap;
                            leap.y = -8.0f;
                            leap.z = -5.0f;
                            leap.RotateZ(-DAT_10db6ee8[slot].targetHeading);
                            if (DAT_10db6ee8[slot].IsClearOfMapLimited(&leap))
                                DAT_10db6ee8[slot].SetPosition(leap.x + position.x, leap.y + position.y,
                                                               leap.z + position.z);
                        }
                        else if (DAT_1020b430[slot].rivalFrames < 0x2d)
                        {
                            motion = MOTION_STAND;
                            Vec4 leap;
                            leap.y = -5.0f;
                            leap.z = -8.0f;
                            leap.RotateZ(-DAT_10db6ee8[slot].targetHeading);
                            if (DAT_10db6ee8[slot].IsClearOfMapLimited(&leap))
                                DAT_10db6ee8[slot].SetPosition(leap.x + position.x, leap.y + position.y,
                                                               leap.z + position.z);
                        }
                        else if (DAT_1020b430[slot].rivalFrames == POUNCE_END_FRAME)
                        {
                            motion = MOTION_STAND;
                            DAT_10db6ee8[slot].SetPosition(position.x, position.y, UNDERGROUND_HEIGHT);
                            DAT_1020b430[slot].rivalState = RIVAL_LOOK;
                            DAT_1020b430[slot].rivalFrames = 0;
                            DAT_1020b430[slot].rivalTarget = 0;
                        }
                        DAT_10db6ee8[slot].PlayMotion(motion, 0, MOTION_BLEND, 1.0f);
                    }
                    else if (DAT_1020b430[slot].rivalState == RIVAL_STUNNED)
                    {
                        DAT_10db6ee8[slot].PlayMotion(MOTION_STUN, 0, MOTION_BLEND, 1.0f);
                        int difficulty = DAT_10c635f8;
                        int frames = ++DAT_1020b430[slot].rivalFrames;
                        if ((difficulty == 0 && frames > STUN_FRAMES_NORMAL) ||
                            (difficulty == 1 && frames > STUN_FRAMES_EASY) ||
                            (difficulty == 2 && frames > STUN_FRAMES_HARD))
                        {
                            DAT_1020b430[slot].rivalFrames = 0;
                            DAT_1020b430[slot].rivalState = RIVAL_LOOK;
                            DAT_1020b430[slot].rivalTarget = 0;
                            DAT_1020b430[slot].rivalStuckFrames = 0;
                            DAT_1020b430[slot].rivalSkipTarget = 0;
                        }
                    }
                }
            } while (++slot < RIVAL_END);
        }
    }
    return 0;
}
