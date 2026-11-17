// Character model parameters (offsets inside the model object of a Character, see world.h).

#include <math.h>

struct AnimModel
{
    char pad_0000[0x29c];
    float scale;
    char pad_02a0[0x2138];
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
    void M_10167400();
    int M_10167390(int id, int value, int frames);
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
