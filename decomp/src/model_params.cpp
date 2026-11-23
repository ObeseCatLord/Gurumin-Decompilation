// Character model parameters (offsets inside the model object of a Character, see world.h).

#include <math.h>
#include <stdlib.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "anim_model.h"

// 0x101ae060: transforms `point` by `matrix` and divides by w (projection to -1..1 screen space).
Vec4 Project(const Vec4 &point, const Matrix4 &matrix);

extern "C" Node *DAT_10faaa88[0x100];
extern "C" int DAT_10ff19e8[0x100];   // per model: its collision data

// Collision test of a model (world matrix) against another model: 0x10193140 (stdcall, recursive over nodes).
extern "C" int __stdcall FUN_10193140(Matrix4 *matrix, Node *other, int otherData, Vec4 *probe, int count,
                                      int *hitCount, int *hitIds, Vec4 *hits, int *hitOther, int mode, int a, int b,
                                      int c, int flags);

extern "C" int DAT_11b002c0;    // collision filter of the current test (the model's collisionFilter)
extern "C" char DAT_11b002c8;   // 1 while a model-against-world test runs

// Sets the model's movement offset (x, y scaled by the model scale) and the movement mode.
int AnimModel::M_101627d0(float x, float y, int mode)
{
    m_23d8.x = x * speedScale;
    *(int *)&m_23d8.z = mode;  // the int mode is stored in the float slot as it is
    m_23d8.y = y * speedScale;
    return 0;
}

// Turns the model to face the direction (x, y): heading = atan2(y, x) + pi (3.1415).
int AnimModel::M_101629b0(float y, float x)
{
    targetHeading = (float)atan2(y, x) + 3.1415f;
    return 1;
}

// Turns the model away from `point`: heading = atan2(dx, dy) of the vector from `point` to the model.
int AnimModel::FaceAway(const Vec4 &point)
{
    Vec4 delta;
    Vec4 unused;  // constructed but never used (the original has it too)
    delta.Copy(matrix.GetTranslation());
    delta.Copy(delta - point);
    targetHeading = (float)atan2(delta.x, delta.y);
    return 0;
}

// Turns the model away from model `other` (heading of the vector from `other` to this model).
int AnimModel::FaceAwayFrom(AnimModel *other)
{
    Vec4 position;
    Vec4 otherPosition;
    position.Copy(matrix.GetTranslation());
    otherPosition.Copy(other->matrix.GetTranslation());
    position.Copy(position - otherPosition);
    targetHeading = (float)atan2(position.x, position.y);
    return 0;
}

// Adds `speed` (scaled by 0.961875 and 0.9, and divided by the model's speed divisor when it has one) to the
// motion playback speed, capped at 60. Used for idle animations (FUN_1001b840 passes 30). Returns 0.
int AnimModel::AddMotionSpeed(float speed)
{
    speed = speed * 0.961875;
    if (speedDivisor != 0)
        speed = speed / speedDivisor;
    m_2388 = 0;
    m_238c = 0;
    speed = speed * 0.9f + motionSpeed;
    motionSpeed = speed;
    if (m_229c == 0)
        m_229c = 8;
    m_2388 = 1;
    if (speed > 60.0f)
        motionSpeed = 60.0f;
    return 0;
}

// Gentler version of AddMotionSpeed used every frame while a push continues: a fifth of `speed` (divided by the
// model's speed divisor) is added only when a speed change is pending (m_238c == 1), the model has no divisor and
// fewer than 16 blend frames are left. The speed is capped at 60 and the frame counter m_2388 advances. Returns 0.
int AnimModel::PushMotionSpeed(float speed)
{
    speed = speed * 0.2f;
    if (speedDivisor != 0)
        speed = speed / speedDivisor;
    if (m_238c == 1 && speedDivisor == 0)
    {
        m_2388 = 0;
        if (m_229c < 16)
            motionSpeed = speed * 0.9f + motionSpeed;
        m_238c = 0;
        if (m_229c == 0)
            m_229c = 8;
    }
    if (motionSpeed > 60.0f)
        motionSpeed = 60.0f;
    m_2388++;
    return 0;
}

// Sets the movement offset to the way from the model's position to (x, y), each axis limited to +-40, and the
// movement mode. Returns 0.
int AnimModel::MoveTowards(float x, float y, int mode)
{
    x = x - matrix.m[12];
    y = y - matrix.m[13];
    m_23d8.x = x;
    m_23d8.y = y;
    *(int *)&m_23d8.z = mode;  // the int mode is stored in the float slot as it is
    if (x > 40.0f)
        m_23d8.x = 40.0f;
    if (m_23d8.x < -40.0f)
        m_23d8.x = -40.0f;
    if (y > 40.0f)
        m_23d8.y = 40.0f;
    if (m_23d8.y < -40.0f)
        m_23d8.y = -40.0f;
    return 0;
}

// Wraps the heading into 0..2pi (2pi = 6.283 here) and snaps it to the nearer side view: pi/2 when it points into the
// first half turn, 3pi/2 otherwise (pi = 3.1415). Returns 0.
int AnimModel::SnapHeadingToSide()
{
    while (targetHeading > 6.283f)
        targetHeading -= 6.283f;
    while (targetHeading < 0.0f)
        targetHeading += 6.283f;
    if (!(targetHeading < 0.0f) && targetHeading < 3.1415f)
    {
        targetHeading = 3.1415f / 2;
        return 0;
    }
    targetHeading = 3.1415f * 1.5f;
    return 0;
}

// Projects a point 50 units above the model's position to 640x480 screen pixels (y grows downwards) and stores
// them in *x and *y. Used to place balloons and markers over characters. Returns 0.
int AnimModel::ScreenPosition(int *x, int *y, const Matrix4 &viewProjection)
{
    Vec4 position;
    position.Copy(matrix.GetTranslation());
    position.z += 50.0f;
    position.Copy(Project(position, viewProjection));
    position.x = (position.x + 1.0f) * 320.0f;
    position.y = (2.0f - (position.y + 1.0f)) * 240.0f;
    *x = (int)position.x;
    *y = (int)position.y;
    return 0;
}

// Gives the model a movement impulse (x, y, z). While a speed change is pending (m_238c == 1) the impulse is 0.7 of
// the value (x and y also scaled by the model scale), or 1.4 of it when the model is nearly at rest (squared speed
// below 1). Otherwise only x and y are set, at 0.55. Returns 0.
int AnimModel::Push(float x, float y, float z)
{
    if (m_238c == 1)
    {
        move.x = x * speedScale * 0.7f;
        move.y = y * speedScale * 0.7f;
        move.z = z * 0.7f;
        if (velocity.LengthSquared3() < 1.0f)
        {
            move.x = x * 1.4f;
            move.y = y * 1.4f;
            move.z = z * 1.4f;
        }
    }
    else
    {
        move.x = x * 0.55f;
        move.y = y * 0.55f;
    }
    return 0;
}

// 1 when the model, moved by `offset` (0 = not moved), touches none of the other loaded models (collision test
// mode -1). Not byte-identical yet (with IsClearAny and IsClearFlagged): the original builds the arguments in edx
// only, ours alternates ecx and edx (see open.tsv).
int AnimModel::IsClear(const Vec4 *offset)
{
    Vec4 probe[2];
    if (offset != 0)
        probe[0].Copy(*offset);
    else
    {
        float zero = 0;
        probe[0].Fill(zero);
    }
    Vec4 hits[0x40];
    int hitOther = 0;
    int hitCount = 0;
    int hitIds[0x40];
    for (int i = 0; i < 0x100; i++)
    {
        if (DAT_10faaa88[i] != 0 && root != DAT_10faaa88[i] && DAT_10faaa88[i]->hasCollision != 0)
        {
            if (FUN_10193140(&matrix, DAT_10faaa88[i], DAT_10ff19e8[i], probe, 1, &hitCount, hitIds, hits, &hitOther,
                             -1, 0, 0, 0, 0) != 0)
                return 0;
        }
    }
    return 1;
}

// Same as IsClear with collision test mode 0.
int AnimModel::IsClearAny(const Vec4 *offset)
{
    Vec4 probe[2];
    if (offset != 0)
        probe[0].Copy(*offset);
    else
    {
        float zero = 0;
        probe[0].Fill(zero);
    }
    Vec4 hits[0x40];
    int hitOther = 0;
    int hitCount = 0;
    int hitIds[0x40];
    for (int i = 0; i < 0x100; i++)
    {
        if (DAT_10faaa88[i] != 0 && root != DAT_10faaa88[i] && DAT_10faaa88[i]->hasCollision != 0 &&
            FUN_10193140(&matrix, DAT_10faaa88[i], DAT_10ff19e8[i], probe, 1, &hitCount, hitIds, hits, &hitOther, 0,
                         0, 0, 0, 0) != 0)
            return 0;
    }
    return 1;
}

// Same as IsClear, testing only nodes with flag 0x400.
int AnimModel::IsClearFlagged(const Vec4 *offset)
{
    Vec4 probe[2];
    if (offset != 0)
        probe[0].Copy(*offset);
    else
    {
        float zero = 0;
        probe[0].Fill(zero);
    }
    Vec4 hits[0x40];
    int hitOther = 0;
    int hitCount = 0;
    int hitIds[0x40];
    for (int i = 0; i < 0x100; i++)
    {
        if (DAT_10faaa88[i] != 0 && root != DAT_10faaa88[i] && DAT_10faaa88[i]->hasCollision != 0 &&
            FUN_10193140(&matrix, DAT_10faaa88[i], DAT_10ff19e8[i], probe, 1, &hitCount, hitIds, hits, &hitOther, -1,
                         0, 0, 0, 0x400) != 0)
            return 0;
    }
    return 1;
}

// 1 when the model, moved by `offset` (0 = not moved), does not touch the map collision of kind 0x4000000.
int AnimModel::IsClearOfMap(const Vec4 *offset)
{
    if (collider == 0)
        return 1;
    Vec4 probe[2];
    if (offset != 0)
        probe[0].Copy(*offset);
    else
        probe[0].Fill(0);
    probe[1].Fill(0);
    Vec4 hitPoint;
    Vec4 hitNormal;
    int out5;
    int out12;
    DAT_11b002c0 = collisionFilter;
    DAT_11b002c8 = 1;
    int hitCount = 0;
    int hit = collider->M_1019f340(0x4000000, &matrix, m_231c, m_2318, &out5, probe, 1, &hitCount, &hitPoint, &hitNormal, 0,
                                   &out12, 0, 0, 0);
    DAT_11b002c8 = 0;
    DAT_11b002c0 = 0;
    return hit == 0;
}

// Same as IsClearOfMap against all collision kinds, with the model's limit m_2390.
int AnimModel::IsClearOfMapLimited(const Vec4 *offset)
{
    if (collider == 0)
        return 1;
    Vec4 probe[2];
    if (offset != 0)
        probe[0].Copy(*offset);
    else
        probe[0].Fill(0);
    probe[1].Fill(0);
    Vec4 hitPoint;
    Vec4 hitNormal;
    int out5;
    int out12;
    DAT_11b002c0 = collisionFilter;
    DAT_11b002c8 = 1;
    int hitCount = 0;
    int hit = collider->M_1019f340(-1, &matrix, m_231c, m_2318, &out5, probe, 1, &hitCount, &hitPoint, &hitNormal, m_2390,
                                   &out12, 0, 0, 0);
    DAT_11b002c8 = 0;
    DAT_11b002c0 = 0;
    return hit == 0;
}

// Starts or continues holding on to model `other` (on = 1): on the first call the offset to `other` is recorded
// (only for models with a "Bip01 Spine" bone), then the hold weight grows by `amount` up to 0.9. With on = 0 a
// running hold is marked released. Returns 0.
int AnimModel::Hold(int on, float amount, AnimModel *other)
{
    if (on != 0)
    {
        if (holdState == 0)
        {
            holdOffset.Fill(0);
            if (root != 0 && root->Find("Bip01 Spine", 0) != 0)
            {
                holdOffset.Copy(other->holdOffset + other->matrix.GetTranslation());
                holdOffset.Copy(holdOffset - matrix.GetTranslation());
            }
        }
        float weight = amount + holdWeight;
        holdWeight = weight;
        if (weight > 0.9f)
            holdWeight = 0.9f;
        holdState = 1;
        return 0;
    }
    if (holdState == 1)
        holdState = 2;
    return 0;
}

// 0x10163c70. Like Hold, but the offset recorded on the first call is the model's own "Bip01 Spine" bone relative to
// its position (0 when there is no such bone). Returns 0.
int AnimModel::HoldAtSpine(int on, float amount)
{
    if (on != 0)
    {
        if (holdState == 0)
        {
            holdOffset.Fill(0);
            if (root != 0)
            {
                Node *spine = root->Find("Bip01 Spine", 0);
                if (spine != 0)
                {
                    holdOffset.Copy(spine->worldMatrix.GetTranslation());
                    holdOffset.Copy(holdOffset - matrix.GetTranslation());
                }
            }
        }
        float weight = amount + holdWeight;
        holdWeight = weight;
        if (weight > 0.9f)
            holdWeight = 0.9f;
        holdState = 1;
        return 0;
    }
    if (holdState == 1)
        holdState = 2;
    return 0;
}

// 0x10161e40. Moves the model to the nearest height that is a multiple of `step` (worked out in tenths of a unit,
// so step and height are rounded towards zero to 0.1), plus `offset`. Returns 0.
int AnimModel::SnapHeight(float step, float offset)
{
    int candidates[4];  // the multiples just below, above and two below; the fourth entry is unused
    Vec4 position = matrix.GetTranslation();
    int height = (int)(position.z * 10.0f);
    int grid = (int)(step * 10.0f);
    int below = height / grid;
    candidates[0] = below * grid;
    candidates[1] = (below + 1) * grid;
    candidates[2] = (below - 1) * grid;
    int best = 0;
    for (int i = 1; i < 3; i++)
    {
        if (abs(candidates[best] - height) > abs(candidates[i] - height))
            best = i;
    }
    position.z = (float)candidates[best] * 0.1f + offset;
    matrix.SetTranslation(position);
    return 0;
}

// 0x10161cb0. Like SnapHeight for x and y: both are moved to the nearest multiple of `step` (in tenths of a unit)
// plus `offset`; the height stays. Returns 0.
int AnimModel::SnapPosition(float step, float offset)
{
    int candidates[4];  // the multiples just below, above and two below; the fourth entry is unused
    Vec4 position = matrix.GetTranslation();
    int x = (int)(position.x * 10.0f);
    int grid = (int)(step * 10.0f);
    int below = x / grid;
    candidates[0] = below * grid;
    candidates[1] = (below + 1) * grid;
    candidates[2] = (below - 1) * grid;
    int best = 0;
    int i;
    for (i = 1; i < 3; i++)
    {
        if (abs(candidates[best] - x) > abs(candidates[i] - x))
            best = i;
    }
    position.x = (float)candidates[best] * 0.1f + offset;
    int y = (int)(position.y * 10.0f);
    below = y / grid;
    candidates[0] = below * grid;
    candidates[1] = (below + 1) * grid;
    candidates[2] = (below - 1) * grid;
    best = 0;
    for (i = 1; i < 3; i++)
    {
        if (abs(candidates[best] - y) > abs(candidates[i] - y))
            best = i;
    }
    position.y = (float)candidates[best] * 0.1f + offset;
    matrix.SetTranslation(position);
    return 0;
}
