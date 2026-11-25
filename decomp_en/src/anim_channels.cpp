// Constructor of the placed-model container (EN build). The body is empty: all the work is done by the member
// constructors (Vec4, Matrix4, EffectOptions), which the compiler calls in declaration order. JP twin:
// anim_channels.cpp.
#include "model_instance.h"

// ModelInstance::ModelInstance: constructs the bounding vectors, the three matrices and the effect options. Called
// where a model instance is created.
ModelInstance::ModelInstance()
{
}
