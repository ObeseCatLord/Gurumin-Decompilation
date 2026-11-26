// StepCounter (EN build, EN-only: no JP twin found).
#include "step_counter.h"

// Moves the value one step towards the target (no move when they are equal). Returns 0. Called every frame by the
// game's per-slot update at EN 0x60cb10.
int StepCounter::StepTowardTarget()
{
    if (value < target)
        value++;
    if (value > target)
        value--;
    return 0;
}
