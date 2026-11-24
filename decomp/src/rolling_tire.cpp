// An enemy that is a rolling tire (0x100a4440, a per-kind update of 0x100f9670). It rolls over the map steering round
// walls: five probe rays fanned out in front of it (every 0x12th frame, one ray per frame, so each ray is refreshed
// every 18 frames) tell it how far it can go and how close the player is to each ray. Without a sight of the player
// it drives straight on and veers left or right now and then; once the player is nearer than a ray reaches it turns
// onto the best ray towards the player. A wall ahead bounces it back. Its wheel node ("@tire" or the EEQ_25 layer)
// spins and tilts with the speed and the steering, and it leaves a dust effect now and then.

#include <math.h>
#include <stdlib.h>

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

#include "rolling_tire.h"

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_100241e0(int index, float x, float y, float z);            // Character_SetMove
extern "C" int FUN_100242c0(int index, Vec4 *ground, float depth, int fromHead, int testOnly, const Vec4 *offset);
extern "C" int FUN_100246b0(const Vec4 *from, const Vec4 *to, Vec4 *hit, int testOnly, const char *name);
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

// The game's atan2 wrapper (float arguments, so each difference is rounded to a float first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

#define TIRE_WHEEL_NODE "@tire"
#define TIRE_WHEEL_NODE_LAYER "@EEQ_25:Layer1"
#define TIRE_WHEEL_NODE_LAYER_B "@EEQ_25b:Layer1"
#define TIRE_WHEEL_NAME_START 't'      // "@tire" is looked up first; a wheel found by the layer names is not a tire
#define TIRE_STEP_START 0
#define TIRE_STEP_ROAM 1               // drives straight on, now and then veering
#define TIRE_STEP_HUNT 2               // the player is within reach of a ray: turns onto the best one
#define TIRE_RAY_COUNT 5
#define TIRE_FIRST_RAY_WORD 2          // the ray lengths are words 2..6 of the state
#define TIRE_PI 3.1415f
#define TIRE_TWO_PI 6.283f
#define TIRE_STEER_LIMIT 1.0f
#define TIRE_RAY_STEP 0.7854f          // the angle between neighbouring rays
#define TIRE_RAY_SIDE 1.5708f          // the angle of the outermost rays
#define TIRE_RAY_LIFT 75.0f
#define TIRE_RAY_LENGTH (-4000.0f)
#define TIRE_RAY_MASK 4
#define TIRE_AHEAD_LENGTH 500.0f       // the look-ahead that sets wallAhead
#define TIRE_NO_RAY 999999.0f
#define TIRE_SPEED_KEEP 0.92f
#define TIRE_SPEED_ADD 1.44f
#define TIRE_TURN_AFTER_FRAMES 0x96
#define TIRE_TURN_ROLL_RANGE 300
#define TIRE_TURN_ROLL 0x32
#define TIRE_VEER_STEP 1.0f
#define TIRE_VEER_FRAMES 5
#define TIRE_VEER_MIN_FRAMES 10
#define TIRE_WANDER_AFTER_FRAMES 0x5a
#define TIRE_DUST_ROLL_RANGE 1000
#define TIRE_DUST_ROLL 100
#define TIRE_DUST_START (-45)
#define TIRE_DUST_EVERY 15
#define TIRE_DUST_EFFECT 0x463
#define TIRE_DUST_SOUND 0x1c0
#define TIRE_DUST_SCALE 1.5f
#define TIRE_IDLE_MOTION 2
#define TIRE_MOTION_BLEND 5
#define TIRE_WAKE_BLEND 5
#define TIRE_WHEEL_SPIN_RATE 0.01f
#define TIRE_WHEEL_LEAN_FROM_STEER 0.1f
#define TIRE_WHEEL_LEAN_KEEP 0.7f
#define TIRE_STEER_CLAMP 0.5236f
#define TIRE_BODY_OFFSET (-95.0f)
#define TIRE_BODY_LIFT 65.0f
#define TIRE_SMOKE_AHEAD 100.0f
#define TIRE_SMOKE_EVERY 6
#define TIRE_SMOKE_EFFECT 0x5e6
#define TIRE_STEP_LIFT 30.0f
#define TIRE_BOUNCE_FRAMES (-5)
#define TIRE_BOUNCE_COOLDOWN 10
#define TIRE_BOUNCE_FACTOR (-2.0f)
#define TIRE_BOUNCE_ROLL 0x96
#define TIRE_BOUNCE_HALF 0x4b
#define TIRE_HALF_TURN 180.0f
#define TIRE_SPEED_LIMIT 32.0f
#define TIRE_PROBE_SLOTS 0x12          // every ray (and the wall look-ahead) is refreshed once in this many frames
#define TIRE_PROBE_GAP 3               // frames between two probes

// 0x100a4440 (Enemy_RollingTire). `hitFlag` is stored in DAT_1021f6c0 (the caller's hit flag). Every 18th frame one
// of the five rays (angles +-1.5708, +-0.7854 and 0 off its heading, 4000 units long, from 75 units above its feet)
// is cast against the world; the length it reaches (the whole ray when it hits nothing) and the distance of the
// player to the ray are kept in the state. Step 0 finds the wheel node and sets up; step 1 rolls straight on with a
// veer now and then and starts hunting once the player is nearer than any ray reaches; step 2 turns onto the ray
// closest to the player. A dust effect starts every 15th frame of a wander pause. The wheel node is turned with the
// speed and steering, the smoke effect follows, and the character is moved ahead (bouncing back on a wall).
// Returns 0.
int Enemy_RollingTire(int index, char hitFlag)
{
    DAT_10255a88[index] = 0;
    if (DAT_102550e0[index] != 0)
    {
        DAT_102550e0[index] = 0;
        DAT_10db6ee8[index].PlayMotion(TIRE_IDLE_MOTION, 1, TIRE_WAKE_BLEND, 1.0f);
    }
    DAT_1021f6c0[index] = hitFlag;
    Vec4 player;
    Vec4 self;
    Vec4 move;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float distance = (player - self).Length();
    (player - self).Length2D();
    float steer = Atan2(player.x - self.x, player.y - self.y) + TIRE_PI;
    steer = steer - DAT_10db6ee8[index].heading;
    while (steer > TIRE_PI)
        steer -= TIRE_TWO_PI;
    while (steer < -TIRE_PI)
        steer += TIRE_TWO_PI;
    DAT_11b002c0 = DAT_10db6ee8[index].collisionFilter;
    int slot = DAT_104c0380 % TIRE_PROBE_SLOTS;
    if (slot == index % TIRE_PROBE_SLOTS)
    {
        Vec4 from;
        Vec4 to;
        Vec4 end;
        from.Copy(self);
        from.z = from.z + TIRE_RAY_LIFT;
        to.y = TIRE_RAY_LENGTH;
        to.RotateZ(-(DAT_10db6ee8[index].heading - steer - TIRE_RAY_SIDE));
        to.Copy(to + from);
        Vec4 hit;
        if (DAT_10255f08 != 0 && DAT_10255f08->RayCast(0, TIRE_RAY_MASK, 0x4000000, &to, &from, &hit, 0) != 0)
        {
            end.Copy(hit);
            DAT_1020b430[index].rayPlayerGap0 = (int)player.DistanceSquaredToSegment(from, end);
            DAT_1020b430[index].rayLength0 = (int)(from - end).Length();
        }
        else
        {
            DAT_1020b430[index].rayPlayerGap0 = (int)player.DistanceSquaredToSegment(from, to);
            DAT_1020b430[index].rayLength0 = (int)(from - to).Length();
        }
    }
    else if (slot == (index + TIRE_PROBE_GAP) % TIRE_PROBE_SLOTS)
    {
        Vec4 from;
        Vec4 to;
        Vec4 end;
        from.Copy(self);
        from.z = from.z + TIRE_RAY_LIFT;
        to.y = TIRE_RAY_LENGTH;
        to.RotateZ(-(DAT_10db6ee8[index].heading - steer - TIRE_RAY_STEP));
        to.Copy(to + from);
        Vec4 hit;
        if (DAT_10255f08 != 0 && DAT_10255f08->RayCast(0, TIRE_RAY_MASK, 0x4000000, &to, &from, &hit, 0) != 0)
        {
            end.Copy(hit);
            DAT_1020b430[index].rayPlayerGap1 = (int)player.DistanceSquaredToSegment(from, end);
            DAT_1020b430[index].rayLength1 = (int)(from - end).Length();
        }
        else
        {
            DAT_1020b430[index].rayPlayerGap1 = (int)player.DistanceSquaredToSegment(from, to);
            DAT_1020b430[index].rayLength1 = (int)(from - to).Length();
        }
    }
    else if (slot == (index + 2 * TIRE_PROBE_GAP) % TIRE_PROBE_SLOTS)
    {
        Vec4 from;
        Vec4 to;
        Vec4 end;
        from.Copy(self);
        from.z = from.z + TIRE_RAY_LIFT;
        to.y = TIRE_RAY_LENGTH;
        to.RotateZ(-(DAT_10db6ee8[index].heading - steer));
        to.Copy(to + from);
        Vec4 hit;
        if (DAT_10255f08 != 0 && DAT_10255f08->RayCast(0, TIRE_RAY_MASK, 0x4000000, &to, &from, &hit, 0) != 0)
        {
            end.Copy(hit);
            DAT_1020b430[index].rayPlayerGap2 = (int)player.DistanceSquaredToSegment(from, end);
            DAT_1020b430[index].rayLength2 = (int)(from - end).Length();
        }
        else
        {
            DAT_1020b430[index].rayPlayerGap2 = (int)player.DistanceSquaredToSegment(from, to);
            DAT_1020b430[index].rayLength2 = (int)(from - to).Length();
        }
    }
    else if (slot == (index + 3 * TIRE_PROBE_GAP) % TIRE_PROBE_SLOTS)
    {
        Vec4 from;
        Vec4 to;
        Vec4 hit;
        from.Copy(self);
        from.z = from.z + TIRE_RAY_LIFT;
        to.y = TIRE_RAY_LENGTH;
        to.RotateZ(-(DAT_10db6ee8[index].heading - steer + TIRE_RAY_STEP));
        to.Copy(to + from);
        if (FUN_100246b0(&from, &to, &hit, 0, 0) != 0)
        {
            DAT_1020b430[index].rayPlayerGap3 = (int)player.DistanceSquaredToSegment(from, hit);
            DAT_1020b430[index].rayLength3 = (int)(from - hit).Length();
        }
        else
        {
            DAT_1020b430[index].rayPlayerGap3 = (int)player.DistanceSquaredToSegment(from, to);
            DAT_1020b430[index].rayLength3 = (int)(from - to).Length();
        }
    }
    else if (slot == (index + 4 * TIRE_PROBE_GAP) % TIRE_PROBE_SLOTS)
    {
        Vec4 from;
        Vec4 to;
        Vec4 hit;
        from.Copy(self);
        from.z = from.z + TIRE_RAY_LIFT;
        to.y = TIRE_RAY_LENGTH;
        to.RotateZ(-(DAT_10db6ee8[index].heading - steer + TIRE_RAY_SIDE));
        to.Copy(to + from);
        if (FUN_100246b0(&from, &to, &hit, 0, 0) != 0)
        {
            DAT_1020b430[index].rayPlayerGap4 = (int)player.DistanceSquaredToSegment(from, hit);
            DAT_1020b430[index].rayLength4 = (int)(from - hit).Length();
        }
        else
        {
            DAT_1020b430[index].rayPlayerGap4 = (int)player.DistanceSquaredToSegment(from, to);
            DAT_1020b430[index].rayLength4 = (int)(from - to).Length();
        }
    }
    else if (slot == (index + 5 * TIRE_PROBE_GAP) % TIRE_PROBE_SLOTS)
    {
        if (FUN_100242c0(index, 0, TIRE_AHEAD_LENGTH, 1, 1, 0) != 0)
            DAT_1020b430[index].wallAhead = 1;
        else
            DAT_1020b430[index].wallAhead = 0;
    }
    DAT_11b002c0 = 0;
    if (steer > TIRE_STEER_LIMIT)
        steer = TIRE_STEER_LIMIT;
    else if (steer < -TIRE_STEER_LIMIT)
        steer = -TIRE_STEER_LIMIT;
    if (DAT_1020b430[index].state == TIRE_STEP_START)
    {
        DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(TIRE_WHEEL_NODE, 0);
        if (DAT_1021bdf8[index][0] == 0)
        {
            DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(TIRE_WHEEL_NODE_LAYER, 0);
            if (DAT_1021bdf8[index][0] == 0)
                DAT_1021bdf8[index][0] = DAT_10db6ee8[index].root->Find(TIRE_WHEEL_NODE_LAYER_B, 0);
        }
        if (DAT_1021bdf8[index][0] != 0)
            DAT_10802bf8[index].position.Copy(DAT_1021bdf8[index][0]->matrix.GetTranslation());
        DAT_10255160[index][1] = 0;
        DAT_10255160[index][2] = 0;
        DAT_1020b430[index].wait = 0;
        DAT_1020b430[index].rayLength0 = -1;
        DAT_1020b430[index].rayLength1 = -1;
        DAT_1020b430[index].rayLength2 = -1;
        DAT_1020b430[index].rayLength3 = -1;
        DAT_1020b430[index].rayLength4 = -1;
        DAT_1020b430[index].state = TIRE_STEP_ROAM;
    }
    else if (DAT_1020b430[index].state == TIRE_STEP_ROAM)
    {
        float speed = DAT_10255160[index][0] * TIRE_SPEED_KEEP + TIRE_SPEED_ADD;
        move.y = -speed;
        DAT_10255160[index][0] = speed;
        DAT_1020b430[index].driveFrames++;
        move.RotateZ(-DAT_10db6ee8[index].heading);
        if (DAT_1020b430[index].turnDirection == 0)
        {
            if ((DAT_1020b430[index].driveFrames > TIRE_TURN_AFTER_FRAMES &&
                 FUN_1013f880() % TIRE_TURN_ROLL_RANGE < TIRE_TURN_ROLL) ||
                DAT_1020b430[index].driveFrames < 0 || DAT_1020b430[index].wallAhead != 0)
            {
                DAT_1020b430[index].turnDirection = FUN_1013f880() % 2 + 1;
                DAT_1020b430[index].driveFrames = 0;
            }
        }
        else
        {
            do
            {
                float veered;
                if (DAT_1020b430[index].turnDirection == 1)
                    veered = DAT_10db6ee8[index].targetHeading + TIRE_VEER_STEP;
                else if (DAT_1020b430[index].turnDirection == 2)
                    veered = DAT_10db6ee8[index].targetHeading - TIRE_VEER_STEP;
                else
                    break;
                int frames = DAT_1020b430[index].driveFrames;
                DAT_10db6ee8[index].targetHeading = veered;
                if ((frames > TIRE_VEER_MIN_FRAMES && FUN_1013f880() % TIRE_TURN_ROLL_RANGE < TIRE_TURN_ROLL) ||
                    (DAT_1020b430[index].bumpCooldown == 0 && DAT_1020b430[index].driveFrames < 0))
                {
                    DAT_1020b430[index].driveFrames = 0;
                    DAT_1020b430[index].turnDirection = 0;
                }
            } while (0);
        }
        for (int ray = TIRE_FIRST_RAY_WORD; ray < TIRE_FIRST_RAY_WORD + TIRE_RAY_COUNT; ray++)
        {
            if ((float)DAT_1020b430[index].words[ray] > distance)
            {
                DAT_1020b430[index].state = TIRE_STEP_HUNT;
                DAT_1020b430[index].wait = 0;
                break;
            }
        }
    }
    else if (DAT_1020b430[index].state == TIRE_STEP_HUNT)
    {
        float speed = DAT_10255160[index][0] * TIRE_SPEED_KEEP + TIRE_SPEED_ADD;
        move.y = -speed;
        DAT_10255160[index][0] = speed;
        move.RotateZ(-DAT_10db6ee8[index].heading);
        float ahead = (float)DAT_1020b430[index].rayLength2;
        if (ahead > distance)
        {
            if (DAT_1020b430[index].bumpCooldown == 0 && DAT_1020b430[index].driveFrames < 0)
                DAT_1020b430[index].turnDirection = FUN_1013f880() % 2 + 1;
            int frames = DAT_1020b430[index].driveFrames;
            float heading = DAT_10db6ee8[index].heading;
            DAT_1020b430[index].driveFrames = frames + 1;
            if (DAT_1020b430[index].turnDirection == 1)
            {
                DAT_10db6ee8[index].targetHeading = heading - TIRE_VEER_STEP;
                if (frames + 1 > TIRE_VEER_FRAMES)
                {
                    DAT_1020b430[index].turnDirection = 0;
                    DAT_1020b430[index].driveFrames = 0;
                }
            }
            else if (DAT_1020b430[index].turnDirection == 2)
            {
                DAT_10db6ee8[index].targetHeading = heading + TIRE_VEER_STEP;
                if (frames + 1 > TIRE_VEER_FRAMES)
                {
                    DAT_1020b430[index].turnDirection = 0;
                    DAT_1020b430[index].driveFrames = 0;
                }
            }
            else
                DAT_10db6ee8[index].targetHeading = heading - steer;
            if (++DAT_1020b430[index].wait > TIRE_WANDER_AFTER_FRAMES &&
                FUN_1013f880() % TIRE_DUST_ROLL_RANGE < TIRE_DUST_ROLL)
                DAT_1020b430[index].wait = TIRE_DUST_START;
            if (DAT_1020b430[index].wait < 0 && DAT_1020b430[index].wait % TIRE_DUST_EVERY == 0)
            {
                EffectOptions options;
                options.m_0000 = 0;
                options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
                options.m_0008 = TIRE_DUST_SCALE;
                Matrix4 matrix;
                matrix = RotationZ(DAT_10db6ee8[index].heading);
                Vec4 offset;
                FUN_1001d8b0(index, TIRE_DUST_EFFECT, offset, -1, -1, &DAT_10868838[index][0], 0, 0, &matrix, &options);
                FUN_10065b10(index, TIRE_DUST_SOUND, 0, 0);
            }
        }
        else
        {
            int nearest = 0;
            float nearestGap = TIRE_NO_RAY;
            if ((float)DAT_1020b430[index].rayLength0 > distance)
            {
                nearestGap = (float)DAT_1020b430[index].rayPlayerGap0;
                nearest = 2;
            }
            if ((float)DAT_1020b430[index].rayLength1 > distance &&
                (nearest == 0 || nearestGap > (float)DAT_1020b430[index].rayPlayerGap1))
            {
                nearestGap = (float)DAT_1020b430[index].rayPlayerGap1;
                nearest = 3;
            }
            if ((float)DAT_1020b430[index].rayLength2 > distance &&
                (nearest == 0 || nearestGap > (float)DAT_1020b430[index].rayPlayerGap2))
            {
                nearestGap = (float)DAT_1020b430[index].rayPlayerGap2;
                nearest = 4;
            }
            if ((float)DAT_1020b430[index].rayLength3 > distance &&
                (nearest == 0 || nearestGap > (float)DAT_1020b430[index].rayPlayerGap3))
            {
                nearestGap = (float)DAT_1020b430[index].rayPlayerGap3;
                nearest = 5;
            }
            if ((float)DAT_1020b430[index].rayLength4 > distance &&
                (nearest == 0 || nearestGap > (float)DAT_1020b430[index].rayPlayerGap4))
                nearest = 6;
            if (nearest != 0)
            {
                DAT_10db6ee8[index].targetHeading =
                    (DAT_10db6ee8[index].heading - steer) - (float)(nearest - 4) * TIRE_RAY_STEP;
            }
            else
            {
                DAT_1020b430[index].wait = 0;
                DAT_1020b430[index].state = TIRE_STEP_ROAM;
            }
        }
    }
    {
        Vec4 body;
        body.y = TIRE_BODY_OFFSET;
        body.RotateZ(-DAT_10db6ee8[index].heading);
        body.z = body.z + TIRE_BODY_LIFT;
        body.Copy(body + self);
        DAT_10868838[index][0] = TranslationMatrix(body.x, body.y, body.z);
    }
    if (DAT_104c0380 % TIRE_SMOKE_EVERY == index % TIRE_SMOKE_EVERY &&
        (DAT_1021bdf8[index][0] == 0 || DAT_1021bdf8[index][0]->name[0] == TIRE_WHEEL_NAME_START))
    {
        Matrix4 matrix;
        Vec4 smoke;
        smoke.y = TIRE_SMOKE_AHEAD;
        smoke.RotateZ(-DAT_10db6ee8[index].heading);
        smoke.z = smoke.z + TIRE_BODY_LIFT;
        smoke.Copy(smoke + self);
        matrix = RotationZ(DAT_10db6ee8[index].heading);
        matrix.SetTranslation(smoke);
        smoke.Fill(0.0f);
        DAT_10daaa80.Start(TIRE_SMOKE_EFFECT, smoke, -1, -1, 0, 0, 0, &matrix, 0, &DAT_10219770[index]);
    }
    DAT_10db6ee8[index].PlayMotion(TIRE_IDLE_MOTION, 0, TIRE_MOTION_BLEND, 1.0f);
    float facing = DAT_10db6ee8[index].heading;
    float aimed = DAT_10db6ee8[index].targetHeading;
    steer = facing - aimed;
    while (steer > TIRE_PI)
        steer -= TIRE_TWO_PI;
    while (steer < -TIRE_PI)
        steer += TIRE_TWO_PI;
    if (steer > 0.0f)
    {
        if (!(TIRE_STEER_CLAMP > steer))
            steer = TIRE_STEER_CLAMP;
    }
    else if (!(-TIRE_STEER_CLAMP < steer))
        steer = -TIRE_STEER_CLAMP;
    DAT_10db6ee8[index].targetHeading = steer + DAT_10db6ee8[index].heading;
    float spin = (steer + DAT_10255160[index][0]) * TIRE_WHEEL_SPIN_RATE + DAT_10255160[index][1];
    DAT_10255160[index][1] = spin;
    DAT_10255160[index][2] = DAT_10255160[index][2] * TIRE_WHEEL_LEAN_KEEP + steer * TIRE_WHEEL_LEAN_FROM_STEER;
    if (spin > TIRE_TWO_PI)
    {
        while (DAT_10255160[index][1] > TIRE_TWO_PI)
            DAT_10255160[index][1] -= TIRE_TWO_PI;
    }
    if (DAT_10255160[index][1] < 0.0f)
    {
        while (DAT_10255160[index][1] < 0.0f)
            DAT_10255160[index][1] += TIRE_TWO_PI;
    }
    if (DAT_1021bdf8[index][0] != 0)
    {
        DAT_1021bdf8[index][0]->matrix =
            RotationX(-DAT_10255160[index][1]) * RotationZ(DAT_10255160[index][2]) *
            TranslationMatrix(DAT_10802bf8[index].position.x, DAT_10802bf8[index].position.y,
                              DAT_10802bf8[index].position.z);
    }
    if (DAT_10255160[index][0] < 0.0f)
        move.RotateZ(DAT_10255160[index][3]);
    move.z = move.z + TIRE_STEP_LIFT;
    if (DAT_10db6ee8[index].IsClearOfMapLimited(&move) == 0 || DAT_10db6ee8[index].IsClearAny(&move) == 0)
    {
        DAT_1020b430[index].driveFrames = TIRE_BOUNCE_FRAMES;
        DAT_1020b430[index].bumpCooldown = TIRE_BOUNCE_COOLDOWN;
        DAT_10255160[index][0] = DAT_10255160[index][0] * TIRE_BOUNCE_FACTOR;
        DAT_10255160[index][3] = (float)(FUN_1013f880() % TIRE_BOUNCE_ROLL - TIRE_BOUNCE_HALF) * TIRE_PI / TIRE_HALF_TURN;
    }
    else if (DAT_1020b430[index].bumpCooldown > 0)
        DAT_1020b430[index].bumpCooldown--;
    if ((float)fabs(DAT_10255160[index][0]) > TIRE_SPEED_LIMIT)
    {
        if (DAT_10255160[index][0] > 0.0f)
            DAT_10255160[index][0] = TIRE_SPEED_LIMIT;
        else
            DAT_10255160[index][0] = -TIRE_SPEED_LIMIT;
    }
    move.z = move.z - TIRE_STEP_LIFT;
    FUN_100241e0(index, move.x, move.y, move.z);
    return 0;
}
