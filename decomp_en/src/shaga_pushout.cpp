// Pushing a shaga piece out of the world collision (EN build). Ported from the matched JP source
// decomp/src/shaga_pushout.cpp.
#include "collide_regions.h"
#include "node.h"
#include "shaga.h"

// Distance a piece is moved along the hit normal each step.
const float SHAGA_PUSH_STEP = 5.0f;

// Steps after which the push stops even when the piece still touches the world.
const int SHAGA_PUSH_MAX_STEPS = 10;

// Extra radius the collision sphere has over the piece's own radius.
const float SHAGA_PUSH_MARGIN = 1.0f;

// Shaga_PushOut: moves shaga piece `index` out of the world collision: its centre (the placement raised by its radius)
// is tested as a sphere of radius + SHAGA_PUSH_MARGIN and, while it hits something, moved SHAGA_PUSH_STEP units along the
// hit normal (at most SHAGA_PUSH_MAX_STEPS times). Returns 0. Called after a piece is placed or has moved.
int Shaga_PushOut(int index)
{
    Matrix4 identity;
    Vec4 position;
    Vec4 normal;
    float depth;
    identity.SetIdentity();
    position.Copy(g_shagaNode[index]->matrix.GetTranslation());
    position.z += g_shagaRadius[index];
    int steps = 0;
    do
    {
        if (g_worldRoot == 0)
            break;
        if (g_worldRoot->Query(&identity, &position, g_shagaRadius[index] + SHAGA_PUSH_MARGIN, &normal, &depth, 0, 0) == 0)
            break;
        normal.Normalized3();
        float step = SHAGA_PUSH_STEP;
        normal.Copy(normal * step);
        position.Copy(position + normal);
        steps++;
    } while (steps < SHAGA_PUSH_MAX_STEPS);
    position.z -= g_shagaRadius[index];
    g_shagaNode[index]->matrix.SetTranslation(position);
    return 0;
}
