// Falling drops: 1024 points that drift and fall, drawn as screen-facing quads while they are close to the camera,
// and fade out over 30 frames once they pass a height (DAT_118e6cc0, the level they land on).

struct Matrix4;

#include "vec4.h"
#include "render_vars.h"

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);   // 0x101ae060 (vec4.cpp)

// The 2D triangle batch at 0x1021cffc (texture_mgr.cpp TriangleBatch).
struct TriangleBatch
{
    int Reset();                                   // 0x10172aa0
    // 0x10172cb0: adds a quad from (left, top) to (right, bottom) at depth z with UVs (u0, v0)-(u1, v1).
    void DrawQuad(float left, float top, float right, float bottom, float u0, float v0, float u1, float v1, float z,
                  float alpha, float r, float g, float b, int a0, int a1, int a2, int a3);
};

#define DROP_COUNT 0x400

extern "C" TriangleBatch DAT_1021cffc;
extern "C" unsigned char DAT_10254c40[DROP_COUNT]; // 0 = unused, 1 = falling, 2..30 = fading out
#include "char_vars.h"

// 0x1000c390 (Drops_Update). Moves every drop (x and y by 0.02, down by 3.5), draws it when its depth is between 0.9
// and 1.0 (larger when nearer, fading with its state) and advances its state: a falling drop starts fading once it is
// below the landing height (with an offset of -10 for even drops and 800 for odd ones); a fading drop counts up
// while below that height and down while above it, and is freed after 30. Returns 0.
extern "C" int FUN_1000c390(void)
{
    DAT_1021cffc.Reset();
    for (int i = 0; i < DROP_COUNT; i++)
    {
        if (DAT_10254c40[i] != 0)
        {
            DAT_104c0438[i].x += 0.02f;
            DAT_104c0438[i].y += 0.02f;
            DAT_104c0438[i].z -= 3.5f;
            Vec4 screen;
            screen.Copy(Project(DAT_104c0438[i], DAT_108392d8));
            screen.x = screen.x * 640.0f / 2.0f + 320.0f;
            screen.y = 240.0f - screen.y * 480.0f / 2.0f;
            if (!(screen.z > 1.0 || screen.z < 0.9))   // only drops close to the camera
            {
                float size = (1.0f - screen.z) * 344.0f;
                float alpha = 1.0f - (float)DAT_10254c40[i] / 30.0f;
                DAT_1021cffc.DrawQuad(screen.x - size, screen.y - size, size + screen.x, size + screen.y, 0, 0, 1.0f,
                                      1.0f, screen.z, alpha, 1.0f, 1.0f, 1.0f, 0, 0, 0, 0);
            }
            int offsets[2];
            offsets[0] = -10;
            offsets[1] = 800;
            if (DAT_10254c40[i] > 1)
            {
                if ((float)offsets[i % 2] + DAT_104c0438[i].z < DAT_118e6cc0)
                    DAT_10254c40[i]++;
                else
                    DAT_10254c40[i]--;
                if (DAT_10254c40[i] > 30)
                    DAT_10254c40[i] = 0;
            }
            else if ((float)offsets[i % 2] + DAT_104c0438[i].z < DAT_118e6cc0)
                DAT_10254c40[i] = 2;
        }
    }
    return 0;
}
