// A batch of screen-space vertices: 2D quads are laid out for a 640x480 screen and scaled to the real back buffer
// size when they are added.

// A screen vertex (0x20 bytes): position, then colour and texture coordinates.
struct ScreenVertex
{
    float x;
    float y;
    float rest[6];
};

struct ScreenBatch
{
    ScreenVertex *vertices;      // room for the batch
    int pad_0004;
    int count;                   // 0x08: vertices added so far
    int Add(const ScreenVertex *source, int sourceCount);
};

#include "render_vars.h"

// 0x10172ac0 (ScreenBatch::Add). Appends `sourceCount` vertices given for a 640x480 screen, scaling their positions
// to the back buffer. Returns 1.
int ScreenBatch::Add(const ScreenVertex *source, int sourceCount)
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
