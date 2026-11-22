// A node's animation data (the extra block of a scene node): three key tracks for translation, rotation and scale,
// each of which can be replaced by another track. Matrix4 has the engine's constructor and empty destructor, so the
// function holding matrices by value gets an exception frame.

#include "vec4.h"

struct Matrix4
{
    float m[16];
    Matrix4();                                   // 0x101ae770
    ~Matrix4();                                  // 0x1013eda0 (empty)
    Matrix4 operator*(const Matrix4 &other);     // 0x101b0670: matrix product
};

Matrix4 ScaleMatrix(float x, float y, float z);         // 0x101b13c0
Matrix4 RotationMatrix(const Vec4 &quaternion);         // 0x101b08c0
Matrix4 TranslationMatrix(float x, float y, float z);   // 0x101b1360

// A key track, 0x14 bytes.
struct KeyTrack
{
    char data[0x14];
    // 0x101afe00: the value at `frame` (with `rotation` set, a quaternion interpolated as a rotation).
    int Sample(int frame, float *x, float *y, float *z, float *w, int rotation);
};

struct NodeExtra
{
    KeyTrack translation;          // 0x00
    KeyTrack rotation;             // 0x14
    KeyTrack scale;                // 0x28
    char pad_003c[0x28];
    KeyTrack *translationOverride; // 0x64: used instead of `translation` when set
    KeyTrack *rotationOverride;    // 0x68
    KeyTrack *scaleOverride;       // 0x6c
    Matrix4 AnimatedMatrix(int frame);
};

// 0x101693f0 (NodeExtra::AnimatedMatrix). The node's matrix at `frame`: scale, then rotation, then translation, each
// sampled from its track (or its override).
Matrix4 NodeExtra::AnimatedMatrix(int frame)
{
    Matrix4 result;
    Vec4 value;
    if (scaleOverride != 0)
        scaleOverride->Sample(frame, &value.x, &value.y, &value.z, &value.w, 0);
    else
        scale.Sample(frame, &value.x, &value.y, &value.z, &value.w, 0);
    result = ScaleMatrix(value.x, value.y, value.z);
    if (rotationOverride != 0)
        rotationOverride->Sample(frame, &value.x, &value.y, &value.z, &value.w, 1);
    else
        rotation.Sample(frame, &value.x, &value.y, &value.z, &value.w, 1);
    result = result * RotationMatrix(value);
    if (translationOverride != 0)
        translationOverride->Sample(frame, &value.x, &value.y, &value.z, &value.w, 0);
    else
        translation.Sample(frame, &value.x, &value.y, &value.z, &value.w, 0);
    result = result * TranslationMatrix(value.x, value.y, value.z);
    return result;
}
