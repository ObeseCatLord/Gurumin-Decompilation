// Character model parameters (offsets inside the model object of a Character, see world.h).

#include <math.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Copy(const Vec4 &source);
    Vec4 operator-(const Vec4 &other) const;
    float LengthSquared3();  // 0x101ac6a0
    void Fill(const float &value);
};

struct Matrix4
{
    float m[16];
    Vec4 GetTranslation();
};

// 0x101ae060: transforms `point` by `matrix` and divides by w (projection to -1..1 screen space).
Vec4 Project(const Vec4 &point, const Matrix4 &matrix);

// Model table (anim_model.cpp: DAT_10faaa88): the loaded models, each with its collision state in m_00a8.
struct LoadedModel
{
    char pad_0000[0xa8];
    int m_00a8;               // non-zero while the model takes part in collision
};

extern "C" LoadedModel *DAT_10faaa88[0x100];
extern "C" int DAT_10ff19e8[0x100];   // per model: its collision data

// Collision test of a model (world matrix) against another model: 0x10193140 (stdcall, recursive over nodes).
extern "C" int __stdcall FUN_10193140(Matrix4 *matrix, LoadedModel *other, int otherData, Vec4 *probe, int count,
                                      int *hitCount, int *hitIds, Vec4 *hits, int *hitOther, int mode, int a, int b,
                                      int c, int flags);

struct AnimModel
{
    char pad_0000[0x84];
    unsigned char speedDivisor;  // 0x84: slows every motion of this model down (0 = no division)
    char pad_0085[0x217];
    float scale;
    char pad_02a0[0x1ffc];
    int m_229c;                  // 0x229c: blend frames, 8 when unset
    char pad_22a0[0x4c];
    LoadedModel *root;           // 0x22ec: this model's own entry in the model table
    char pad_22f0[0x38];
    Matrix4 world;               // 0x2328: world matrix (translation = position)
    char pad_2368[4];
    float motionSpeed;           // 0x236c: current motion playback speed (at most 60)
    char pad_2370[0x18];
    int m_2388;                  // 0x2388: 1 once a speed change is pending
    int m_238c;
    char pad_2390[0x18];
    Vec4 push;                   // 0x23a8: movement impulse applied by the next update
    Vec4 velocity;               // 0x23b8: current movement
    char pad_23c8[0x10];
    float m_23d8;
    float m_23dc;
    int m_23e0;
    char pad_23e4[0x9c];
    float heading;
    char pad_2484[0x1510];
    float m_3994;
    int m_3998;
    float m_399c;
    float m_39a0;
    int m_39a4;
    int M_101627d0(float x, float y, int mode);
    int M_101629b0(float y, float x);
    int FaceAway(const Vec4 &point);
    int FaceAwayFrom(AnimModel *other);
    void M_10167400();
    int M_10167390(int id, int value, int frames);
    int AddMotionSpeed(float speed);
    int PushMotionSpeed(float speed);
    int MoveTowards(float x, float y, int mode);
    int SnapHeadingToSide();
    int ScreenPosition(int *x, int *y, const Matrix4 &viewProjection);
    int Push(float x, float y, float z);
    int IsClear(const Vec4 *offset);
    int IsClearAny(const Vec4 *offset);
    int IsClearFlagged(const Vec4 *offset);
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

// Advances a timed value: adds the step to the current value each frame and snaps to the target when the
// counter reaches zero.
void AnimModel::M_10167400()
{
    if (m_3998 != 0)
    {
        m_39a0 += m_3994;
        if (--m_3998 == 0)
            m_39a0 = m_399c;
    }
}

// Starts a timed change of the value at m_39a0 towards `value` over `frames` frames (0 = set at once),
// remembering `id` in m_39a4. M_10167400 advances it each frame.
int AnimModel::M_10167390(int id, int value, int frames)
{
    m_39a4 = id;
    m_399c = value;
    if (frames == 0)
    {
        m_39a0 = value;
        m_399c = value;
        m_3994 = 0;
        m_3998 = 0;
        return 0;
    }
    m_3998 = frames;
    m_3994 = (value - m_39a0) / frames;
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
