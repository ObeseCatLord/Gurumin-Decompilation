// Constructors of the animation containers. The bodies are empty: all the work is done by the member
// constructors (Vec4, KeyTrack, Matrix4, EffectOptions), which the compiler calls in declaration order.

#include "vec4.h"

#include "key_track.h"

#include "anim_channels.h"

// 0x101727f0.
AnimChannels::AnimChannels()
{
}

#include "matrix4.h"

#include "effect_manager.h"

// 0x101728c0.
ModelInstance::ModelInstance()
{
}
