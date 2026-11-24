// The six attachment slots of a kind-0x58 character (run from the per-kind update 0x100f9670; FUN_1001c410 hands out the
// slots). Each slot is a small projectile: its state is CharState::values[slot], its data lives in the slot's matrix
// DAT_10868838[index][slot] (the first three words are the point it started from, the fourth column holds four
// numbers, the translation is where it is now). State 1 flies out from the start point along a heading, turns into
// a homing flight over the player when its life runs out, state 2 circles the start point, state 3 drifts about the
// map and state 4 homes in on the player (or the player's head).

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "node.h"

#include "char_state.h"

#include "char_slot.h"

#include "char_tables.h"

#include "char_vars.h"

#include "attach_slots.h"

extern "C" int FUN_1013f880(void);                // the game's random number
extern "C" int FUN_1001c510(Vec4 *point, Matrix4 *matrix);  // Point_Transform

// The matrix of the slot being run.
#define ATTACH_FRAME DAT_10868838[index][slot]
#define ATTACH_SLOT_COUNT 6
#define ATTACH_STATE_FLY 1
#define ATTACH_STATE_ORBIT 2
#define ATTACH_STATE_DRIFT 3
#define ATTACH_STATE_HOME 4
#define ATTACH_FLY_SWITCH_HIGH 2.0f        // the flight turns into a homing flight when its life is in [1, 2)
#define ATTACH_FLY_SWITCH_LOW 1.0f
#define ATTACH_HOME_BACK -10.0f            // the homing flight starts from this far behind
#define ATTACH_HOME_SPEED 20.0f
#define ATTACH_HOME_LIFE 77.0f
#define ATTACH_DRIFT_CEILING 70.0f
#define ATTACH_DRIFT_SINK -5.0f
#define ATTACH_DRIFT_RANGE 1700.0f
#define ATTACH_DRIFT_SPREAD 90
#define ATTACH_DRIFT_SPREAD_HALF 45
#define ATTACH_DRIFT_TOLERANCE 0.7854f
#define ATTACH_HOME_MIN_DISTANCE 1.0f
#define ATTACH_HOME_FAR_STEP 10.0f
#define ATTACH_HOME_NEAR_STEP 5.0f
#define ATTACH_HOME_NEAR_LIFE 44.0f
#define ATTACH_HOME_EASY_SCALE 0.75f
#define ATTACH_HOME_STEP_RADIANS 0.01745f
#define ATTACH_HOME_ACCELERATION 2.0f
#define ATTACH_PI 3.1415f
#define ATTACH_TWO_PI 6.283f
#define ATTACH_HALF_TURN 180.0f

// 0x1001c670 (Char_UpdateAttachSlots; C++ linkage: the original has an EH frame). One frame of the kind-0x58 character
// `index`; the first call only sets its state to 1. Every live slot of its six runs its state (the slot's four numbers
// a, b, c, d are m[3], m[7], m[11], m[15] of its matrix, b is the life counted down):
// 1: it flies out along a distance `a` from the start point in the matrix's direction and loses a life; with life in
//    [1, 2) it becomes a homing flight: a = 20, life 77, the angles c (pitch) and d (heading) point away from the
//    start point's offset.
// 2: it circles the start point at radius d, the angle turning with the counter m[10] up to c frames.
// 3: it drifts (offset (0, -c, 0) turned by -b), aims at the player with a random error when it is over 1700 units
//    from the map origin, loses a life of `a` and ends when `a` is below zero.
// 4: it homes in on the player's head (the character's own head node when it has one, else the player): pitch c and
//    heading d turn towards the player by at most 10 degrees a frame (5 once the life is 44 or less, 0.75 times that
//    with DAT_10c635f8 == 1), it moves `a` along them, `a` grows by 2 and the life shrinks until it is below 1.
// Returns 0.
int Char_UpdateAttachSlots(int index)
{
    if (DAT_1020b430[index].state == 0)
    {
        DAT_1020b430[index].state = 1;
        return 0;
    }
    Matrix4 shift;
    Matrix4 basis;
    Vec4 point;
    Vec4 offset;
    for (int slot = 0; slot < ATTACH_SLOT_COUNT; slot++)
    {
        if (DAT_1020b430[index].values[slot] != 0)
        {
            shift.SetIdentity();
            basis.SetIdentity();
            float zero = 0.0f;
            point.Fill(zero);
            float zeroOffset = 0.0f;
            offset.Fill(zeroOffset);
            float a = ATTACH_FRAME.m[3];
            float b = ATTACH_FRAME.m[7];
            float c = ATTACH_FRAME.m[11];
            float d = ATTACH_FRAME.m[15];
            switch (DAT_1020b430[index].values[slot])
            {
            case ATTACH_STATE_FLY:
            {
                shift.SetTranslation(Vec4(0.0f, a, 0.0f, 0.0f));
                basis = ATTACH_FRAME;
                basis.SetTranslation(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
                shift = shift * basis;
                point.Copy(shift.GetTranslation() + ATTACH_FRAME.GetTranslation());
                b -= 1.0f;
                ATTACH_FRAME.m[3] = a;
                ATTACH_FRAME.m[7] = b;
                ATTACH_FRAME.translation.x = point.x;
                ATTACH_FRAME.translation.y = point.y;
                ATTACH_FRAME.translation.z = point.z;
                if (b < ATTACH_FLY_SWITCH_HIGH && b >= ATTACH_FLY_SWITCH_LOW)
                {
                    shift.SetIdentity();
                    float zeroBack = 0.0f;
                    offset.Fill(zeroBack);
                    shift = ATTACH_FRAME;
                    offset.y = ATTACH_HOME_BACK;
                    shift.SetTranslation(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
                    FUN_1001c510(&offset, &shift);
                    ATTACH_FRAME.SetIdentity();
                    ATTACH_FRAME.SetTranslation(point);
                    ATTACH_FRAME.m[3] = ATTACH_HOME_SPEED;
                    ATTACH_FRAME.m[7] = ATTACH_HOME_LIFE;
                    float pitch;
                    if (offset.z < 0.0f)
                        pitch = -acosf(offset.Length2D() / offset.Length());
                    else
                        pitch = acosf(offset.Length2D() / offset.Length());
                    while (pitch > ATTACH_PI)
                        pitch -= ATTACH_TWO_PI;
                    while (pitch < -ATTACH_PI)
                        pitch += ATTACH_TWO_PI;
                    ATTACH_FRAME.m[11] = pitch;
                    float heading = atan2f(offset.x, offset.y) + ATTACH_PI;
                    while (heading > ATTACH_PI)
                        heading -= ATTACH_TWO_PI;
                    while (heading < -ATTACH_PI)
                        heading += ATTACH_TWO_PI;
                    ATTACH_FRAME.m[15] = heading;
                    DAT_1020b430[index].values[slot] = ATTACH_STATE_HOME;
                }
                break;
            }
            case ATTACH_STATE_ORBIT:
            {
                // the original keeps the start point in `point` and then reads the matrix again
                point.x = ATTACH_FRAME.m[0];
                point.y = ATTACH_FRAME.m[1];
                point.z = ATTACH_FRAME.m[2];
                offset.y = -d;
                offset.RotateZ(-ATTACH_FRAME.m[3] - b * ATTACH_FRAME.m[10]);
                offset.z = ATTACH_FRAME.m[6] * ATTACH_FRAME.m[10];
                offset.x = offset.x + ATTACH_FRAME.m[0];
                ATTACH_FRAME.m[12] = offset.x;
                offset.y = offset.y + ATTACH_FRAME.m[1];
                ATTACH_FRAME.m[13] = offset.y;
                offset.z = offset.z + ATTACH_FRAME.m[2];
                ATTACH_FRAME.m[14] = offset.z;
                float timer = ATTACH_FRAME.m[10] + 1.0f;
                ATTACH_FRAME.m[10] = timer;
                if (timer > c)
                    DAT_1020b430[index].values[slot] = 0;
                break;
            }
            case ATTACH_STATE_DRIFT:
            {
                point.Copy(ATTACH_FRAME.GetTranslation());
                offset.y = -c;
                offset.RotateZ(-b);
                if (point.z > ATTACH_DRIFT_CEILING)
                    offset.z = ATTACH_DRIFT_SINK;
                point.Copy(point + offset);
                float heading = b;
                if (point.Length2D() > ATTACH_DRIFT_RANGE)
                {
                    Vec4 spare;  // the original constructs it and never reads it
                    offset.Copy(DAT_10db6ee8[0].matrix.GetTranslation() - point);
                    float aim = atan2f(offset.x, offset.y) +
                                (float)(FUN_1013f880() % ATTACH_DRIFT_SPREAD - ATTACH_DRIFT_SPREAD_HALF) * ATTACH_PI /
                                    ATTACH_HALF_TURN +
                                ATTACH_PI;
                    float away = atan2f(-point.x, -point.y) + ATTACH_PI;
                    float error = aim - away;
                    while (error > ATTACH_PI)
                        error -= ATTACH_TWO_PI;
                    while (error < -ATTACH_PI)
                        error += ATTACH_TWO_PI;
                    if (error > ATTACH_DRIFT_TOLERANCE)
                        aim = (float)(FUN_1013f880() % ATTACH_DRIFT_SPREAD - ATTACH_DRIFT_SPREAD_HALF) * ATTACH_PI /
                                  ATTACH_HALF_TURN +
                              atan2f(-point.x, -point.y) + ATTACH_PI;
                    heading = aim;
                }
                a -= 1.0f;
                if (a < 0.0f)
                    DAT_1020b430[index].values[slot] = 0;
                ATTACH_FRAME.m[3] = a;
                ATTACH_FRAME.m[7] = heading;
                ATTACH_FRAME.translation.x = point.x;
                ATTACH_FRAME.translation.y = point.y;
                ATTACH_FRAME.translation.z = point.z;
                break;
            }
            case ATTACH_STATE_HOME:
            {
                point.y = -a;
                point.RotateX(c);
                point.RotateZ(-d);
                if (DAT_10db6ee8[index].headNode != 0)
                    offset.Copy(DAT_10db6ee8[0].headNode->worldMatrix.GetTranslation() - ATTACH_FRAME.GetTranslation());
                else
                    offset.Copy(DAT_10db6ee8[0].matrix.GetTranslation() - ATTACH_FRAME.GetTranslation());
                float pitchError;
                if (offset.Length() < ATTACH_HOME_MIN_DISTANCE)
                    pitchError = 0.0f;
                else if (offset.z < 0.0f)
                    pitchError = -acosf(offset.Length2D() / offset.Length());
                else
                    pitchError = acosf(offset.Length2D() / offset.Length());
                float yawError = (float)atan2(offset.x, offset.y) + ATTACH_PI;
                yawError = yawError - d;
                pitchError = pitchError - c;
                while (yawError > ATTACH_PI)
                    yawError -= ATTACH_TWO_PI;
                while (yawError < -ATTACH_PI)
                    yawError += ATTACH_TWO_PI;
                while (pitchError > ATTACH_PI)
                    pitchError -= ATTACH_TWO_PI;
                while (pitchError < -ATTACH_PI)
                    pitchError += ATTACH_TWO_PI;
                float turnRate = ATTACH_HOME_FAR_STEP;
                if (!(b > ATTACH_HOME_NEAR_LIFE))
                    turnRate = ATTACH_HOME_NEAR_STEP;
                if (DAT_10c635f8 == 1)
                    turnRate = turnRate * ATTACH_HOME_EASY_SCALE;
                float step = turnRate * ATTACH_HOME_STEP_RADIANS;
                float yawAbs = (float)fabs(yawError);
                if (yawAbs < step)
                    d = yawError + d;
                else if (yawError > 0.0f)
                    d = step + d;
                else if (yawError < 0.0f)
                    d = d - step;
                float pitchAbs = (float)fabs(pitchError);
                if (pitchAbs < step)
                    c = pitchError + c;
                else if (pitchError > 0.0f)
                    c = step + c;
                else if (pitchError < 0.0f)
                    c = c - step;
                ATTACH_FRAME.m[11] = c;
                ATTACH_FRAME.m[3] = ATTACH_FRAME.m[3] + ATTACH_HOME_ACCELERATION;
                ATTACH_FRAME.m[15] = d;
                point.Copy(point + ATTACH_FRAME.GetTranslation());
                float life = ATTACH_FRAME.m[7] - 1.0f;
                ATTACH_FRAME.translation.x = point.x;
                ATTACH_FRAME.translation.y = point.y;
                ATTACH_FRAME.translation.z = point.z;
                ATTACH_FRAME.m[7] = life;
                if (life < 1.0f)
                    DAT_1020b430[index].values[slot] = 0;
                break;
            }
            }
        }
    }
    return 0;
}
