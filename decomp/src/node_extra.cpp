// A node's animation data (the extra block of a scene node): three key tracks for translation, rotation and scale,
// each of which can be replaced by another track. Matrix4 has the engine's constructor and empty destructor, so the
// function holding matrices by value gets an exception frame.

#include "vec4.h"

#include "matrix4.h"

Matrix4 ScaleMatrix(float x, float y, float z);         // 0x101b13c0
Matrix4 RotationMatrix(const Vec4 &quaternion);         // 0x101b08c0
Matrix4 TranslationMatrix(float x, float y, float z);   // 0x101b1360

#include "key_track.h"

#include "node_data.h"

// 0x101693f0 (NodeExtra::AnimatedMatrix). The node's matrix at `frame`: scale, then rotation, then translation, each
// sampled from its track (or its override).
Matrix4 NodeExtra::AnimatedMatrix(int frame)
{
    Matrix4 result;
    Vec4 value;
    if (scaleOverride != 0)
        scaleOverride->Sample(frame, &value.x, &value.y, &value.z, &value.w, 0);
    else
        tracks[NODE_TRACK_SCALE].Sample(frame, &value.x, &value.y, &value.z, &value.w, 0);
    result = ScaleMatrix(value.x, value.y, value.z);
    if (rotationOverride != 0)
        rotationOverride->Sample(frame, &value.x, &value.y, &value.z, &value.w, 1);
    else
        tracks[NODE_TRACK_ROTATION].Sample(frame, &value.x, &value.y, &value.z, &value.w, 1);
    result = result * RotationMatrix(value);
    if (translationOverride != 0)
        translationOverride->Sample(frame, &value.x, &value.y, &value.z, &value.w, 0);
    else
        tracks[NODE_TRACK_TRANSLATION].Sample(frame, &value.x, &value.y, &value.z, &value.w, 0);
    result = result * TranslationMatrix(value.x, value.y, value.z);
    return result;
}
