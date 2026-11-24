// Attachment slots and smoothing (EN build). JP twin: group_tables.cpp.
#include "group_tables.h"

// Fraction of the remaining distance SmoothApproach covers per round.
const float SMOOTH_FRACTION = 0.55f;
// Halving factor of the step count, and the step count at or below which the smoothing ends.
const float SMOOTH_STEP_FACTOR = 0.5f;
const float SMOOTH_LAST_STEP = 1.0f;
// With `first` set, fewer steps than this leave the value unchanged.
const float SMOOTH_MIN_FIRST_STEPS = 2.0f;

// Stores a copy of `matrix` moved by (x, y, z) in the first free attachment slot and records `owner` for it. Returns
// the slot, or 0 when all six are in use. Called when an object is attached to another.
extern "C" int AttachSlot_Add(const Matrix4 *matrix, float x, float y, float z, int owner)
{
    for (int i = 0; i < ATTACH_SLOT_COUNT; i++)
    {
        if (g_attachOwner[i] == 0)
        {
            g_attachOwner[i] = owner;
            g_attachMatrix[i] = *matrix;
            g_attachMatrix[i].m[MATRIX_TRANSLATION_X] += x;
            g_attachMatrix[i].m[MATRIX_TRANSLATION_Y] += y;
            g_attachMatrix[i].m[MATRIX_TRANSLATION_Z] += z;
            return i;
        }
    }
    return 0;
}

// SmoothApproach: moves `current` 55 % of the way towards `target`, halving `steps` (as an integer) each time, and
// repeats until one step or less is left; the result goes to *out. With `first` set, fewer than 2 steps store
// `current` unchanged. Returns 0. Called by camera and object easing code.
extern "C" int SmoothApproach(float target, float current, float *out, float steps, int first)
{
    if (first != 0 && steps < SMOOTH_MIN_FIRST_STEPS)
    {
        *out = current;
        return 0;
    }
    if (target > current)
    {
        current = (target - current) * SMOOTH_FRACTION + current;
        steps = steps * SMOOTH_STEP_FACTOR;
        steps = (float)(int)steps;
        if (steps <= SMOOTH_LAST_STEP)
        {
            *out = current;
            return 0;
        }
    }
    else
    {
        current = (current - target) * SMOOTH_FRACTION + target;
        steps = steps * SMOOTH_STEP_FACTOR;
        steps = (float)(int)steps;
        if (steps <= SMOOTH_LAST_STEP)
        {
            *out = current;
            return 0;
        }
    }
    SmoothApproach(target, current, out, steps, 0);
    return 0;
}
