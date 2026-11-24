// The laser zone: the update of an area zone character whose map record is of type 6 (called by Char_AreaZone). The
// zone is a box of the map (record rows 0 and 1) that holds the model "@tekiatari1" with up to 12 pairs of beam
// nodes ("laserc00".."laserc23" under it). While it runs, new pairs start at random places along the box edges; each
// pair sweeps across the box with the speed in its code and is released when it has crossed.

#include <stdio.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "effect_manager.h"

#include "last_position.h"

#include "char_state.h"

#include "char_tables.h"

#include "map_records.h"

#include "map_vars.h"

#include "char_vars.h"

#include "char_hikari.h"

#include "laser_zone.h"

// The pattern words of all slots as one array (the original indexes the behaviour state as a flat word table; the
// start-up loop even clears 24 words, running over into the next slot).
#define LASER_PATTERN(index, pair) ((&DAT_1020b430[0].laserPattern[0])[(index) * 16 + (pair)])

// The two nodes of the beam pair `pair` of zone `index`.
__forceinline Node *LaserEndA(int index, int pair)
{
    return DAT_1021bdf8[index][0]->children[pair * 2];
}

__forceinline Node *LaserEndB(int index, int pair)
{
    return DAT_1021bdf8[index][0]->children[pair * 2 + 1];
}

// Puts the two nodes of a pair at the given corners of the zone's box (at its height) and their world matrices there.
__forceinline void PlaceLaserEnds(int index, int pair, const float &ax, const float &ay, const float &bx,
                                  const float &by)
{
    LaserEndA(index, pair)->matrix.SetIdentity();
    LaserEndB(index, pair)->matrix.SetIdentity();
    LaserEndA(index, pair)->matrix.Translate(ax, ay, DAT_10802bf8[index].position.z);
    LaserEndB(index, pair)->matrix.Translate(bx, by, DAT_10802bf8[index].position.z);
    LaserEndA(index, pair)->worldMatrix = LaserEndA(index, pair)->matrix;
    LaserEndB(index, pair)->worldMatrix = LaserEndB(index, pair)->matrix;
}

// Scales a pair's step by the speed factor of the game mode.
__forceinline void ScaleLaserStep(Vec4 &step)
{
    if (DAT_10c635f8 == 1)
        step.Copy(step * LASER_SPEED_EASY);
    else if (DAT_10c635f8 == 2)
        step.Copy(step * LASER_SPEED_HARD);
}

// Sets the y step of a corner chaser: the span of its flag minus the x step already in `step`.
__forceinline void SetCornerStep(int code, Vec4 &step)
{
    if (code / LASER_CODE_FLAG != 0)
        step.y = LASER_CORNER_STEP_LONG - step.x;
    else
        step.y = LASER_CORNER_STEP_SHORT - step.x;
}

// Copies both nodes' own matrices into their world matrices.
__forceinline void UpdateLaserWorld(int index, int pair)
{
    LaserEndA(index, pair)->worldMatrix = LaserEndA(index, pair)->matrix;
    LaserEndB(index, pair)->worldMatrix = LaserEndB(index, pair)->matrix;
}


// 0x1002d4e0 (Char_LaserZone). One frame of the laser zone `index`. First call: finds the model "@tekiatari1" in the
// map, makes sure it has its beam nodes, and reads the box from the map record (corners (x0, y0) and (x1, y1), the
// height z). Later frames count up; once LASER_START_WAIT frames have passed, each frame has a LASER_START_CHANCE in
// 1000 to start a pair in a free slot: a random pattern type, a random speed, the two nodes put at their start
// corners and the beam effect started between them. Every running pair then moves its two nodes by its step (scaled
// by the game mode) until they have crossed the box; the pair is then freed, the zone counts a hit when the player
// stands in the box at that moment, and the effect of the pair is removed. Returns 0 (the caller ignores it).
int Char_LaserZone(int index)
{
    if (DAT_1020b430[index].laserSetup == 0)
    {
        if (DAT_10255f08 != 0)
            DAT_1021bdf8[index][0] = DAT_10255f08->Find("@tekiatari1", 0);
        if (DAT_1021bdf8[index][0] != 0)
        {
            DAT_1020b430[index].laserSetup = 1;
            if (DAT_1021bdf8[index][0]->Find("@laserc", 0) == 0)
            {
                Node unused;
                char name[80];
                for (int i = 0; i < LASER_CHILD_COUNT; i++)
                {
                    sprintf(name, "laserc%02d", i);
                    DAT_1021bdf8[index][0]->AddChild(name, i + 1);
                    LASER_PATTERN(index, i) = 0;
                }
            }
            DAT_1021bdf8[index][0]->SetFlags(LASER_HIDDEN);
            int zone = DAT_10db6ee8[index].zone;
            DAT_10802bf8[index].position.x = (float)DAT_10210fc0[zone].points[0].x;
            DAT_10802bf8[index].position.y = (float)DAT_10210fc0[zone].points[0].y;
            DAT_10802bf8[index].position.z = (float)DAT_10210fc0[zone].points[0].z;
            DAT_10802bf8[index].step.x = (float)DAT_10210fc0[zone].points[1].x;
            DAT_10802bf8[index].step.y = (float)DAT_10210fc0[zone].points[1].y;
            DAT_10802bf8[index].step.z = (float)DAT_10210fc0[zone].points[1].z;
        }
    }
    else
    {
        DAT_1020b430[index].laserFrames++;
        DAT_1020b430[index].laserTimer++;
        if (DAT_1020b430[index].laserTimer >= LASER_START_WAIT && FUN_1013f880() % 1000 < LASER_START_CHANCE)
        {
            DAT_1020b430[index].laserTimer = 0;
            for (int pair = 0; pair < LASER_PATTERN_COUNT; pair++)
            {
                if (LASER_PATTERN(index, pair) != 0)
                    continue;
                switch (FUN_1013f880() % 8)
                {
                case LASER_SWEEP_UP:
                    PlaceLaserEnds(index, pair, DAT_10802bf8[index].position.x, DAT_10802bf8[index].position.y,
                                   DAT_10802bf8[index].step.x, DAT_10802bf8[index].position.y);
                    LASER_PATTERN(index, pair) = (FUN_1013f880() % 9 + 8) * 100 + LASER_SWEEP_UP;
                    break;
                case LASER_SWEEP_DOWN:
                    PlaceLaserEnds(index, pair, DAT_10802bf8[index].position.x, DAT_10802bf8[index].step.y,
                                   DAT_10802bf8[index].step.x, DAT_10802bf8[index].step.y);
                    LASER_PATTERN(index, pair) = (FUN_1013f880() % 9) * 100 + 0x321;
                    break;
                case LASER_SWEEP_RIGHT:
                    PlaceLaserEnds(index, pair, DAT_10802bf8[index].position.x, DAT_10802bf8[index].position.y,
                                   DAT_10802bf8[index].position.x, DAT_10802bf8[index].step.y);
                    LASER_PATTERN(index, pair) = (FUN_1013f880() % 9) * 100 + 0x322;
                    break;
                case LASER_SWEEP_LEFT:
                    PlaceLaserEnds(index, pair, DAT_10802bf8[index].step.x, DAT_10802bf8[index].position.y,
                                   DAT_10802bf8[index].step.x, DAT_10802bf8[index].step.y);
                    LASER_PATTERN(index, pair) = (FUN_1013f880() % 9) * 100 + 0x323;
                    break;
                case LASER_CORNER_LOW_LOW:
                    PlaceLaserEnds(index, pair, DAT_10802bf8[index].position.x, DAT_10802bf8[index].position.y,
                                   DAT_10802bf8[index].position.x, DAT_10802bf8[index].position.y);
                    if (FUN_1013f880() % 2 != 0)
                        LASER_PATTERN(index, pair) = (FUN_1013f880() % 12) * 100 + 0x28a4;
                    else
                        LASER_PATTERN(index, pair) = (FUN_1013f880() % 6) * 100 + 0xcc;
                    break;
                case LASER_CORNER_HIGH_LOW:
                    PlaceLaserEnds(index, pair, DAT_10802bf8[index].step.x, DAT_10802bf8[index].position.y,
                                   DAT_10802bf8[index].step.x, DAT_10802bf8[index].position.y);
                    if (FUN_1013f880() % 2 != 0)
                        LASER_PATTERN(index, pair) = (FUN_1013f880() % 12) * 100 + 0x28a5;
                    else
                        LASER_PATTERN(index, pair) = (FUN_1013f880() % 6) * 100 + 0xcd;
                    break;
                case LASER_CORNER_LOW_HIGH:
                    PlaceLaserEnds(index, pair, DAT_10802bf8[index].position.x, DAT_10802bf8[index].step.y,
                                   DAT_10802bf8[index].position.x, DAT_10802bf8[index].step.y);
                    if (FUN_1013f880() % 2 != 0)
                        LASER_PATTERN(index, pair) = (FUN_1013f880() % 12) * 100 + 0x28a6;
                    else
                        LASER_PATTERN(index, pair) = (FUN_1013f880() % 6) * 100 + 0xce;
                    break;
                case LASER_CORNER_HIGH_HIGH:
                    PlaceLaserEnds(index, pair, DAT_10802bf8[index].step.x, DAT_10802bf8[index].step.y,
                                   DAT_10802bf8[index].step.x, DAT_10802bf8[index].step.y);
                    if (FUN_1013f880() % 2 != 0)
                        LASER_PATTERN(index, pair) = (FUN_1013f880() % 12) * 100 + 0x28a7;
                    else
                        LASER_PATTERN(index, pair) = (FUN_1013f880() % 6) * 100 + 0xcf;
                    break;
                }
                EffectOptions options;
                Vec4 position;
                options.owner = pair + LASER_EFFECT_OWNER;
                options.m_0004 = LASER_EFFECT_VALUE;
                options.m_0000 = 0;
                DAT_10daaa80.Start(LASER_EFFECT, position, -1, -1, &LaserEndA(index, pair)->matrix,
                                   (int)&LaserEndB(index, pair)->matrix, 0, 0, &options, 0);
                break;
            }
        }
        Vec4 step;
        for (int pair = 0; pair < LASER_PATTERN_COUNT; pair++)
        {
            if (LASER_PATTERN(index, pair) == 0)
                continue;
            step.Fill(0.0f);
            int code = LASER_PATTERN(index, pair);
            switch (code % LASER_CODE_TYPE)
            {
            case LASER_SWEEP_UP:
                step.y = (float)(code / LASER_CODE_TYPE % 100);
                ScaleLaserStep(step);
                LaserEndA(index, pair)->matrix.translation.y += step.y;
                LaserEndB(index, pair)->matrix.translation.y += step.y;
                UpdateLaserWorld(index, pair);
                if (LaserEndA(index, pair)->matrix.translation.y > DAT_10802bf8[index].step.y)
                    LASER_PATTERN(index, pair) = 0;
                break;
            case LASER_SWEEP_DOWN:
                step.y = (float)(code / -LASER_CODE_TYPE % 100);
                ScaleLaserStep(step);
                LaserEndA(index, pair)->matrix.translation.y += step.y;
                LaserEndB(index, pair)->matrix.translation.y += step.y;
                UpdateLaserWorld(index, pair);
                if (LaserEndA(index, pair)->matrix.translation.y < DAT_10802bf8[index].position.y)
                    LASER_PATTERN(index, pair) = 0;
                break;
            case LASER_SWEEP_RIGHT:
                step.x = (float)(code / LASER_CODE_TYPE % 100);
                ScaleLaserStep(step);
                LaserEndA(index, pair)->matrix.translation.x += step.x;
                LaserEndB(index, pair)->matrix.translation.x += step.x;
                UpdateLaserWorld(index, pair);
                if (LaserEndA(index, pair)->matrix.translation.x > DAT_10802bf8[index].step.x)
                    LASER_PATTERN(index, pair) = 0;
                break;
            case LASER_SWEEP_LEFT:
                step.x = (float)(code / -LASER_CODE_TYPE % 100);
                ScaleLaserStep(step);
                LaserEndA(index, pair)->matrix.translation.x += step.x;
                LaserEndB(index, pair)->matrix.translation.x += step.x;
                UpdateLaserWorld(index, pair);
                if (LaserEndA(index, pair)->matrix.translation.x < DAT_10802bf8[index].position.x)
                    LASER_PATTERN(index, pair) = 0;
                break;
            case LASER_CORNER_LOW_LOW:
                step.x = (float)(code / LASER_CODE_TYPE % 100);
                SetCornerStep(code, step);
                ScaleLaserStep(step);
                if (LaserEndA(index, pair)->matrix.translation.x > DAT_10802bf8[index].step.x)
                {
                    LaserEndA(index, pair)->matrix.translation.y += step.y;
                    if (LaserEndA(index, pair)->matrix.translation.y > DAT_10802bf8[index].step.y)
                        LASER_PATTERN(index, pair) = 0;
                }
                else
                    LaserEndA(index, pair)->matrix.translation.x += step.x;
                if (LaserEndB(index, pair)->matrix.translation.y > DAT_10802bf8[index].step.y)
                {
                    LaserEndB(index, pair)->matrix.translation.x += step.x;
                    if (LaserEndB(index, pair)->matrix.translation.x > DAT_10802bf8[index].step.x)
                        LASER_PATTERN(index, pair) = 0;
                }
                else
                    LaserEndB(index, pair)->matrix.translation.y += step.y;
                UpdateLaserWorld(index, pair);
                break;
            case LASER_CORNER_HIGH_LOW:
                step.x = (float)(code / LASER_CODE_TYPE % 100);
                SetCornerStep(code, step);
                ScaleLaserStep(step);
                if (LaserEndA(index, pair)->matrix.translation.x < DAT_10802bf8[index].position.x)
                {
                    LaserEndA(index, pair)->matrix.translation.y += step.y;
                    if (LaserEndA(index, pair)->matrix.translation.y > DAT_10802bf8[index].step.y)
                        LASER_PATTERN(index, pair) = 0;
                }
                else
                    LaserEndA(index, pair)->matrix.translation.x -= step.x;
                if (LaserEndB(index, pair)->matrix.translation.y > DAT_10802bf8[index].step.y)
                {
                    LaserEndB(index, pair)->matrix.translation.x -= step.x;
                    if (LaserEndB(index, pair)->matrix.translation.x < DAT_10802bf8[index].position.x)
                        LASER_PATTERN(index, pair) = 0;
                }
                else
                    LaserEndB(index, pair)->matrix.translation.y += step.y;
                UpdateLaserWorld(index, pair);
                break;
            case LASER_CORNER_LOW_HIGH:
                step.x = (float)(code / LASER_CODE_TYPE % 100);
                SetCornerStep(code, step);
                ScaleLaserStep(step);
                if (LaserEndA(index, pair)->matrix.translation.x > DAT_10802bf8[index].step.x)
                {
                    LaserEndA(index, pair)->matrix.translation.y -= step.y;
                    if (LaserEndA(index, pair)->matrix.translation.y < DAT_10802bf8[index].position.y)
                        LASER_PATTERN(index, pair) = 0;
                }
                else
                    LaserEndA(index, pair)->matrix.translation.x += step.x;
                if (LaserEndB(index, pair)->matrix.translation.y < DAT_10802bf8[index].position.y)
                {
                    LaserEndB(index, pair)->matrix.translation.x += step.x;
                    if (LaserEndB(index, pair)->matrix.translation.x > DAT_10802bf8[index].step.x)
                        LASER_PATTERN(index, pair) = 0;
                }
                else
                    LaserEndB(index, pair)->matrix.translation.y -= step.y;
                UpdateLaserWorld(index, pair);
                break;
            case LASER_CORNER_HIGH_HIGH:
                step.x = (float)(code / LASER_CODE_TYPE % 100);
                SetCornerStep(code, step);
                ScaleLaserStep(step);
                if (LaserEndA(index, pair)->matrix.translation.x < DAT_10802bf8[index].position.x)
                {
                    LaserEndA(index, pair)->matrix.translation.y -= step.y;
                    if (LaserEndA(index, pair)->matrix.translation.y < DAT_10802bf8[index].position.y)
                        LASER_PATTERN(index, pair) = 0;
                }
                else
                    LaserEndA(index, pair)->matrix.translation.x -= step.x;
                if (LaserEndB(index, pair)->matrix.translation.y < DAT_10802bf8[index].position.y)
                {
                    LaserEndB(index, pair)->matrix.translation.x -= step.x;
                    if (LaserEndB(index, pair)->matrix.translation.x < DAT_10802bf8[index].position.x)
                        LASER_PATTERN(index, pair) = 0;
                }
                else
                    LaserEndB(index, pair)->matrix.translation.y -= step.y;
                UpdateLaserWorld(index, pair);
                break;
            }
            if (LASER_PATTERN(index, pair) == 0)
            {
                Vec4 player;
                player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
                if (player.x > DAT_10802bf8[index].position.x && player.x < DAT_10802bf8[index].step.x &&
                    player.y > DAT_10802bf8[index].position.y && player.y < DAT_10802bf8[index].step.y)
                    DAT_1020b430[index].laserHits++;
                DAT_10daaa80.RemoveOwnedBy(pair + LASER_EFFECT_OWNER);
            }
        }
    }
    return 0;
}
