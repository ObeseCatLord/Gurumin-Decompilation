// Camera orbit: turns the camera position (DAT_11af3e28) around its target (DAT_118e6c78) on the ground plane.

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
