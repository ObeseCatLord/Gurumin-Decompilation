// EN out-of-line float atan2 wrapper (JP twin: attach_slots.cpp; the JP map called it acosf). The compiler's
// atan2f(float y, float x) inline calls the double atan2; this copy was emitted out of line in the attach-slots object.
// The kind-0x58 character (Char_UpdateAttachSlots) that throws six attachment slots (JP: decomp/src/attach_slots.cpp): each live
// slot flies out, orbits, drifts or homes in on the player; its state is kept in CharState::values and its position and four
// numbers in the character's saved node matrices.

#include <math.h>

#include "anim_model.h"
#include "anti_tamper.h"
#include "attach_slots.h"
#include "char.h"
#include "engine.h"
#include "node.h"
#include "point_transform.h"
#include "vec4.h"

// Heading of the vector (x, y) as a float. Called by the weapon attach pitch/heading code.
extern "C" float Attach_Atan2(float y, float x)
{
    return atan2f(y, x);
}

const int ATTACH_SLOT_COUNT = CHAR_NODE_MATRIX_COUNT;   // slots of one character

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

#define ATTACH_FRAME g_characterNodeMatrices[index][slot]   // the six saved matrices of each character hold its attachment slots

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
    if (g_charState[index].state == 0)
    {
        g_charState[index].state = 1;
        return 0;
    }
    Matrix4 shift;
    Matrix4 basis;
    Vec4 point;
    Vec4 offset;
    for (int slot = 0; slot < ATTACH_SLOT_COUNT; slot++)
    {
        if (g_charState[index].values[slot] != 0)
        {
            shift.SetIdentity();
            basis.SetIdentity();
            point.Fill(0.0f);
            offset.Fill(0.0f);
            float a = ATTACH_FRAME.m[3];
            float b = ATTACH_FRAME.m[7];
            float c = ATTACH_FRAME.m[11];
            float d = ATTACH_FRAME.m[15];
            switch (g_charState[index].values[slot])
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
                ATTACH_FRAME.m[MATRIX_TRANSLATION_X] = point.x;
                ATTACH_FRAME.m[MATRIX_TRANSLATION_Y] = point.y;
                ATTACH_FRAME.m[MATRIX_TRANSLATION_Z] = point.z;
                if (b < ATTACH_FLY_SWITCH_HIGH && b >= ATTACH_FLY_SWITCH_LOW)
                {
                    shift.SetIdentity();
                    offset.Fill(0.0f);
                    shift = ATTACH_FRAME;
                    offset.y = ATTACH_HOME_BACK;
                    shift.SetTranslation(Vec4(0.0f, 0.0f, 0.0f, 0.0f));
                    Point_Transform(&offset, &shift);
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
                    g_charState[index].values[slot] = ATTACH_STATE_HOME;
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
                offset.y = offset.y + ATTACH_FRAME.m[1];
                offset.z = offset.z + ATTACH_FRAME.m[2];
                ATTACH_FRAME.m[12] = offset.x;
                ATTACH_FRAME.m[13] = offset.y;
                ATTACH_FRAME.m[14] = offset.z;
                float timer = ATTACH_FRAME.m[10] + 1.0f;
                ATTACH_FRAME.m[10] = timer;
                if (timer > c)
                    g_charState[index].values[slot] = 0;
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
                    offset.Copy(g_characterModels[0].matrix.GetTranslation() - point);
                    float aim = (float)(Random() % ATTACH_DRIFT_SPREAD - ATTACH_DRIFT_SPREAD_HALF) * ATTACH_PI /
                                    ATTACH_HALF_TURN +
                                (atan2f(offset.x, offset.y) + ATTACH_PI);
                    float away = atan2f(-point.x, -point.y) + ATTACH_PI;
                    float error = aim - away;
                    while (error > ATTACH_PI)
                        error -= ATTACH_TWO_PI;
                    while (error < -ATTACH_PI)
                        error += ATTACH_TWO_PI;
                    if (error > ATTACH_DRIFT_TOLERANCE)
                        aim = (float)(Random() % ATTACH_DRIFT_SPREAD - ATTACH_DRIFT_SPREAD_HALF) * ATTACH_PI /
                                  ATTACH_HALF_TURN +
                              (atan2f(-point.x, -point.y) + ATTACH_PI);
                    heading = aim;
                }
                a -= 1.0f;
                if (a < 0.0f)
                    g_charState[index].values[slot] = 0;
                ATTACH_FRAME.m[3] = a;
                ATTACH_FRAME.m[7] = heading;
                ATTACH_FRAME.m[MATRIX_TRANSLATION_X] = point.x;
                ATTACH_FRAME.m[MATRIX_TRANSLATION_Y] = point.y;
                ATTACH_FRAME.m[MATRIX_TRANSLATION_Z] = point.z;
                break;
            }
            case ATTACH_STATE_HOME:
            {
                point.y = -a;
                point.RotateX(c);
                point.RotateZ(-d);
                if (g_characterModels[index].headNode != 0)
                    offset.Copy(g_characterModels[0].headNode->worldMatrix.GetTranslation() - ATTACH_FRAME.GetTranslation());
                else
                    offset.Copy(g_characterModels[0].matrix.GetTranslation() - ATTACH_FRAME.GetTranslation());
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
                float turnRate;
                if (b > ATTACH_HOME_NEAR_LIFE)
                    turnRate = ATTACH_HOME_FAR_STEP;
                else
                    turnRate = ATTACH_HOME_NEAR_STEP;
                if (g_guardedValue8 == 1)
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
                ATTACH_FRAME.m[MATRIX_TRANSLATION_X] = point.x;
                ATTACH_FRAME.m[MATRIX_TRANSLATION_Y] = point.y;
                ATTACH_FRAME.m[MATRIX_TRANSLATION_Z] = point.z;
                float life = ATTACH_FRAME.m[7] - 1.0f;
                ATTACH_FRAME.m[7] = life;
                if (life < 1.0f)
                    g_charState[index].values[slot] = 0;
                break;
            }
            }
        }
    }
    return 0;
}
