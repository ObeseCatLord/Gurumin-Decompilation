// Character model parameters (offsets inside the model object of a Character, see world.h).

#include <math.h>
#include <stdlib.h>

#include "vec4.h"

#include "matrix4.h"

// 0x101ae060: transforms `point` by `matrix` and divides by w (projection to -1..1 screen space).
Vec4 Project(const Vec4 &point, const Matrix4 &matrix);

// Model table (anim_model.cpp: DAT_10faaa88): the loaded models, each with its collision state in m_00a8.
struct LoadedModel
{
    char pad_0000[0xa8];
    int m_00a8;               // non-zero while the model takes part in collision
    char pad_00ac[0x1a0];
    Matrix4 worldMatrix;      // 0x24c: world matrix of the node (scene_node.cpp Node::worldMatrix)
    LoadedModel *Find(const char *name, int flags);  // Node::Find 0x1019ece0
};

extern "C" LoadedModel *DAT_10faaa88[0x100];
extern "C" int DAT_10ff19e8[0x100];   // per model: its collision data

// Collision test of a model (world matrix) against another model: 0x10193140 (stdcall, recursive over nodes).
extern "C" int __stdcall FUN_10193140(Matrix4 *matrix, LoadedModel *other, int otherData, Vec4 *probe, int count,
                                      int *hitCount, int *hitIds, Vec4 *hits, int *hitOther, int mode, int a, int b,
                                      int c, int flags);

#include "node.h"

extern "C" int DAT_11b002c0;    // collision filter of the current test (the model's m_0070)
extern "C" char DAT_11b002c8;   // 1 while a model-against-world test runs

struct AnimModel
{
    char pad_0000[0x70];
    int m_0070;                  // 0x70: collision filter
    char pad_0074[0x10];
    unsigned char speedDivisor;  // 0x84: slows every motion of this model down (0 = no division)
    char pad_0085[0x1c3];
    float holdWeight;            // 0x248: blend weight while holding (at most 0.9)
    char pad_024c[0x50];
    float scale;
    char pad_02a0[4];
    char holdState;              // 0x2a4: 0 = free, 1 = holding, 2 = released
    char pad_02a5[0x1ff7];
    int m_229c;                  // 0x229c: blend frames, 8 when unset
    char pad_22a0[0x4c];
    LoadedModel *root;           // 0x22ec: this model's own entry in the model table
    char pad_22f0[0x18];
    Node *collider;     // 0x2308: the map collision this model moves against
    char pad_230c[0xc];
    int m_2318;
    int m_231c;
    char pad_2320[8];
    Matrix4 world;               // 0x2328: world matrix (translation = position)
    char pad_2368[4];
    float motionSpeed;           // 0x236c: current motion playback speed (at most 60)
    char pad_2370[0x18];
    int m_2388;                  // 0x2388: 1 once a speed change is pending
    int m_238c;
    int m_2390;                  // 0x2390: limit for map collision tests
    char pad_2394[4];
    Vec4 holdOffset;             // 0x2398: position relative to the held model

    Vec4 push;                   // 0x23a8: movement impulse applied by the next update
    Vec4 velocity;               // 0x23b8: current movement
    char pad_23c8[0x10];
    float m_23d8;
    float m_23dc;
    int m_23e0;
    char pad_23e4[0x9c];
    float heading;
    char pad_2484[0x1510];
    int M_101627d0(float x, float y, int mode);
    int M_101629b0(float y, float x);
    int FaceAway(const Vec4 &point);
    int FaceAwayFrom(AnimModel *other);
    int AddMotionSpeed(float speed);
    int PushMotionSpeed(float speed);
    int MoveTowards(float x, float y, int mode);
    int SnapHeadingToSide();
    int ScreenPosition(int *x, int *y, const Matrix4 &viewProjection);
    int Push(float x, float y, float z);
    int IsClear(const Vec4 *offset);
    int IsClearAny(const Vec4 *offset);
    int IsClearFlagged(const Vec4 *offset);
    int IsClearOfMap(const Vec4 *offset);
    int IsClearOfMapLimited(const Vec4 *offset);
    int Hold(int on, float amount, AnimModel *other);
    int HoldAtSpine(int on, float amount);
    int SnapHeight(float step, float offset);
    int SnapPosition(float step, float offset);
};

// Sets the model's movement offset (x, y scaled by the model scale) and the movement mode.
int AnimModel::M_101627d0(float x, float y, int mode)
{
    m_23d8 = x * scale;
    m_23e0 = mode;
    m_23dc = y * scale;
    return 0;
}

// Turns the model to face the direction (x, y): heading = atan2(y, x) + pi (3.1415).
int AnimModel::M_101629b0(float y, float x)
{
    heading = (float)atan2(y, x) + 3.1415f;
    return 1;
}

// Turns the model away from `point`: heading = atan2(dx, dy) of the vector from `point` to the model.
int AnimModel::FaceAway(const Vec4 &point)
{
    Vec4 delta;
    Vec4 unused;  // constructed but never used (the original has it too)
    delta.Copy(world.GetTranslation());
    delta.Copy(delta - point);
    heading = (float)atan2(delta.x, delta.y);
    return 0;
}

// Turns the model away from model `other` (heading of the vector from `other` to this model).
int AnimModel::FaceAwayFrom(AnimModel *other)
{
    Vec4 position;
    Vec4 otherPosition;
    position.Copy(world.GetTranslation());
    otherPosition.Copy(other->world.GetTranslation());
    position.Copy(position - otherPosition);
    heading = (float)atan2(position.x, position.y);
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
    x = x - world.m[12];
    y = y - world.m[13];
    m_23d8 = x;
    m_23dc = y;
    m_23e0 = mode;
    if (x > 40.0f)
        m_23d8 = 40.0f;
    if (m_23d8 < -40.0f)
        m_23d8 = -40.0f;
    if (y > 40.0f)
        m_23dc = 40.0f;
    if (m_23dc < -40.0f)
        m_23dc = -40.0f;
    return 0;
}

// Wraps the heading into 0..2pi (2pi = 6.283 here) and snaps it to the nearer side view: pi/2 when it points into the
// first half turn, 3pi/2 otherwise (pi = 3.1415). Returns 0.
int AnimModel::SnapHeadingToSide()
{
    while (heading > 6.283f)
        heading -= 6.283f;
    while (heading < 0.0f)
        heading += 6.283f;
    if (!(heading < 0.0f) && heading < 3.1415f)
    {
        heading = 3.1415f / 2;
        return 0;
    }
    heading = 3.1415f * 1.5f;
    return 0;
}

// Projects a point 50 units above the model's position to 640x480 screen pixels (y grows downwards) and stores
// them in *x and *y. Used to place balloons and markers over characters. Returns 0.
int AnimModel::ScreenPosition(int *x, int *y, const Matrix4 &viewProjection)
{
    Vec4 position;
    position.Copy(world.GetTranslation());
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
        push.x = x * scale * 0.7f;
        push.y = y * scale * 0.7f;
        push.z = z * 0.7f;
        if (velocity.LengthSquared3() < 1.0f)
        {
            push.x = x * 1.4f;
            push.y = y * 1.4f;
            push.z = z * 1.4f;
        }
    }
    else
    {
        push.x = x * 0.55f;
        push.y = y * 0.55f;
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
        if (DAT_10faaa88[i] != 0 && root != DAT_10faaa88[i] && DAT_10faaa88[i]->m_00a8 != 0)
        {
            if (FUN_10193140(&world, DAT_10faaa88[i], DAT_10ff19e8[i], probe, 1, &hitCount, hitIds, hits, &hitOther,
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
        if (DAT_10faaa88[i] != 0 && root != DAT_10faaa88[i] && DAT_10faaa88[i]->m_00a8 != 0 &&
            FUN_10193140(&world, DAT_10faaa88[i], DAT_10ff19e8[i], probe, 1, &hitCount, hitIds, hits, &hitOther, 0,
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
        if (DAT_10faaa88[i] != 0 && root != DAT_10faaa88[i] && DAT_10faaa88[i]->m_00a8 != 0 &&
            FUN_10193140(&world, DAT_10faaa88[i], DAT_10ff19e8[i], probe, 1, &hitCount, hitIds, hits, &hitOther, -1,
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
    DAT_11b002c0 = m_0070;
    DAT_11b002c8 = 1;
    int hitCount = 0;
    int hit = collider->M_1019f340(0x4000000, &world, m_231c, m_2318, &out5, probe, 1, &hitCount, &hitPoint, &hitNormal, 0,
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
    DAT_11b002c0 = m_0070;
    DAT_11b002c8 = 1;
    int hitCount = 0;
    int hit = collider->M_1019f340(-1, &world, m_231c, m_2318, &out5, probe, 1, &hitCount, &hitPoint, &hitNormal, m_2390,
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
                holdOffset.Copy(other->holdOffset + other->world.GetTranslation());
                holdOffset.Copy(holdOffset - world.GetTranslation());
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
                LoadedModel *spine = root->Find("Bip01 Spine", 0);
                if (spine != 0)
                {
                    holdOffset.Copy(spine->worldMatrix.GetTranslation());
                    holdOffset.Copy(holdOffset - world.GetTranslation());
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
    Vec4 position = world.GetTranslation();
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
    world.SetTranslation(position);
    return 0;
}

// 0x10161cb0. Like SnapHeight for x and y: both are moved to the nearest multiple of `step` (in tenths of a unit)
// plus `offset`; the height stays. Returns 0.
int AnimModel::SnapPosition(float step, float offset)
{
    int candidates[4];  // the multiples just below, above and two below; the fourth entry is unused
    Vec4 position = world.GetTranslation();
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
    world.SetTranslation(position);
    return 0;
}
