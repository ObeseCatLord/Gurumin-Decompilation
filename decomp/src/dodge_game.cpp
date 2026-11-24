// The map 0x16 mini game: per-frame update of the obstacles the player has to dodge. C++ rather than extern "C":
// the function keeps an exception frame for its Vec4 and EffectOptions locals.

#include "vec4.h"
#include "matrix4.h"
#include "anim_model.h"
#include "char_hikari.h"
#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "dinput_min.h"
#include "dodge_game.h"
#include "doppler_loops.h"
#include "effect_manager.h"
#include "game.h"
#include "game_vars.h"
#include "map_records.h"
#include "map_vars.h"
#include "minigame_hud.h"
#include "mole_game.h"

// Banner sounds: effect 0x221 every 30 frames of the count-down, 0x222 when the start word shows (frame 0x5a); the banner
// runs to frame 0x78.
#define COUNTDOWN_SOUND 0x221
#define START_SOUND 0x222
#define SCORE_SOUND 0x220
#define COUNTDOWN_FRAMES 0x5a
#define BANNER_END_FRAMES 0x78
#define COUNTDOWN_SOUND_PERIOD 0x1e
#define SOUND_VOLUME 1.0f
#define SCORE_POP_FRAMES 0xf

// Obstacle speed factor: floor and range come from the frame counter; the range grows from 1 to 2.5 times 1000 from
// frame 4000 on and the factor is a random value below it, never under the floor (which sinks from 1000 to 500).
#define RAMP_START_FRAMES 4000
#define RAMP_LENGTH 2000.0f
#define RAMP_MIN 1.0f
#define RAMP_MAX 2.5f
#define SPEED_UNIT 1000
#define SPEED_UNIT_F 1000.0f
#define SPEED_FLOOR_START 1000
#define SPEED_FLOOR_MIN 500
#define SPEED_FLOOR_FALL_START 0x1194
#define SPEED_FLOOR_FALL_FRAMES 100

// Spawn interval scale per difficulty (DAT_10c635f8: 1 = easy, 2 = hard) and the obstacle speed factors.
#define SPAWN_SCALE_EASY 2.0f
#define SPAWN_SCALE_HARD 0.75f
#define SPEED_SCALE_EASY 0.75f
#define SPEED_SCALE_HARD 1.25f
#define FAST_SPEED_SCALE 2.0f

// Spawn rules by score band: the obstacle spawns when more than (scale * interval) frames have passed since the last
// one and a 10 in 100 roll succeeds; the kind is 1 + a random number below the band's kind count.
#define SPAWN_ROLL_RANGE 100
#define SPAWN_ROLL_BELOW 10
#define SPAWN_LATE_SCORE 100
#define SPAWN_LATE_INTERVAL 12.0f
#define SPAWN_LATE_SPEED_DIVISOR 10.0f
#define SPAWN_LATE_MIN_INTERVAL 1.0f
#define SPAWN_LATE_ROLL_BASE 10
#define SPAWN_LATE_GROWTH_FRAMES 100
#define SPAWN_LATE_KINDS 12

// Obstacle motion per frame (before the speed factors) and the pole geometry.
#define SWEEP_SPEED 8.0f
#define POLE_FALL_SPEED 10.0f
#define POLE_SPAWN_STEP 25.0f
#define POLE_EDGE_MARGIN 50.0f
#define POLE_HEIGHT 800.0f
#define DRIFT_SUM 16
#define DRIFT_MIN 4
#define DRIFT_RANGE 9

// Effects: 0x768 follows a spawned obstacle (owner = slot + EFFECT_OWNER_BASE), 0x76a is the splash where a pole lands, 0xc
// the knock-down burst over the player.
#define SPAWN_EFFECT 0x768
#define LANDING_EFFECT 0x76a
#define HIT_EFFECT 0xc
#define EFFECT_OWNER_BASE 0x9f
#define DODGE_FIRST_SLOT 0x3b
#define EFFECT_OWNER_OFFSET 100

// Contact: the player's point (20 above the feet) closer than 20 units (squared: 400) to an obstacle's segment.
#define PLAYER_HEIGHT 20.0f
#define HIT_DISTANCE_SQUARED 400.0f
#define HIT_RUMBLE_DIVISOR 1.0f
#define HIT_RUMBLE_MS 100
#define HIT_DISPLAY_STATE 0x3c
#define HIT_PLAYER_STATE 4
#define HIT_EFFECT_LIFT 50.0f
#define DAMAGE_FLAG 2

#define END_EVENT 0xea

// Obstacle index of an effect owner id (the first loop walks the owner ids, the arrays follow by index).
#define DODGE_INDEX(owner) ((owner) - EFFECT_OWNER_BASE)

// Scales a step by the obstacle's speed factor and the difficulty; the fast kind first moves at twice the speed.
static __forceinline void ScaleStep(Vec4 &step, int kind, int fastKind, const float &speed)
{
    if (kind == fastKind)
        step.Copy(step * FAST_SPEED_SCALE);
    step.Copy(step * speed);
    if (DAT_10c635f8 == 1)
        step.Copy(step * SPEED_SCALE_EASY);
    else if (DAT_10c635f8 == 2)
        step.Copy(step * SPEED_SCALE_HARD);
}

// 0x10053020 (Map16_UpdateDodgeGame). Runs the count-down banner sounds, sets the game up on its first frame (arena
// from the map record, all obstacles free) and then every frame: picks whether a new obstacle spawns (the more points,
// the more often and the more kinds, see the spawn bands), moves every obstacle in play, tests the player against its
// segment, and scores a point for each one that leaves the arena. A contact (or a downed player) knocks the player
// down, rumbles, ends the timer and removes the obstacles' effects. When the map timer reaches 0 the player's health is
// restored and the end event is raised. Returns 0.
int Map16_UpdateDodgeGame()
{
    float spawnScale = 1.0f;
    int speedFloor = SPEED_FLOOR_MIN;
    int floorCandidate = SPEED_FLOOR_START - (DAT_1020c2b4 - SPEED_FLOOR_FALL_START) / SPEED_FLOOR_FALL_FRAMES;
    if (floorCandidate >= SPEED_FLOOR_MIN)
        speedFloor = floorCandidate;
    float ramp = (float)(DAT_1020c2b4 - RAMP_START_FRAMES) / RAMP_LENGTH;
    ramp = (RAMP_MIN < ramp) ? ((RAMP_MAX > ramp) ? ramp : RAMP_MAX) : RAMP_MIN;
    int roll = FUN_1013f880() % (int)(ramp * SPEED_UNIT_F);
    roll = (speedFloor < roll) ? roll : speedFloor;
    float speedScale = (float)roll / SPEED_UNIT_F;
    if (DAT_10c635f8 == 1)
        spawnScale = SPAWN_SCALE_EASY;
    else if (DAT_10c635f8 == 2)
        spawnScale = SPAWN_SCALE_HARD;
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
    if (DAT_104c0135 == 0 && DAT_104bfe1c == 0 && DAT_1020c2ec != 0)
        DAT_1020c2ec--;
    if (DAT_1032d69c == 0 || DAT_104c0135 != 0 || DAT_104bfe1c != 0)
        return 0;
    if (DAT_1020c2b0 == DODGE_STATE_SETUP)
    {
        DAT_104c00b8 = 1;
        DAT_1020c2b0 = DODGE_STATE_RUNNING;
        DAT_1020c2b4 = 0;
        for (int i = 0; i < DODGE_OBJECT_COUNT; i++)
        {
            DAT_1020c2f0[i].kind = 0;
            DAT_1020c2f0[i].reserved = 0;
        }
        DAT_10803a78.minX = (float)DAT_10210fc0[0].points[0].x;
        DAT_10803a78.spawnY = (float)DAT_10210fc0[0].points[1].y;
        DAT_10803a78.minZ = (float)DAT_10210fc0[0].points[0].z;
        DAT_10803a78.maxX = (float)DAT_10210fc0[0].points[1].x;
        DAT_10803a78.endY = (float)DAT_10210fc0[0].points[0].y;
        DAT_10803a78.maxZ = (float)DAT_10210fc0[0].points[1].z;
    }
    else if (DAT_1020c2b0 == DODGE_STATE_RUNNING)
    {
        DAT_1020c2b4++;
        DAT_1020c2b8++;
        int spawnKind = 0;
        Vec4 vector;
        if (DAT_1020c32c < 10)
        {
            if (DAT_1020c2b8 > (int)(spawnScale * 45.0f) && FUN_1013f880() % SPAWN_ROLL_RANGE < SPAWN_ROLL_BELOW)
            {
                spawnKind = FUN_1013f880() % 2 + 1;
                DAT_1020c2b8 = 0;
            }
        }
        else if (DAT_1020c32c < 20)
        {
            if (DAT_1020c2b8 > (int)(spawnScale * 43.0f) && FUN_1013f880() % SPAWN_ROLL_RANGE < SPAWN_ROLL_BELOW)
            {
                spawnKind = FUN_1013f880() % 3 + 1;
                DAT_1020c2b8 = 0;
            }
        }
        else if (DAT_1020c32c < 30)
        {
            if (DAT_1020c2b8 > (int)(spawnScale * 41.0f) && FUN_1013f880() % SPAWN_ROLL_RANGE < SPAWN_ROLL_BELOW)
            {
                spawnKind = FUN_1013f880() % 6 + 1;
                DAT_1020c2b8 = 0;
            }
        }
        else if (DAT_1020c32c < 40)
        {
            if (DAT_1020c2b8 > (int)(spawnScale * 39.0f) && FUN_1013f880() % SPAWN_ROLL_RANGE < SPAWN_ROLL_BELOW)
            {
                spawnKind = FUN_1013f880() % 7 + 1;
                DAT_1020c2b8 = 0;
            }
        }
        else if (DAT_1020c32c < 50)
        {
            if (DAT_1020c2b8 > (int)(spawnScale * 37.0f) && FUN_1013f880() % SPAWN_ROLL_RANGE < SPAWN_ROLL_BELOW)
            {
                spawnKind = FUN_1013f880() % 8 + 1;
                DAT_1020c2b8 = 0;
            }
        }
        else if (DAT_1020c32c < 60)
        {
            if (DAT_1020c2b8 > (int)(spawnScale * 35.0f) && FUN_1013f880() % SPAWN_ROLL_RANGE < SPAWN_ROLL_BELOW)
            {
                spawnKind = FUN_1013f880() % SPAWN_LATE_KINDS + 1;
                DAT_1020c2b8 = 0;
            }
        }
        else if (DAT_1020c32c < 70)
        {
            if (DAT_1020c2b8 > (int)(spawnScale * 30.0f) && FUN_1013f880() % SPAWN_ROLL_RANGE < SPAWN_ROLL_BELOW)
            {
                spawnKind = FUN_1013f880() % SPAWN_LATE_KINDS + 1;
                DAT_1020c2b8 = 0;
            }
        }
        else if (DAT_1020c32c < 80)
        {
            if (DAT_1020c2b8 > (int)(spawnScale * 20.0f) && FUN_1013f880() % SPAWN_ROLL_RANGE < SPAWN_ROLL_BELOW)
            {
                spawnKind = FUN_1013f880() % SPAWN_LATE_KINDS + 1;
                DAT_1020c2b8 = 0;
            }
        }
        else if (DAT_1020c32c < 90)
        {
            if (DAT_1020c2b8 > (int)(spawnScale * 15.0f) && FUN_1013f880() % SPAWN_ROLL_RANGE < SPAWN_ROLL_BELOW)
            {
                spawnKind = FUN_1013f880() % SPAWN_LATE_KINDS + 1;
                DAT_1020c2b8 = 0;
            }
        }
        else if (DAT_1020c32c < SPAWN_LATE_SCORE)
        {
            if (DAT_1020c2b8 > (int)(spawnScale * 15.0f) && FUN_1013f880() % SPAWN_ROLL_RANGE < SPAWN_ROLL_BELOW)
            {
                spawnKind = FUN_1013f880() % SPAWN_LATE_KINDS + 1;
                DAT_1020c2b8 = 0;
            }
        }
        else
        {
            float rawInterval = (float)(int)(spawnScale * SPAWN_LATE_INTERVAL) - speedScale / SPAWN_LATE_SPEED_DIVISOR;
            float interval = (SPAWN_LATE_MIN_INTERVAL < rawInterval) ? rawInterval : SPAWN_LATE_MIN_INTERVAL;
            if ((float)DAT_1020c2b8 > interval)
            {
                int rollBelow = FUN_1013f880() % ((DAT_1020c2b8 - SPEED_FLOOR_FALL_START) / SPAWN_LATE_GROWTH_FRAMES + 1) +
                                SPAWN_LATE_ROLL_BASE;
                if (FUN_1013f880() % SPAWN_ROLL_RANGE < rollBelow)
                {
                    spawnKind = FUN_1013f880() % SPAWN_LATE_KINDS + 1;
                    DAT_1020c2b8 = 0;
                }
            }
        }
        int hit = 0;
        for (int owner = EFFECT_OWNER_BASE; owner < EFFECT_OWNER_BASE + DODGE_OBJECT_COUNT; owner++)
        {
            int offscreen = 0;
            if (DAT_1020c2f0[DODGE_INDEX(owner)].kind != 0)
            {
                vector.Fill(0.0f);
                switch (DAT_1020c2f0[DODGE_INDEX(owner)].kind)
                {
                case DODGE_KIND_BAR:
                case DODGE_KIND_FAST_BAR:
                {
                    vector.y = -SWEEP_SPEED;
                    ScaleStep(vector, DAT_1020c2f0[DODGE_INDEX(owner)].kind, DODGE_KIND_FAST_BAR, DAT_10255510[DODGE_INDEX(owner)].scale);
                    float y = DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.y + vector.y;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.y = y;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endB.translation.y += vector.y;
                    if (y < DAT_10803a78.endY)
                        offscreen = 1;
                    break;
                }
                case DODGE_KIND_POLE:
                case DODGE_KIND_FAST_POLE:
                {
                    vector.y = -POLE_FALL_SPEED;
                    ScaleStep(vector, DAT_1020c2f0[DODGE_INDEX(owner)].kind, DODGE_KIND_FAST_POLE, DAT_10255510[DODGE_INDEX(owner)].scale);
                    float y = DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.y + vector.y;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.y = y;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endB.translation.y += vector.y;
                    if (y < DAT_10803a78.endY)
                        offscreen = 1;
                    else if (y + vector.y < DAT_10803a78.endY)
                    {
                        DAT_1086e0b8[DODGE_INDEX(owner)].landingA = DAT_1086e0b8[DODGE_INDEX(owner)].endA;
                        DAT_1086e0b8[DODGE_INDEX(owner)].landingA.translation.y += vector.y;
                        DAT_1086e0b8[DODGE_INDEX(owner)].landingB = DAT_1086e0b8[DODGE_INDEX(owner)].endB;
                        DAT_1086e0b8[DODGE_INDEX(owner)].landingB.translation.y += vector.y;
                        vector.Fill(0.0f);
                        DAT_10daaa80.Start(LANDING_EFFECT, vector, -1, -1, &DAT_1086e0b8[DODGE_INDEX(owner)].landingA,
                                           (int)&DAT_1086e0b8[DODGE_INDEX(owner)].landingB, 0, 0, 0, 0);
                    }
                    break;
                }
                case DODGE_KIND_SWEEP_RIGHT:
                case DODGE_KIND_FAST_SWEEP_RIGHT:
                {
                    vector.x = SWEEP_SPEED;
                    ScaleStep(vector, DAT_1020c2f0[DODGE_INDEX(owner)].kind, DODGE_KIND_FAST_SWEEP_RIGHT, DAT_10255510[DODGE_INDEX(owner)].scale);
                    float x = DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.x + vector.x;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.x = x;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endB.translation.x += vector.x;
                    if (x > DAT_10803a78.maxX)
                        offscreen = 1;
                    break;
                }
                case DODGE_KIND_SWEEP_LEFT:
                case DODGE_KIND_FAST_SWEEP_LEFT:
                {
                    vector.x = -SWEEP_SPEED;
                    ScaleStep(vector, DAT_1020c2f0[DODGE_INDEX(owner)].kind, DODGE_KIND_FAST_SWEEP_LEFT, DAT_10255510[DODGE_INDEX(owner)].scale);
                    float x = DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.x + vector.x;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.x = x;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endB.translation.x += vector.x;
                    if (x < DAT_10803a78.minX)
                        offscreen = 1;
                    break;
                }
                case DODGE_KIND_DIAGONAL_RIGHT:
                case DODGE_KIND_FAST_DIAGONAL_RIGHT:
                {
                    vector.x = (float)DAT_1020c2f0[DODGE_INDEX(owner)].driftX;
                    vector.y = (float)-DAT_1020c2f0[DODGE_INDEX(owner)].driftY;
                    ScaleStep(vector, DAT_1020c2f0[DODGE_INDEX(owner)].kind, DODGE_KIND_FAST_DIAGONAL_RIGHT, DAT_10255510[DODGE_INDEX(owner)].scale);
                    if (DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.x < DAT_10803a78.maxX)
                        DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.x += vector.x;
                    else
                        DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.y += vector.y;
                    if (DAT_1086e0b8[DODGE_INDEX(owner)].endB.translation.y > DAT_10803a78.endY)
                        DAT_1086e0b8[DODGE_INDEX(owner)].endB.translation.y += vector.y;
                    else
                        DAT_1086e0b8[DODGE_INDEX(owner)].endB.translation.x += vector.x;
                    if (DAT_1086e0b8[DODGE_INDEX(owner)].endB.translation.y <= DAT_10803a78.endY &&
                        DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.y <= DAT_10803a78.endY)
                        offscreen = 1;
                    break;
                }
                case DODGE_KIND_DIAGONAL_LEFT:
                case DODGE_KIND_FAST_DIAGONAL_LEFT:
                {
                    vector.x = (float)-DAT_1020c2f0[DODGE_INDEX(owner)].driftX;
                    vector.y = (float)-DAT_1020c2f0[DODGE_INDEX(owner)].driftY;
                    ScaleStep(vector, DAT_1020c2f0[DODGE_INDEX(owner)].kind, DODGE_KIND_FAST_DIAGONAL_LEFT, DAT_10255510[DODGE_INDEX(owner)].scale);
                    if (DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.x > DAT_10803a78.minX)
                        DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.x += vector.x;
                    else
                        DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.y += vector.y;
                    if (DAT_1086e0b8[DODGE_INDEX(owner)].endB.translation.y > DAT_10803a78.endY)
                        DAT_1086e0b8[DODGE_INDEX(owner)].endB.translation.y += vector.y;
                    else
                        DAT_1086e0b8[DODGE_INDEX(owner)].endB.translation.x += vector.x;
                    if (DAT_1086e0b8[DODGE_INDEX(owner)].endB.translation.y <= DAT_10803a78.endY &&
                        DAT_1086e0b8[DODGE_INDEX(owner)].endA.translation.y <= DAT_10803a78.endY)
                        offscreen = 1;
                    break;
                }
                }
                Vec4 player;
                Vec4 endA;
                Vec4 endB;
                player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                player.z += PLAYER_HEIGHT;
                endA.Copy(DAT_1086e0b8[DODGE_INDEX(owner)].endA.GetTranslation());
                endB.Copy(DAT_1086e0b8[DODGE_INDEX(owner)].endB.GetTranslation());
                if (hit || player.DistanceSquaredToSegment(endA, endB) < HIT_DISTANCE_SQUARED)
                    hit = 1;
                if (offscreen)
                {
                    DAT_10daaa80.RemoveOwnedBy(owner);
                    DAT_1020c32c = DAT_10c65060 + 1;
                    DAT_1020c2f0[DODGE_INDEX(owner)].kind = 0;
                    DAT_1020c2ec = SCORE_POP_FRAMES;
                    DAT_10c65060 = DAT_1020c32c;
                    FUN_1003ad20(SCORE_SOUND, 0, SOUND_VOLUME);
                }
            }
            vector.Fill(0.0f);
            if (spawnKind != 0 && DAT_1020c2f0[DODGE_INDEX(owner)].kind == 0)
            {
                DAT_10255510[DODGE_INDEX(owner)].scale = speedScale;
                DAT_1020c2f0[DODGE_INDEX(owner)].kind = spawnKind;
                switch (spawnKind)
                {
                case DODGE_KIND_BAR:
                case DODGE_KIND_FAST_BAR:
                    vector.x = DAT_10803a78.minX;
                    vector.y = DAT_10803a78.spawnY;
                    vector.z = DAT_10803a78.minZ;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endA.SetTranslation(vector);
                    vector.x = DAT_10803a78.maxX;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endB.SetTranslation(vector);
                    break;
                case DODGE_KIND_POLE:
                case DODGE_KIND_FAST_POLE:
                    do
                    {
                        do
                        {
                            vector.x = (float)(FUN_1013f880() % (int)((DAT_10803a78.maxX - DAT_10803a78.minX) / POLE_SPAWN_STEP)) *
                                           POLE_SPAWN_STEP + DAT_10803a78.minX;
                        } while (vector.x < DAT_10803a78.minX + POLE_EDGE_MARGIN);
                    } while (DAT_10803a78.maxX - POLE_EDGE_MARGIN < vector.x);
                    vector.y = DAT_10803a78.spawnY;
                    vector.z = DAT_10803a78.minZ - POLE_EDGE_MARGIN;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endA.SetTranslation(vector);
                    vector.z += POLE_HEIGHT;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endB.SetTranslation(vector);
                    break;
                case DODGE_KIND_SWEEP_RIGHT:
                case DODGE_KIND_FAST_SWEEP_RIGHT:
                    vector.x = DAT_10803a78.minX;
                    vector.y = DAT_10803a78.spawnY;
                    vector.z = DAT_10803a78.minZ;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endA.SetTranslation(vector);
                    vector.y = DAT_10803a78.endY;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endB.SetTranslation(vector);
                    break;
                case DODGE_KIND_SWEEP_LEFT:
                case DODGE_KIND_FAST_SWEEP_LEFT:
                    vector.x = DAT_10803a78.maxX;
                    vector.y = DAT_10803a78.endY;
                    vector.z = DAT_10803a78.minZ;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endA.SetTranslation(vector);
                    vector.y = DAT_10803a78.spawnY;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endB.SetTranslation(vector);
                    break;
                case DODGE_KIND_DIAGONAL_RIGHT:
                case DODGE_KIND_FAST_DIAGONAL_RIGHT:
                    vector.x = DAT_10803a78.minX;
                    vector.y = DAT_10803a78.spawnY;
                    vector.z = DAT_10803a78.minZ;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endA.SetTranslation(vector);
                    DAT_1086e0b8[DODGE_INDEX(owner)].endB.SetTranslation(vector);
                    DAT_1020c2f0[DODGE_INDEX(owner)].driftX = FUN_1013f880() % DRIFT_RANGE + DRIFT_MIN;
                    DAT_1020c2f0[DODGE_INDEX(owner)].driftY = DRIFT_SUM - DAT_1020c2f0[DODGE_INDEX(owner)].driftX;
                    break;
                case DODGE_KIND_DIAGONAL_LEFT:
                case DODGE_KIND_FAST_DIAGONAL_LEFT:
                    vector.x = DAT_10803a78.maxX;
                    vector.y = DAT_10803a78.spawnY;
                    vector.z = DAT_10803a78.minZ;
                    DAT_1086e0b8[DODGE_INDEX(owner)].endA.SetTranslation(vector);
                    DAT_1086e0b8[DODGE_INDEX(owner)].endB.SetTranslation(vector);
                    DAT_1020c2f0[DODGE_INDEX(owner)].driftX = FUN_1013f880() % DRIFT_RANGE + DRIFT_MIN;
                    DAT_1020c2f0[DODGE_INDEX(owner)].driftY = DRIFT_SUM - DAT_1020c2f0[DODGE_INDEX(owner)].driftX;
                    break;
                }
                EffectOptions options;
                vector.Fill(0.0f);
                options.owner = owner;
                options.m_0000 = 0;
                options.m_0004 = 0;
                DAT_10daaa80.Start(SPAWN_EFFECT, vector, -1, -1, &DAT_1086e0b8[DODGE_INDEX(owner)].endA, (int)&DAT_1086e0b8[DODGE_INDEX(owner)].endB, 0, 0,
                                   &options, 0);
                spawnKind = 0;
            }
        }
        if (hit || DAT_104686b0[0] != 0 || DAT_104c01e8 != 0)
        {
            DAT_10f08008.Rumble(HIT_RUMBLE_DIVISOR, HIT_RUMBLE_MS);
            if (DAT_104686b0[0] == 0)
            {
                FUN_100119d0(0, 0, DAMAGE_FLAG);
                DAT_104686b0[0] = HIT_DISPLAY_STATE;
                DAT_1020d4c0[0] = HIT_PLAYER_STATE;
                Vec4 burst = DAT_10db6ee8[0].matrix.GetTranslation();
                burst.z += HIT_EFFECT_LIFT;
                DAT_10daaa80.Start(HIT_EFFECT, burst, -1, 0, 0, 0, 0, 0, 0, 0);
            }
            DAT_10c6505c = 0;
            DAT_1020c6b0 = DODGE_STATE_OVER;
            for (int slot = DODGE_FIRST_SLOT; slot < DODGE_FIRST_SLOT + DODGE_OBJECT_COUNT; slot++)
            {
                if (DAT_1020c2f0[slot - DODGE_FIRST_SLOT].kind != 0)
                    DAT_10daaa80.RemoveOwnedBy(slot + EFFECT_OWNER_OFFSET);
            }
        }
    }
    if (DAT_10c6505c == 0)
    {
        DAT_10daaae8[0].stats[STAT_HEALTH] = DAT_10daaae8[0].stats[STAT_MAX_HEALTH];
        DAT_104c00b8 = 0;
        FUN_1000e750();
        DAT_10874848.RegisterEvent(END_EVENT);
    }
    return 0;
}
