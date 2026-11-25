// Scripted character steps (EN build; JP: char_script.h, map_vars.h).
#ifndef GURUMIN_EN_CHAR_SCRIPT_H
#define GURUMIN_EN_CHAR_SCRIPT_H

#include "vec4.h"

// Kinds of script step (the step's z value).
enum ScriptStepKind
{
    SCRIPT_STEP_FADE_IN_OUT = 0,   // fades in and back out over the step
    SCRIPT_STEP_HIDDEN = 1,        // invisible for the step's length
    SCRIPT_STEP_KEEP = 2,          // keeps the opacity; ends the script (back to step 0)
    SCRIPT_STEP_FADE_IN = 3,       // fades in over SCRIPT_MIN_STEP_FRAMES frames
    SCRIPT_STEP_FADE_OUT = 4,      // fades out over the step
    SCRIPT_STEP_SHOW = 5           // visible at once; starts the speaker's voice event (the step's length)
};
// Shortest step, and the full opacity value.
const int SCRIPT_MIN_STEP_FRAMES = 30;

extern "C" int g_scriptSpeaker;        // EN 0x00915aac (JP DAT_104c0364): the character whose voice played (0 = none)
extern "C" int g_scriptPlayback;       // EN 0x00915ebc (JP DAT_104c0370): 1 = a voice event runs, 2 = one was asked for
extern "C" int g_scriptVoiceStarted;   // EN 0x00915ec0 (JP DAT_104c0374): the character whose voice event was started
// Starts voice event `event` (`value` 1 = for the speaking character). EN 0x602d10 (JP 0x1001dbd0).
extern "C" int Voice_StartEvent(int event, int value);

// Runs step `step` of the script of character `index` (see char_script.cpp). EN 0x5ecf70 (JP 0x10029cf0).
extern "C" int Char_RunScriptStep(int index, int *doneStep, int step, int *nextStep, Vec4 *position, int *kind, int *value,
                                  int *extra);

#endif
