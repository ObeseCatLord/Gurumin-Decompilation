// A flying enemy that hovers over the ground (character kind 0x54, per-kind behaviour 0x1006f490 of 0x100f9670). It
// sets itself down a little above the floor, then drifts on a smoothed course that swings round the player (the
// course bends away from the player's bearing and is eased with the last step), tilts into a banking motion when the
// player is off to one side, and now and then, when it faces the player closely enough, fires a shot with a puff of
// smoke from its own position. A shot freezes its course for a moment.

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
#include "char_ground.h"
#include "last_position.h"
#include "map_vars.h"
#include "world.h"

#include "barrage_enemy.h"
#include "hover_shooter.h"

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_10054550(int sound, int delay, const Vec4 &position);       // queued sound
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

#define HOVER_PI 3.1415f
#define HOVER_FULL_TURN 6.283f
#define HOVER_QUARTER_TURN 1.57075f
#define HOVER_AIM_LIMIT 1.570792f       // the bearing error under which a shot may be fired
#define HOVER_UNSET_HEIGHT (-200000)    // hoverHeight before the first frame on the ground
#define HOVER_UNSET_LIMIT (-20000)      // a hoverHeight below this is taken from the model's height
#define HOVER_LOWEST_GROUND (-0xd48)    // ground lower than this: the spawn is dropped
#define HOVER_START_SCALE 0.2f
#define HOVER_FLOAT_HEIGHT 75.0f        // it hovers this high above the ground
#define HOVER_START_DEPTH 10000.0f      // the ground probe starts from this far below the position
#define HOVER_CLIMB 3.0f                // units a frame it rises to its hover height
#define HOVER_SWING_RADIUS 25.0f        // sideways offset of the two bearings it steers between
#define HOVER_SPEED_UP 9.0f
#define HOVER_SMOOTH_NEW 0.15f          // weight of the new course
#define HOVER_SMOOTH_OLD 0.85f          // weight of the last step
#define HOVER_FIRE_RANGE 1500.0f
#define HOVER_FIRE_ROLL_RANGE 10000
#define HOVER_FIRE_DELAY_MIN 30
#define HOVER_FIRE_DELAY_MAX 150
#define HOVER_FREEZE_FRAMES 30
#define HOVER_GROUND_PERIOD 30
#define HOVER_MOTION_BANK_RIGHT 4
#define HOVER_MOTION_BANK_LEFT 2
#define HOVER_MOTION_FLY 7
#define HOVER_MOTION_LAYERS 7
#define HOVER_TRAIL_EFFECT 0x2d4
#define HOVER_TRAIL_PERIOD 8
#define HOVER_SHOT_EFFECT 0x3e9
#define HOVER_SHOT_SOUND 0x2bd
#define HOVER_SMOKE_EFFECT 0x68c
#define HOVER_SMOKE_FRAME 0x14
#define HOVER_SMOKE_SOUND 0x2be
#define HOVER_SHOT_OFFSET (-25.0f)

// 0x1006f490 (Enemy_HoverAndShoot). The first frame sets the character down: it probes the ground under it (a
// shadow-matrix lookup through Char_PlaceOnGround), drops the spawn when the ground is lower than -3400 (returns 0),
// else hovers 75 units above the ground and returns 1. Later frames lower or lift the character to its hover height
// (3 units a frame), then steer: the bearing error to the player picks a course that swings 25 units to the side of
// it, scaled by 9 * DAT_10255160[index][2] and blended 15 % with the last step (85 %); a character facing the player
// within about 90 degrees stops, a frozen one (after a shot) stays still. The banking motion (2, 4 or 7) follows
// the sign of the error. Every 8th motion frame leaves a trail effect, and when aligned and near it fires the shot
// effect (0x3e9) with smoke (0x68c) and sound at random (the chance grows with the frames since the last shot).
// Returns 1 on the setting-down frame or while DAT_10210d30 is set, else 0.
int Enemy_HoverAndShoot(int index)
{
    DAT_10db6ee8[index].m_0012 = 2;
    if (DAT_1020b430[index].hoverReady == 0)
    {
        DAT_1020b430[index].hoverHeight = HOVER_UNSET_HEIGHT;
        Vec4 spare;               // the original declares three Vec4 here and only uses `start`
        if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
            DAT_10db6ee8[index].mode = 0;
        DAT_10218fc0[index] = 0;
        DAT_10db6ee8[index].locked = 1;
        DAT_10255a88[index] = 0;
        DAT_10db6ee8[index].uniformScale = HOVER_START_SCALE;
        DAT_10802bf8[index].position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        DAT_10255160[index][3] = HOVER_QUARTER_TURN - DAT_10db6ee8[index].heading;
        DAT_10255160[index][2] = 2.0f;
        Vec4 start;
        Vec4 below;
        start.Copy(DAT_10802bf8[index].position);
        start.z = start.z - HOVER_START_DEPTH;
        DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
        FUN_1001c090(index, 1);
        int ground = (int)DAT_10db6ee8[index].shadowMatrix.m[14];
        DAT_1020b430[index].floorHeight = ground;
        DAT_11b002c0 = 0;
        if (ground < HOVER_LOWEST_GROUND)
            return 0;
        float groundZ = (float)ground;
        DAT_10db6ee8[index].mode = 1;
        DAT_10255a88[index] = 1;
        DAT_10db6ee8[index].SetPosition(DAT_10802bf8[index].position.x, DAT_10802bf8[index].position.y,
                                        groundZ + HOVER_FLOAT_HEIGHT);
        DAT_1020b430[index].shotTimer = 0;
        DAT_10db6ee8[index].m_238c = 0;
        DAT_1020b430[index].shotDelayMin = HOVER_FIRE_DELAY_MIN;
        DAT_1020b430[index].shotDelayMax = HOVER_FIRE_DELAY_MAX;
        DAT_1020b430[index].hoverReady = 1;
        return 1;
    }
    DAT_10db6ee8[index].locked = 1;
    DAT_10db6ee8[index].uniformScale = 1.0f;
    if (DAT_10210d30[index] != 0)
        return 1;
    if (DAT_10db6ee8[index].mode != 0 && DAT_10db6ee8[index].m_238c == 0)
        return 0;
    if (DAT_1020b430[index].hoverHeight < HOVER_UNSET_LIMIT)
        DAT_1020b430[index].hoverHeight = (int)DAT_10db6ee8[index].matrix.m[14];
    if (DAT_10daaae8[index].stats[STAT_HEALTH] > 0)
    {
        Vec4 at;
        at.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
        float height = (float)DAT_1020b430[index].hoverHeight;
        if (at.z > height)
            DAT_10db6ee8[index].mode = 1;
        else
        {
            DAT_10db6ee8[index].m_238c = 0;
            DAT_10db6ee8[index].mode = 0;
            if (height > at.z)
            {
                at.z = at.z + HOVER_CLIMB;
                if (at.z > height)
                    at.z = height;
                DAT_10db6ee8[index].SetPosition(at.x, at.y, at.z);
            }
        }
    }
    if (DAT_104483e8[index] == 0)
        return 0;
    if (DAT_10210b30[index] != 0)
        DAT_10210b30[index] = 0;
    DAT_10218fc0[index] = 0;
    Vec4 player;
    Vec4 self;
    Vec4 toPlayer;
    Vec4 course;
    int canShoot = 1;
    EffectOptions options;
    Vec4 shotPosition;
    Matrix4 shot;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    toPlayer.Copy(player - self);
    toPlayer.Length();
    float distance = toPlayer.Length2D();
    float angle = (float)atan2((double)toPlayer.x, (double)toPlayer.y);
    float error = angle + HOVER_PI - DAT_10db6ee8[index].targetHeading;
    while (error > HOVER_PI)
        error = error - HOVER_FULL_TURN;
    while (error < -HOVER_PI)
        error = error + HOVER_FULL_TURN;
    if (DAT_1020b430[index].shotMirrored != 0)
        error = error * -1.0f;
    if ((int)DAT_104c0380 % HOVER_GROUND_PERIOD == index % HOVER_GROUND_PERIOD)
    {
        DAT_10db6ee8[index].m_0012 = 2;
        FUN_1001c090(index, 1);
    }
    float scale;
    float bearingA;
    if (error > 0.0f)
    {
        course.x = HOVER_SWING_RADIUS;
        course.RotateZ(DAT_10255160[index][3] + HOVER_QUARTER_TURN);
        bearingA = atan2f(player.x - (self.x + course.x), player.y - (self.y + course.y)) + HOVER_PI -
                   DAT_10db6ee8[index].targetHeading;
        angle = atan2f(player.x - (self.x - course.x), player.y - (self.y - course.y)) + HOVER_PI -
                DAT_10db6ee8[index].targetHeading;
        course.Normalized3();
        scale = HOVER_SPEED_UP;
        course.Copy(course * scale * DAT_10255160[index][2]);
    }
    else if (error < 0.0f)
    {
        course.x = -HOVER_SWING_RADIUS;
        course.RotateZ(DAT_10255160[index][3] + HOVER_QUARTER_TURN);
        bearingA = atan2f(player.x - (self.x - course.x), player.y - (self.y - course.y)) + HOVER_PI -
                   DAT_10db6ee8[index].targetHeading;
        angle = atan2f(player.x - (self.x + course.x), player.y - (self.y + course.y)) + HOVER_PI -
                DAT_10db6ee8[index].targetHeading;
        course.Normalized3();
        scale = HOVER_SPEED_UP;
        course.Copy(course * scale * DAT_10255160[index][2]);
    }
    else
    {
        scale = 0.0f;
        course.Fill(scale);
        bearingA = 0.0f;
        angle = 0.0f;
    }
    while (bearingA > HOVER_PI)
        bearingA = bearingA - HOVER_FULL_TURN;
    while (bearingA < -HOVER_PI)
        bearingA = bearingA + HOVER_FULL_TURN;
    while (angle > HOVER_PI)
        angle = angle - HOVER_FULL_TURN;
    while (angle < -HOVER_PI)
        angle = angle + HOVER_FULL_TURN;
    if (DAT_1020b430[index].shotMirrored != 0)
    {
        bearingA = bearingA * -1.0f;
        angle = angle * -1.0f;
    }
    DAT_1020b430[index].shotAligned = 0;
    if (bearingA < 0.0f && angle > 0.0f && floorf(error) < HOVER_AIM_LIMIT)
    {
        scale = 0.0f;
        course.Fill(scale);
        DAT_1020b430[index].shotAligned = 1;
        error = 0.0f;
        canShoot = 0;
    }
    if (DAT_10db6ee8[index].IsClearOfMapLimited(&course) == 0 || floorf(error) > HOVER_AIM_LIMIT)
    {
        error = 0.0f;
        scale = 0.0f;
        course.Fill(scale);
        canShoot = 0;
    }
    if (DAT_1020b430[index].frozenFrames != 0)
    {
        DAT_1020b430[index].frozenFrames--;
        error = 0.0f;
        scale = 0.0f;
        course.Fill(scale);
        canShoot = 0;
    }
    scale = HOVER_SMOOTH_OLD;
    angle = HOVER_SMOOTH_NEW;
    course.Copy(course * angle + DAT_10802bf8[index].step * scale);
    if ((error > 0.0f && DAT_1020b430[index].shotMirrored == 0) ||
        (error < 0.0f && DAT_1020b430[index].shotMirrored != 0))
        DAT_1020b430[index].hoverMotion = HOVER_MOTION_BANK_RIGHT;
    else if ((error < 0.0f && DAT_1020b430[index].shotMirrored == 0) ||
             (error > 0.0f && DAT_1020b430[index].shotMirrored != 0))
        DAT_1020b430[index].hoverMotion = HOVER_MOTION_BANK_LEFT;
    else
        DAT_1020b430[index].hoverMotion = HOVER_MOTION_FLY;
    int layer = DAT_10db6ee8[index].motion->layer;
    if (layer > -1 && layer < HOVER_MOTION_LAYERS)
        DAT_1020b430[index].motionFrameSaved = DAT_10db6ee8[index].motionFrame;
    DAT_10802bf8[index].step.Copy(course);
    if (DAT_104bfd08 == 0 && DAT_10db6ee8[index].IsClearOfMapLimited(&course) != 0)
        DAT_10db6ee8[index].SetPosition(self.x + course.x, course.y + self.y, course.z + self.z);
    if (DAT_1020b430[index].hoverMotion > -1)
    {
        DAT_10db6ee8[index].PlayMotion(DAT_1020b430[index].hoverMotion, 0, 0, 1.0f);
        if (DAT_1020b430[index].hoverMotion == HOVER_MOTION_FLY)
        {
            layer = DAT_1020b430[index].motionFrameSaved;
            DAT_10db6ee8[index].motionFrame = layer;
            DAT_10db6ee8[index].motion->m_0004 = (float)layer;
            DAT_10db6ee8[index].motion->frame = DAT_10db6ee8[index].motionFrame;
        }
        else if (layer != DAT_1020b430[index].hoverMotion && layer == HOVER_MOTION_FLY)
        {
            DAT_10db6ee8[index].motion->m_0004 = (float)DAT_10db6ee8[index].motionFrame;
            DAT_10db6ee8[index].motion->frame = DAT_10db6ee8[index].motionFrame;
        }
    }
    if (canShoot != 0 && DAT_10db6ee8[index].motionFrame % HOVER_TRAIL_PERIOD == 0)
    {
        shot.SetIdentity();
        shot.SetTranslation(DAT_10db6ee8[index].matrix.GetTranslation());
        shot.m[14] = (float)DAT_1020b430[index].floorHeight;
        scale = 0.0f;
        shotPosition.Fill(scale);
        FUN_1001d8b0(index, HOVER_TRAIL_EFFECT, shotPosition, -1, -1, 0, 0, 0, &shot, 0);
    }
    DAT_1020b430[index].shotTimer++;
    if (DAT_1020b430[index].shotAligned != 0 && distance < HOVER_FIRE_RANGE &&
        (DAT_1030b8f8[index] != 0 || DAT_10254598[index] < HOVER_GROUND_PERIOD) &&
        DAT_1020b430[index].shotTimer > DAT_1020b430[index].shotDelayMin &&
        FUN_1013f880() % HOVER_FIRE_ROLL_RANGE <
            (DAT_1020b430[index].shotDelayMax - DAT_1020b430[index].shotDelayMin) + DAT_1020b430[index].shotTimer)
    {
        DAT_1020b430[index].shotTimer = 0;
        DAT_1020b430[index].frozenFrames = HOVER_FREEZE_FRAMES;
        shot.SetIdentity();
        shotPosition.y = HOVER_SHOT_OFFSET;
        shotPosition.RotateZ(-DAT_10db6ee8[index].heading);
        shotPosition.Copy(shotPosition + DAT_10db6ee8[index].matrix.GetTranslation());
        shot = RotationZ(DAT_10db6ee8[index].heading);
        shot.SetTranslation(shotPosition);
        distance = 0.0f;
        shotPosition.Fill(distance);
        options.m_0000 = 0;
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        FUN_1001d8b0(index, HOVER_SHOT_EFFECT, shotPosition, -1, -1, 0, 0, 0, &shot, &options);
        FUN_10065b10(index, HOVER_SHOT_SOUND, 0, 0);
        FUN_1001d8b0(index, HOVER_SMOKE_EFFECT, shotPosition, -1, -1, 0, 0, HOVER_SMOKE_FRAME, &shot, &options);
        FUN_10054550(HOVER_SMOKE_SOUND, HOVER_SMOKE_FRAME, shot.GetTranslation());
    }
    return 0;
}
