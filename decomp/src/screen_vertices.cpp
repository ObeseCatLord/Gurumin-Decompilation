// Adding screen-space vertices to a triangle batch (tri_batch.h): 2D quads are laid out for a 640x480 screen and
// scaled to the real back buffer size when they are added.

#include "screen_vertex.h"
#include "tri_batch.h"
#include "render_vars.h"

// 0x10172ac0 (TriangleBatch::AppendVertices). Appends `sourceCount` vertices given for a 640x480 screen, scaling their positions
// to the back buffer (DAT_10202b34 x DAT_10202b38). Called by AddVertices. Returns 1.
int TriangleBatch::AppendVertices(const ScreenVertex *source, int sourceCount)
{
    float scaleX = (float)DAT_10202b34 / 640.0f;
    float scaleY = (float)DAT_10202b38 / 480.0f;
    for (int i = 0; i < sourceCount; i++)
    {
        vertices[count + i] = source[i];
        vertices[count + i].x = scaleX * vertices[count + i].x;
        vertices[count + i].y = scaleY * vertices[count + i].y;
    }
    count += sourceCount;
    return 1;
}
