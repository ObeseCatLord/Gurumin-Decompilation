// Keeping the "shaga" effect models (shaga_spawn.cpp) out of the world collision. C++ rather than extern "C": the
// function keeps an exception frame for its Matrix4 local.

#include "vec4.h"

#include "matrix4.h"

#include "node.h"
#include "map_vars.h"

#include "char_vars.h"


// 0x1001af10 (Shaga_PushOut). Moves shaga model `index` out of the world collision: its centre (the placement raised
// by its radius) is tested as a sphere of radius + 1, and while it hits something it is moved 5 units along the hit
// normal (at most 10 times). Returns 0.
int Shaga_PushOut(int index)
{
    Matrix4 identity;
    int steps = 0;
    Vec4 position;
    Vec4 normal;
    float depth;
    identity.SetIdentity();
    position.Copy(DAT_10446e00[index]->matrix.GetTranslation());
    position.z += DAT_1030cd98[index];
    do
    {
        if (DAT_10255f08 == 0)
            break;
        if (DAT_10255f08->Query(&identity, &position, DAT_1030cd98[index] + 1.0f, &normal, &depth, 0, 0) == 0)
            break;
        normal.Normalized3();
        normal.Copy(normal * 5.0f);
        position.Copy(position + normal);
        steps++;
    } while (steps < 10);
    position.z -= DAT_1030cd98[index];
    DAT_10446e00[index]->matrix.SetTranslation(position);
    return 0;
}
