// Visibility of map regions (the table at DAT_104c03c0): a region counts as visible when a corner of its box, at
// half its height, is inside the camera's view volume.

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

// Map region, 0x44 bytes.
struct MapRegion
{
    char pad_0000[0x14];
    Vec4 min;                 // 0x14: box corner with the smallest coordinates
    Vec4 max;                 // 0x24: box corner with the largest coordinates
    int pad_0034;
    int hidden;               // 0x38: non-zero = never visible
    char pad_003c[8];
};

extern "C" MapRegion *DAT_104c03c0;
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
