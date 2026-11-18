// Visibility of map regions (the table at DAT_104c03c0): a region counts as visible when a corner of its box, at
// half its height, is inside the camera's view volume.

#include <math.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Copy(const Vec4 &source);
};

struct Matrix4
{
    float m[16];
};

// A vertex of a region's water surface, 0x70 bytes (only its position is used here).
struct RegionVertex
{
    float x;
    float y;
    float z;
    char pad_000c[0x64];
};

// A control point of a region, 0x7c bytes.
struct RegionPoint
{
    char pad_0000[0x60];
    float m_0060;             // 0x60: its hundreds (odd or even) choose the wave's phase together with m_0064
    float m_0064;
    char pad_0068[0x14];
};

// Map region, 0x44 bytes.
struct MapRegion
{
    RegionVertex *vertices;   // 0x00
    int pad_0004;
    RegionPoint *points;      // 0x08
    char pad_000c[8];
    Vec4 min;                 // 0x14: box corner with the smallest coordinates
    Vec4 max;                 // 0x24: box corner with the largest coordinates
    int pad_0034;
    int hidden;               // 0x38: non-zero = never visible
    char pad_003c[8];
};

extern "C" MapRegion *DAT_104c03c0;
extern "C" unsigned int DAT_104bf8a4;   // frame counter
extern "C" Matrix4 DAT_108392d8;  // the current view-projection matrix
extern "C" int FUN_101ae4d0(const Vec4 *point, const Matrix4 *matrix, float margin);  // View_ContainsPoint

// 0x10021070 (Region_IsVisible). 1 when one of the four corners of region `index` (at half its height) lies in the
// view volume widened by 50, else 0 (also for hidden regions).
extern "C" int FUN_10021070(int index)
{
    Vec4 corner;
    if (DAT_104c03c0[index].hidden != 0)
        return 0;
    float middle = DAT_104c03c0[index].max.z + DAT_104c03c0[index].min.z;
    middle *= 0.5f;  // half the height
    corner.Copy(DAT_104c03c0[index].min);
    corner.z = middle;
    if (FUN_101ae4d0(&corner, &DAT_108392d8, 50.0f) != 0)
        return 1;
    corner.Copy(DAT_104c03c0[index].max);
    corner.z = middle;
    if (FUN_101ae4d0(&corner, &DAT_108392d8, 50.0f) != 0)
        return 1;
    corner.x = DAT_104c03c0[index].min.x;
    corner.y = DAT_104c03c0[index].max.y;
    corner.z = middle;
    if (FUN_101ae4d0(&corner, &DAT_108392d8, 50.0f) != 0)
        return 1;
    corner.x = DAT_104c03c0[index].max.x;
    corner.y = DAT_104c03c0[index].min.y;
    corner.z = middle;
    return FUN_101ae4d0(&corner, &DAT_108392d8, 50.0f) != 0;
}

// sin on floats (an inline helper).
inline float Sin(float angle)
{
    return (float)sin(angle);
}

// 0x100211a0 (Region_WaveVertex). Moves vertex `point` of region `index` by sine waves of the frame counter: 15 units
// in x and y (periods of 240 and 300 frames) and 10 in z (356 frames), half a period later when exactly one of the
// point's two values has an odd hundreds digit. The offsets add up from frame to frame. Returns 0.
extern "C" int FUN_100211a0(int index, int point)
{
    int flip = 0;
    if ((int)DAT_104c03c0[index].points[point].m_0060 / 100 & 1)
        flip = 1;
    if ((int)DAT_104c03c0[index].points[point].m_0064 / 100 & 1)
        flip++;
    float phase = (float)(flip & 1) * 3.1415f;
    float angleX = (float)(DAT_104bf8a4 % 240) * 0.026179167f + phase;
    float angleY = (float)(DAT_104bf8a4 % 300) * 0.020943332f + phase;
    float angleZ = (float)(DAT_104bf8a4 % 356) * 0.017648876f + phase;
    DAT_104c03c0[index].vertices[point].x += Sin(angleX) * 15.0f;
    DAT_104c03c0[index].vertices[point].y += Sin(angleY) * 15.0f;
    DAT_104c03c0[index].vertices[point].z += Sin(angleZ) * 10.0f;
    return 0;
}
