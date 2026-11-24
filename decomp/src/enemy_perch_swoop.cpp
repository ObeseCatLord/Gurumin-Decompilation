// The perching swooper (0x100a5960, behaviour 10 of Enemy_UpdateBehaviour 0x100bcb30). A flier that waits on a perch,
// takes off when the player comes near, rises, flies at the player's chest with a smoothed flight vector and flaps its
// wings (two beam effects between four wing matrices), then now and then swoops down to a landing point it found on
// the ground nearby and settles there. See perch_swooper.h for the words it keeps.

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
#include "map_vars.h"
#include "last_position.h"

#include "perch_swooper.h"

extern "C" int FUN_1013f880(void);                                            // the game's random number
extern "C" int FUN_100241e0(int index, float x, float y, float z);            // Character_SetMove
extern "C" int FUN_1003ad20(int sound, const Vec4 *position, float volume);   // Sound_PlayEffect
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                            int value, int frame, const Matrix4 *matrix, const void *options);  // Char_StartEffect

#define PERCH_PI 3.1415f
#define PERCH_TWO_PI 6.283f
#define PERCH_HALF_TURN 180.0f
#define PERCH_SCAN_PERIOD 15            // every 15th frame (offset by the slot) it looks for a landing point
#define PERCH_SCAN_RADIUS 1500          // ... at a random distance below this and a random heading
#define PERCH_HEADING_STEPS 0x168
#define PERCH_SCAN_HEIGHT 500.0f        // the ground is searched this far above and below that point
#define PERCH_GROUND_MASK 0x4000000
#define PERCH_EYE_HEIGHT 75.0f          // the line to the ground point runs this high above both ends
#define PERCH_WALL_MASK 4
#define PERCH_UNSET (-200000.0f)        // x of a landing point that was not found yet
#define PERCH_UNSET_LIMIT (-10000.0f)
#define PERCH_POINT_PERCH 1
#define PERCH_POINT_COUNT 3             // the landing point is one of the vectors 1..3
#define PERCH_IDLE_MOTION 2
#define PERCH_TURN_MOTION 0
#define PERCH_MOTION_BLEND 5
#define PERCH_WAKE_DISTANCE 800.0f
#define PERCH_WAKE_ROLL_RANGE 1000
#define PERCH_WAKE_ROLL 100
#define PERCH_FLAP_GAIN 0.01f
#define PERCH_FLAP_MAX 1.0472f          // 60 degrees in radians
#define PERCH_FLAP_LOSS 0.003f
#define PERCH_CLIMB_HEIGHT 175.0f
#define PERCH_CLIMB_LIFT 10.0f          // the climb adds flap speed * this to the upward step
#define PERCH_CLIMB_FRAMES 0x78
#define PERCH_CHEST_HEIGHT 75.0f        // the flight vector points at this height above the player's feet
#define PERCH_MAX_SPEED 32.0f           // the flight vector is cut to this length
#define PERCH_BLOCKED_SCALE 0.1f        // a blocked flier keeps a tenth of its sideways step
#define PERCH_BLOCKED_DROP 20.0f
#define PERCH_ORBIT_START 90            // from this frame on it may start the swoop (chance wait / 10000 per frame)
#define PERCH_ORBIT_ROLL_RANGE 10000
#define PERCH_ORBIT_END (-0xd2)         // `wait` counts down while it circles, back to 1 after 210 frames
#define PERCH_ORBIT_NEAR 200.0f         // or when the player is nearer than this
#define PERCH_SMOOTH_KEEP 0.95f         // flying: the flight vector keeps this much of itself ...
#define PERCH_SMOOTH_TAKE 0.05f         // ... and takes this much of the new step
#define PERCH_SWOOP_KEEP 0.75f
#define PERCH_SWOOP_TAKE 0.25f
#define PERCH_SWOOP_LIFT 250.0f         // it aims this high above the landing point
#define PERCH_SWOOP_FRAMES 0x78
#define PERCH_SWOOP_END_FRAMES 0x96
#define PERCH_SWOOP_END_DISTANCE 100.0f
#define PERCH_LAND_SMOOTH 0.5f
#define PERCH_LAND_NEAR 75.0f
#define PERCH_LAND_FRAMES 0x1e
#define PERCH_FACING_SQUARED 0.000304503f  // (one degree in radians) squared
#define PERCH_TRAIL_EFFECT 0x45d
#define PERCH_TRAIL_LIFT 75.0f
#define PERCH_TRAIL_SCALE 1.75f
#define PERCH_TRAIL_MODE 5
#define PERCH_FLAP_TICKS_SCALE 50.0f
#define PERCH_FLAP_TICKS 300
#define PERCH_FLAP_SOUND 0x1bf
#define PERCH_FLAP_HEARD_SQUARED 6250000.0f   // the flap is heard within 2500 units (always for the first 58 slots)
#define PERCH_FLAP_HEARD_SLOTS 0x3a
#define PERCH_WING_EFFECT 0x370f
#define PERCH_WING_OUT 95.0f            // wing tip offset sideways ...
#define PERCH_WING_HEIGHT 45.0f
#define PERCH_WING_IN 0.22f             // ... and the inner point as a fraction of it

// 0x100a5960 (Enemy_PerchAndSwoop). Every 15th frame (by slot) while it has not started to swoop it casts down at a
// random spot within 1500 units and, when the ground there is in clear sight, keeps it as landing point 1 or 2 (point
// 3 is the player's position while the player stands still). Steps: start sets perch point 1 and marks points 2 and 3
// unset; perched waits until the player is within 800 units (10 % a frame) and notes his height; climb rises with the
// flap speed until 175 units above that height (or 120 frames); fly follows the player's chest height with a
// smoothed vector, a blocked path steers it up and away, and after 90 frames it may (wait / 10000 a frame) pick a set
// landing point; swoop flies to a point 250 units above it, land settles on it and faces the player, going back to
// perched once its motion is idle. The flap speed grows 0.01 a frame (at most 60 degrees) while flying and turns the
// heading; the flap beat plays a wing sound and two wing beams. Returns 0.
int Enemy_PerchAndSwoop(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    float distance = (player - self).Length();
    (player - self).Length2D();
    int motion = -1;
    int loop = 0;
    Vec4 move;
    int moveFlag = 0;
    if (DAT_104c0380 % PERCH_SCAN_PERIOD == index % PERCH_SCAN_PERIOD && DAT_1020b430[index].state < PERCH_STEP_SWOOP)
    {
        Vec4 offset;
        Vec4 above;
        Vec4 ground;
        offset.y = (float)(FUN_1013f880() % PERCH_SCAN_RADIUS);
        float degrees = (float)(FUN_1013f880() % PERCH_HEADING_STEPS);
        offset.RotateZ(degrees * PERCH_PI / PERCH_HALF_TURN);
        offset.Copy(offset + self);
        above.Copy(offset);
        offset.z = offset.z - PERCH_SCAN_HEIGHT;
        above.z = above.z + PERCH_SCAN_HEIGHT;
        if (DAT_10255f08 != 0 &&
            DAT_10255f08->CastSegment(0, PERCH_GROUND_MASK, &offset, &above, &ground, 0) != 0)
        {
            Vec4 hit;
            offset.Copy(self);
            above.Copy(ground);
            above.z = above.z + PERCH_EYE_HEIGHT;
            offset.z = offset.z + PERCH_EYE_HEIGHT;
            if (DAT_10255f08->Collide(0, PERCH_WALL_MASK, &offset, &above, &hit) == 0)
                LastPositionVector(index * 4 + PERCH_POINT_PERCH + FUN_1013f880() % 2).Copy(ground);
        }
    }
    if (DAT_10db6ee8[0].m_238c != 0 && DAT_1020b430[index].state < PERCH_STEP_SWOOP)
        DAT_10802bf8[index].history3.Copy(player);
    if (DAT_1020b430[index].state == PERCH_STEP_START)
    {
        LastPositionVector(index * 4 + PERCH_POINT_PERCH).Copy(self);
        DAT_10802bf8[index].history2.x = PERCH_UNSET;
        DAT_10802bf8[index].history3.x = PERCH_UNSET;
        DAT_1020b430[index].state = PERCH_STEP_PERCHED;
        motion = PERCH_IDLE_MOTION;
        loop = 1;
    }
    else if (DAT_1020b430[index].state == PERCH_STEP_PERCHED)
    {
        DAT_10db6ee8[index].mode = 1;
        motion = PERCH_IDLE_MOTION;
        if (distance < PERCH_WAKE_DISTANCE && FUN_1013f880() % PERCH_WAKE_ROLL_RANGE < PERCH_WAKE_ROLL)
        {
            DAT_1020b430[index].state = PERCH_STEP_CLIMB;
            DAT_1020b430[index].wait = 0;
            DAT_1020b430[index].startHeight = (int)player.z;
        }
    }
    else if (DAT_1020b430[index].state == PERCH_STEP_CLIMB)
    {
        motion = PERCH_IDLE_MOTION;
        DAT_1020b430[index].wait++;
        float flap = DAT_10255160[index][0] + PERCH_FLAP_GAIN;
        DAT_10255160[index][0] = flap;
        DAT_10db6ee8[index].mode = 0;
        if (flap > PERCH_FLAP_MAX)
            DAT_10255160[index][0] = PERCH_FLAP_MAX;
        DAT_10db6ee8[index].heading = DAT_10db6ee8[index].targetHeading =
            DAT_10255160[index][0] + DAT_10db6ee8[index].targetHeading;
        moveFlag = 1;
        move.z = DAT_10255160[index][0] * PERCH_CLIMB_LIFT + move.z;
        if ((float)DAT_1020b430[index].startHeight + PERCH_CLIMB_HEIGHT < self.z ||
            DAT_1020b430[index].wait > PERCH_CLIMB_FRAMES)
        {
            DAT_1020b430[index].state = PERCH_STEP_FLY;
            DAT_1020b430[index].wait = 0;
            DAT_10802bf8[index].position.Fill(move.z);
        }
    }
    else if (DAT_1020b430[index].state == PERCH_STEP_FLY)
    {
        motion = PERCH_IDLE_MOTION;
        DAT_10db6ee8[index].mode = 0;
        move.z = PERCH_CLIMB_LIFT;
        int blocked = 0;
        if (DAT_10db6ee8[index].IsClearOfMapLimited(&move) == 0)
            blocked = 1;
        move.Copy(player);
        move.z = move.z + PERCH_CHEST_HEIGHT;
        move.Copy(move - self);
        if (blocked)
        {
            move.x = move.x * PERCH_BLOCKED_SCALE;
            move.y = move.y * PERCH_BLOCKED_SCALE;
            if (move.z > PERCH_CLIMB_LIFT)
                move.z = move.z * -1.0f;
            else
                move.z = move.z - PERCH_BLOCKED_DROP;
        }
        if (move.Length() > PERCH_MAX_SPEED)
        {
            move.Normalized3();
            move.Copy(move * PERCH_MAX_SPEED);
        }
        DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position * PERCH_SMOOTH_KEEP + move * PERCH_SMOOTH_TAKE);
        move.Copy(DAT_10802bf8[index].position);
        float flap = DAT_10255160[index][0] + PERCH_FLAP_GAIN;
        DAT_10255160[index][0] = flap;
        if (flap > PERCH_FLAP_MAX)
            DAT_10255160[index][0] = PERCH_FLAP_MAX;
        DAT_10db6ee8[index].heading = DAT_10db6ee8[index].targetHeading =
            DAT_10255160[index][0] + DAT_10db6ee8[index].targetHeading;
        moveFlag = 1;
        Matrix4 matrix;
        EffectOptions options;
        Vec4 offset;
        options.followOwner = 1;
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        options.m_0008 = PERCH_TRAIL_SCALE;
        options.m_000e = PERCH_TRAIL_MODE;
        matrix.SetIdentity();
        matrix.SetTranslation(self);
        matrix.m[MATRIX_TRANSLATION_Z] += PERCH_TRAIL_LIFT;
        FUN_1001d8b0(index, PERCH_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
        if (DAT_1020b430[index].wait > 0)
        {
            DAT_1020b430[index].wait++;
            if (DAT_1020b430[index].wait > PERCH_ORBIT_START &&
                FUN_1013f880() % PERCH_ORBIT_ROLL_RANGE < DAT_1020b430[index].wait)
            {
                DAT_1020b430[index].state = PERCH_STEP_SWOOP;
                DAT_1020b430[index].wait = 0;
                do
                    DAT_1020b430[index].anchorPoint = FUN_1013f880() % PERCH_POINT_COUNT + 1;
                while (!(LastPositionVector(index * 4 + DAT_1020b430[index].anchorPoint).x > PERCH_UNSET_LIMIT));
            }
        }
        else
        {
            DAT_1020b430[index].wait--;
            if (DAT_1020b430[index].wait < PERCH_ORBIT_END || distance < PERCH_ORBIT_NEAR)
                DAT_1020b430[index].wait = 1;
        }
    }
    else if (DAT_1020b430[index].state == PERCH_STEP_SWOOP)
    {
        motion = PERCH_IDLE_MOTION;
        DAT_10db6ee8[index].mode = 0;
        DAT_1020b430[index].wait++;
        move.Copy(LastPositionVector(index * 4 + DAT_1020b430[index].anchorPoint) - self);
        move.z = move.z + PERCH_SWOOP_LIFT;
        if (move.Length() > PERCH_MAX_SPEED)
        {
            move.Normalized3();
            move.Copy(move * PERCH_MAX_SPEED);
        }
        DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position * PERCH_SWOOP_KEEP + move * PERCH_SWOOP_TAKE);
        move.Copy(DAT_10802bf8[index].position);
        float flap = DAT_10255160[index][0] + PERCH_FLAP_GAIN;
        DAT_10255160[index][0] = flap;
        if (flap > PERCH_FLAP_MAX)
            DAT_10255160[index][0] = PERCH_FLAP_MAX;
        DAT_10db6ee8[index].heading = DAT_10db6ee8[index].targetHeading =
            DAT_10255160[index][0] + DAT_10db6ee8[index].targetHeading;
        moveFlag = 1;
        Matrix4 matrix;
        EffectOptions options;
        Vec4 offset;
        options.followOwner = 1;
        options.m_0004 = DAT_10daaae8[index].stats[STAT_EFFECT_PARAM];
        options.m_0008 = PERCH_TRAIL_SCALE;
        options.m_000e = PERCH_TRAIL_MODE;
        matrix.SetIdentity();
        matrix.SetTranslation(self);
        matrix.m[MATRIX_TRANSLATION_Z] += PERCH_TRAIL_LIFT;
        FUN_1001d8b0(index, PERCH_TRAIL_EFFECT, offset, -1, -1, 0, 0, 0, &matrix, &options);
        if (LastPositionVector(index * 4 + DAT_1020b430[index].anchorPoint).z + PERCH_SWOOP_LIFT < self.z ||
            DAT_1020b430[index].wait > PERCH_SWOOP_FRAMES)
        {
            if ((LastPositionVector(index * 4 + DAT_1020b430[index].anchorPoint) - self).Length2D() < PERCH_SWOOP_END_DISTANCE ||
                DAT_1020b430[index].wait > PERCH_SWOOP_END_FRAMES)
            {
                DAT_1020b430[index].wait = 0;
                DAT_1020b430[index].state = PERCH_STEP_LAND;
            }
        }
    }
    else if (DAT_1020b430[index].state == PERCH_STEP_LAND)
    {
        DAT_1020b430[index].wait++;
        DAT_10db6ee8[index].mode = 0;
        move.Copy(LastPositionVector(index * 4 + DAT_1020b430[index].anchorPoint) - self);
        move.z = 0.0f;
        if (move.Length() > PERCH_MAX_SPEED)
        {
            move.Normalized3();
            move.Copy(move * PERCH_MAX_SPEED);
        }
        DAT_10802bf8[index].position.Copy(DAT_10802bf8[index].position * PERCH_LAND_SMOOTH + move * PERCH_LAND_SMOOTH);
        move.Copy(DAT_10802bf8[index].position);
        float flap = DAT_10255160[index][0] - PERCH_FLAP_LOSS;
        DAT_10255160[index][0] = flap;
        if (flap < 0.0f)
            DAT_10255160[index][0] = 0.0f;
        else
            DAT_10db6ee8[index].heading = DAT_10db6ee8[index].targetHeading =
                flap + DAT_10db6ee8[index].targetHeading;
        moveFlag = 1;
        if ((LastPositionVector(index * 4 + DAT_1020b430[index].anchorPoint) - self).Length2D() < PERCH_LAND_NEAR ||
            DAT_1020b430[index].wait > PERCH_LAND_FRAMES)
        {
            DAT_10db6ee8[index].mode = 1;
            if (DAT_10db6ee8[index].m_238c != 0)
            {
                DAT_10db6ee8[index].targetHeading = atan2f(player.x - self.x, player.y - self.y) + PERCH_PI;
                DAT_10255160[index][0] = 0.0f;
            }
            float turn = DAT_10db6ee8[index].heading - DAT_10db6ee8[index].targetHeading;
            while (turn > PERCH_PI)
                turn -= PERCH_TWO_PI;
            while (turn < -PERCH_PI)
                turn += PERCH_TWO_PI;
            if (DAT_10db6ee8[index].m_238c != 0 && turn * turn > PERCH_FACING_SQUARED)
                motion = PERCH_TURN_MOTION;
            else
            {
                motion = PERCH_IDLE_MOTION;
                if (DAT_10db6ee8[index].m_238c != 0)
                {
                    DAT_1020b430[index].state = PERCH_STEP_PERCHED;
                    DAT_1020b430[index].wait = 0;
                }
            }
        }
        else
            motion = PERCH_IDLE_MOTION;
    }
    DAT_1020b430[index].flapTicks += (int)(DAT_10255160[index][0] * PERCH_FLAP_TICKS_SCALE);
    if (DAT_1020b430[index].flapTicks > PERCH_FLAP_TICKS)
    {
        float heardSquared = (DAT_10db6ee8[index].matrix.GetTranslation() - DAT_118e6cb8).LengthSquared3();
        if (heardSquared < PERCH_FLAP_HEARD_SQUARED || index < PERCH_FLAP_HEARD_SLOTS)
        {
            Vec4 spot;
            spot.Set(DAT_10db6ee8[index].matrix.translation.x, DAT_10db6ee8[index].matrix.translation.y,
                     DAT_10db6ee8[index].matrix.translation.z, 0.0f);
            FUN_1003ad20(PERCH_FLAP_SOUND, &spot, 1.0f);
        }
        DAT_1020b430[index].flapTicks -= PERCH_FLAP_TICKS;
        EffectOptions options;
        Vec4 offset;
        options.followOwner = 1;
        DAT_10daaa80.Start(PERCH_WING_EFFECT, offset, -1, -1, &DAT_10868838[index][0], (int)&DAT_10868838[index][1], 0,
                           0, &options, &DAT_10219770[index]);
        DAT_10daaa80.Start(PERCH_WING_EFFECT, offset, -1, -1, &DAT_10868838[index][2], (int)&DAT_10868838[index][3], 0,
                           0, &options, &DAT_10219770[index]);
    }
    Vec4 wing;
    wing.x = PERCH_WING_OUT;
    wing.y = 0.0f;
    wing.z = PERCH_WING_HEIGHT;
    DAT_10868838[index][0].SetIdentity();
    DAT_10868838[index][1].SetIdentity();
    DAT_10868838[index][2].SetIdentity();
    DAT_10868838[index][3].SetIdentity();
    wing.RotateZ(-DAT_10db6ee8[index].heading);
    DAT_10868838[index][0].SetTranslation(wing + self);
    wing.RotateZ(PERCH_PI);
    DAT_10868838[index][2].SetTranslation(wing + self);
    wing.Copy(wing * PERCH_WING_IN);
    wing.z = PERCH_WING_HEIGHT;
    DAT_10868838[index][3].SetTranslation(wing + self);
    wing.RotateZ(PERCH_PI);
    DAT_10868838[index][1].SetTranslation(wing + self);
    if (moveFlag != 0)
        FUN_100241e0(index, move.x, move.y, move.z);
    if (motion > -1)
        DAT_10db6ee8[index].PlayMotion(motion, loop, PERCH_MOTION_BLEND, 1.0f);
    return 0;
}
