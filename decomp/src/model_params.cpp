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
};

struct Matrix4
{
    float m[16];
    Vec4 GetTranslation();
};

struct AnimModel
{
    char pad_0000[0x84];
    unsigned char speedDivisor;  // 0x84: slows every motion of this model down (0 = no division)
    char pad_0085[0x217];
    float scale;
    char pad_02a0[0x1ffc];
    int m_229c;                  // 0x229c: blend frames, 8 when unset
    char pad_22a0[0x88];
    Matrix4 world;               // 0x2328: world matrix (translation = position)
    char pad_2368[4];
    float motionSpeed;           // 0x236c: current motion playback speed (at most 60)
    char pad_2370[0x18];
    int m_2388;                  // 0x2388: 1 once a speed change is pending
    int m_238c;
    char pad_2390[0x48];
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
