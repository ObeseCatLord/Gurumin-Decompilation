// The animation tracks of a node (EN build). Ported from the matched JP source decomp/src/node_extra.cpp.
#include "containers.h"
#include "node_data.h"

// Quaternion flag of KeyTrack::Sample: the value is a rotation and is interpolated as one.
const int SAMPLE_AS_ROTATION = 1;

// NodeExtra::AnimatedMatrix: the node's matrix at `frame`: scale, then rotation, then translation, each sampled from its
// track (or its override track when one is set). Called for every animated node each frame.
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
        rotationOverride->Sample(frame, &value.x, &value.y, &value.z, &value.w, SAMPLE_AS_ROTATION);
    else
        tracks[NODE_TRACK_ROTATION].Sample(frame, &value.x, &value.y, &value.z, &value.w, SAMPLE_AS_ROTATION);
    result = result * RotationMatrix(value);
    if (translationOverride != 0)
        translationOverride->Sample(frame, &value.x, &value.y, &value.z, &value.w, 0);
    else
        tracks[NODE_TRACK_TRANSLATION].Sample(frame, &value.x, &value.y, &value.z, &value.w, 0);
    result = result * TranslationMatrix(value.x, value.y, value.z);
    return result;
}
