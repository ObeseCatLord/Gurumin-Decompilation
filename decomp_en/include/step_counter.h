// A counter that eases towards a target one step per call (EN build, EN-only: no JP twin found). Declared once here.
#ifndef GURUMIN_EN_STEP_COUNTER_H
#define GURUMIN_EN_STEP_COUNTER_H

struct StepCounter
{
    int value;    // 0x00: the current value
    int target;   // 0x04: the value it moves towards

    int StepTowardTarget();   // EN 0x5a8cd0 (step_counter.cpp): moves value one step towards target; returns 0
};

#endif
