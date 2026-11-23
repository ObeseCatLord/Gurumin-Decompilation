// Quads for the triangle batch (tri_batch.h): a rectangle laid out for a 640x480 screen, optionally turned about its
// centre, is written as two triangles scaled to the real back buffer size.

#include "screen_vertex.h"
#include "tri_batch.h"
#include "render_vars.h"

#define SCREEN_LAYOUT_WIDTH 640.0f
#define SCREEN_LAYOUT_HEIGHT 480.0f
#define COLOR_CHANNEL_MAX 255
#define VERTICES_PER_QUAD 6

// Turns a 0..1 colour channel into 0..255, clamped.
static __forceinline int ColorByte(float channel)
{
    int value = (int)(channel * 255.0f);
    if (value > COLOR_CHANNEL_MAX)
        value = COLOR_CHANNEL_MAX;
    else if (value < 0)
        value = 0;
    return value;
}

// Fills vertex `index` of the batch (position already in back buffer pixels, rhw 1).
#define PUT_QUAD_VERTEX(px, py, tu, tv)     do     {         int index = first++;         vertices[index].x = (px);         vertices[index].y = (py);         vertices[index].u = (tu);         vertices[index].v = (tv);         vertices[index].z = z;         vertices[index].rhw = 1.0f;         vertices[index].diffuse = diffuse;         vertices[index].specular = specular;     } while (0)

// 0x10172cb0 (TriangleBatch::AppendQuad). Appends a quad (two triangles, six vertices) from (left, top) to
// (right, bottom) for a 640x480 screen, scaled to the back buffer. The corners are first turned by `rotation`
// about the quad's centre when it is not 0. Colours: diffuse = alpha, r, g, b; specular = r2, g2, b2.
// Returns 0.
int TriangleBatch::AppendQuad(float left, float top, float right, float bottom, float u0, float v0, float u1,
                              float v1, float z, float alpha, float r, float g, float b, float r2, float g2,
                              float b2, float rotation)
{
    int alpha8 = ColorByte(alpha);
    int red = ColorByte(r);
    int green = ColorByte(g);
    int blue = ColorByte(b);
    unsigned int diffuse = ((((alpha8 << 8) | red) << 8) | green) << 8 | blue;
    int red2 = ColorByte(r2);
    int green2 = ColorByte(g2);
    int blue2 = ColorByte(b2);
    unsigned int specular = ((red2 << 8) | green2) << 8 | blue2;
    int first = count;
    float scaleX = (float)DAT_10202b34 / SCREEN_LAYOUT_WIDTH;
    float scaleY = (float)DAT_10202b38 / SCREEN_LAYOUT_HEIGHT;
    if (rotation != 0.0f)
    {
        Vec4 topLeft;
        Vec4 bottomRight;
        Vec4 topRight;
        Vec4 bottomLeft;
        Vec4 center;
        center.x = (left + right) * 0.5f;
        center.y = (top + bottom) * 0.5f;
        topLeft.x = left - center.x;
        topLeft.y = top - center.y;
        bottomRight.x = right - center.x;
        bottomRight.y = bottom - center.y;
        topRight.x = right - center.x;
        topRight.y = top - center.y;
        bottomLeft.x = left - center.x;
        bottomLeft.y = bottom - center.y;
        topLeft.RotateZ(rotation);
        bottomRight.RotateZ(rotation);
        topRight.RotateZ(rotation);
        bottomLeft.RotateZ(rotation);
        topLeft.x += center.x;
        topLeft.y += center.y;
        bottomRight.x += center.x;
        bottomRight.y += center.y;
        topRight.x += center.x;
        topRight.y += center.y;
        bottomLeft.x += center.x;
        bottomLeft.y += center.y;
        PUT_QUAD_VERTEX(topLeft.x * scaleX, topLeft.y * scaleY, u0, v0);
        PUT_QUAD_VERTEX(topRight.x * scaleX, topRight.y * scaleY, u1, v0);
        PUT_QUAD_VERTEX(bottomLeft.x * scaleX, bottomLeft.y * scaleY, u0, v1);
        PUT_QUAD_VERTEX(topRight.x * scaleX, topRight.y * scaleY, u1, v0);
        PUT_QUAD_VERTEX(bottomRight.x * scaleX, bottomRight.y * scaleY, u1, v1);
        PUT_QUAD_VERTEX(bottomLeft.x * scaleX, bottomLeft.y * scaleY, u0, v1);
    }
    else
    {
        float x0 = scaleX * left;
        float y0 = scaleY * top;
        float x1 = scaleX * right;
        float y1 = scaleY * bottom;
        PUT_QUAD_VERTEX(x0, y0, u0, v0);
        PUT_QUAD_VERTEX(x1, y0, u1, v0);
        PUT_QUAD_VERTEX(x0, y1, u0, v1);
        PUT_QUAD_VERTEX(x1, y0, u1, v0);
        PUT_QUAD_VERTEX(x1, y1, u1, v1);
        PUT_QUAD_VERTEX(x0, y1, u0, v1);
    }
    count += VERTICES_PER_QUAD;
    return 0;
}
