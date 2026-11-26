// Out-of-line rotation setter of an effect model (EN build, EN-only: no JP twin found).
#include "effect_model.h"

// EffectModel::Rotate: marks the model changed and sets its rotation (degrees about x, y, z), exactly like the inline
// SetRotation, but as a separate function that returns 0. No direct caller (reached through a pointer).
int EffectModel::Rotate(float x, float y, float z)
{
    changed = 1;
    rotation.Set(x, y, z, 0);
    return 0;
}
