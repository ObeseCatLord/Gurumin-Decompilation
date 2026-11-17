// Squash-and-stretch pulse for a character's pose matrix.

#include <stdlib.h>
#include "world.h"

struct Matrix4
{
    float m[16];
    ~Matrix4();
};

// Pose object of a character (pointer table DAT_1021bdf8, 0x24 bytes per character); matrix at 0x1cc.
struct Pose
{
    char pad_0000[0x1cc];
    Matrix4 matrix;
};

struct PoseRef
{
    Pose *pose;
    int pad_0004[8];
};

extern "C" PoseRef DAT_1021bdf8[0x80];
extern "C" Matrix4 FUN_101b13c0(float x, float y, float z);

// Advances the pulse step (a 0..7 triangle wave in m_0004[1]) and sets the pose to a scale matrix that
// widens x/y while flattening z (t = |3 - step| / 3). The step advances by two per call. Returns 0.
extern "C" int FUN_1001a390(int index)
{
    if (DAT_1020b430[index].m_0004[1] > 3)
        DAT_1020b430[index].m_0004[1] = 7 - DAT_1020b430[index].m_0004[1];
    float t = (float)abs(3 - ++DAT_1020b430[index].m_0004[1]) / 3.0f;
    if (DAT_1021bdf8[index].pose != 0)
    {
        float s = 2.0f - t;
        DAT_1021bdf8[index].pose->matrix = FUN_101b13c0(s, s, t * 0.7f + 0.3f);
    }
    DAT_1020b430[index].m_0004[1]++;
    return 0;
}
