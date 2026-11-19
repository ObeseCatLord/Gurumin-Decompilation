// Switching the camera to one of the map's camera paths (map camera data, see camera_reset.cpp): the path is looked
// up by its id and the camera starts at the path frame closest to the camera target.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                          // 0x101ac990: zero
    void Copy(const Vec4 &source);                   // 0x101acc20
    float DistanceZWeighted(const Vec4 &other);      // 0x101ac5f0: distance with z counted twice
};

struct Matrix4
{
    float m[16];
    Vec4 GetTranslation();                           // 0x101ade80
};

// A key track (vec4.cpp KeyTrack), 0x14 bytes: its frames are sorted, so the last one is the length.
struct KeyTrack
{
    int capacity;
    int *frames;
    Vec4 *values;
    char *flags;
    int count;
    // 0x101afe00: the value at `frame` (with `rotation` set, a quaternion interpolated as a rotation).
    int Sample(int frame, float *x, float *y, float *z, float *w, int rotation);
};

// The model the camera follows (AnimModel), only its world matrix.
struct CameraTarget
{
    char pad_0000[0x2328];
    Matrix4 world;                                   // 0x2328
};

extern "C" int DAT_1101d89c;                         // number of camera paths
extern "C" int DAT_10f8d938[];                       // id of each path
extern "C" KeyTrack DAT_118e69f8[];                  // the position track of each path
extern "C" int DAT_1101d8a4;                         // the current path
extern "C" int DAT_1101d890;                         // 1 while a path camera runs
extern "C" int DAT_1101d8a0;
extern "C" float DAT_1101c800;                       // frame of the path
extern "C" char DAT_10faa678;
extern "C" CameraTarget *DAT_1101d884;               // the model the camera follows

extern "C" int FUN_1015c9e0(int mode);               // Screen_StartTransition

// 0x1016b050 (Camera_SelectPath). Id -1 stops the path camera. Otherwise the path with that id becomes current (if
// the camera already runs on it nothing changes; switching paths starts transition 0xf), and the camera starts at
// the frame of the path closest to the camera target (distance with z counted twice). Returns 0.
extern "C" int FUN_1016b050(int id)
{
    if (id == -1)
    {
        DAT_1101d890 = 0;
        return 0;
    }
    int previous = DAT_1101d8a4;
    for (int i = 0; i < DAT_1101d89c; i++)
    {
        if (DAT_10f8d938[i] == id)
        {
            DAT_1101d8a4 = i;
            break;
        }
    }
    if (DAT_1101d890 != 0)
    {
        if (DAT_1101d8a4 == previous)
            return 0;
        FUN_1015c9e0(0xf);
    }
    int path = DAT_1101d8a4;
    float nearest = 0.0f;
    Vec4 point;
    Vec4 target;
    target.Copy(DAT_1101d884->world.GetTranslation());
    float length = (float)DAT_118e69f8[path].frames[DAT_118e69f8[path].count - 1];
    float bestFrame;
    int bestPath;
    int frame = 0;
    for (float f = 0.0f; f < length; f = (float)++frame)
    {
        DAT_118e69f8[path].Sample(frame, &point.x, &point.y, &point.z, &point.w, 0);
        float distance = point.DistanceZWeighted(target);
        if (nearest >= distance || frame == 0)
        {
            nearest = distance;
            bestFrame = f;
            bestPath = path;
        }
    }
    DAT_1101d8a0 = 1;
    DAT_1101d8a4 = bestPath;
    DAT_1101c800 = bestFrame;
    DAT_1101d890 = 1;
    DAT_10faa678 = 1;
    return 0;
}
