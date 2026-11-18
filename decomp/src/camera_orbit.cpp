// Camera helpers: orbiting the camera position (DAT_11af3e28) around its target (DAT_118e6c78) on the ground plane
// and turning input directions into camera space.

#include <math.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Fill(const float &value);
    void Copy(const Vec4 &source);
    void RotateZ(float angle);
    float Length2D();
    Vec4 operator+(const Vec4 &other) const;
    Vec4 operator-(const Vec4 &other) const;
};

extern "C" Vec4 DAT_11af3e28;  // camera position
extern "C" Vec4 DAT_118e6c78;  // camera target
extern "C" Vec4 DAT_11af3f58;  // the camera-space push of the player (x, y; z is 0)

// Places the camera at heading `angle` around the target, keeping its ground distance and its height above the
// target. Returns 0.
extern "C" int FUN_1016bdf0(float angle)
{
    Vec4 offset;
    float zero = 0;
    offset.Fill(zero);
    offset.z = DAT_11af3e28.z - DAT_118e6c78.z;
    offset.y = (DAT_11af3e28 - DAT_118e6c78).Length2D();
    offset.RotateZ(angle);
    DAT_11af3e28.Copy(DAT_118e6c78 + offset);
    return 0;
}

// Turns the direction (x, y) by the camera heading (the angle from the camera position to its target) and stores it,
// doubled, in DAT_11af3f58 with z = 0. Returns 0.
extern "C" int FUN_1016b260(float x, float y)
{
    float angle = (float)atan2(DAT_118e6c78.x - DAT_11af3e28.x, DAT_118e6c78.y - DAT_11af3e28.y);
    float turnedX = (float)cos(angle) * x - (float)sin(angle) * y;
    float turnedY = (float)sin(angle) * x + (float)cos(angle) * y;
    DAT_11af3f58.z = 0;
    DAT_11af3f58.x = turnedX + turnedX;
    DAT_11af3f58.y = turnedY + turnedY;
    return 0;
}

// Turns the input direction (x, y) into world space for the current camera heading (the camera looks from its
// position towards its target) and stores the result in *outX and *outY. Returns 0.
extern "C" int FUN_1016b340(float *outX, float *outY, float x, float y)
{
    float angle = -(float)atan2(DAT_118e6c78.x - DAT_11af3e28.x, DAT_118e6c78.y - DAT_11af3e28.y);
    *outX = (float)cos(angle) * x - (float)sin(angle) * y;
    *outY = (float)cos(angle) * y + (float)sin(angle) * x;
    return 0;
}
