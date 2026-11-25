// EN-only tiny methods without a JP twin (batch 11, section 4): the input recording's setters.
#include "demo_record.h"

// Sets value `index` of the frame being built. Returns 1. Called by the main loop when it gathers the input.
int DemoRecording::SetValue(int value, int index)
{
    values_now[index] = value;
    return 1;
}

// Sets float `index` of the frame being built. Returns 1. Called by the main loop when it gathers the input.
int DemoRecording::SetFloat(float value, int index)
{
    floats_now[index] = value;
    return 1;
}
