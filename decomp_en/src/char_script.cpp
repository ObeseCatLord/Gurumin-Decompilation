// Scripted character steps (EN build). Ported from the matched JP source decomp/src/char_script.cpp.
#include "anim_model.h"
#include "char.h"
#include "char_script.h"
#include "map_records.h"

// Digits one packed route value holds (x * 10000 + y).
const int SCRIPT_PACK_BASE = 10000;
// Opacity (tenths) a step has to pass for the character to count as visible after fading in or out.
const int SCRIPT_VISIBLE_FADE_IN_OUT = 5;
const int SCRIPT_VISIBLE_FADE = 2;
// Index in CharState::values of the frame counter of the step, and of the frames kept after the script.
const int SCRIPT_WAIT_VALUE = 0;
const int SCRIPT_KEEP_VALUE = 5;
// Tenths per unit of model alpha.
const float SCRIPT_ALPHA_TENTHS = 10.0f;

// The larger of a script step's length and SCRIPT_MIN_STEP_FRAMES (written out twice in the original, like a macro).
#define STEP_FRAMES(length) ((length) > SCRIPT_MIN_STEP_FRAMES ? (length) : SCRIPT_MIN_STEP_FRAMES)

// Char_RunScriptStep: runs step `step` of the script of character `index`: the script is map record AnimModel::bankIndex,
// one route point per step packing x * 10000 + y and z * 10000 + extra (each part wrapped to -4999..5000), the step kind
// and its length or parameter. The character is placed at the step's position, and the kinds change the opacity value
// (*value, tenths): 0 fades in and out over the step, 1 holds it at 0, 2 keeps the current opacity (counting
// keepAfterScript down), 3 fades in over 30 frames, 4 fades out, 5 shows it at once and starts the voice event `length`
// of the speaking character. Once the step's frames are up (or at once for kinds 1 with a reached length, 2 and 5)
// *doneStep = step, the frame counter restarts and the next step (0 after kind 2) is read. Writes the next step to
// *nextStep, the position to `position`, the step kind to *kind and the extra value to *extra. Returns 1 when the
// character is (still) visible enough to count, else 0.
extern "C" int Char_RunScriptStep(int index, int *doneStep, int step, int *nextStep, Vec4 *position, int *kind, int *value,
                                  int *extra)
{
    int record = g_characterModels[index].bankIndex;
    int packedX = g_mapRecords[record].entry.route[step].x / SCRIPT_PACK_BASE;
    int visible = 0;
    if (packedX > 5000)
        packedX -= 10000;
    float x = (float)packedX;
    int packedY = g_mapRecords[record].entry.route[step].x % SCRIPT_PACK_BASE;
    if (packedY > 5000)
        packedY -= 10000;
    float y = (float)packedY;
    int packedZ = g_mapRecords[record].entry.route[step].y / SCRIPT_PACK_BASE;
    if (packedZ > 5000)
        packedZ -= 10000;
    float z = (float)packedZ;
    int packedExtra = g_mapRecords[record].entry.route[step].y % SCRIPT_PACK_BASE;
    if (packedExtra > 5000)
        packedExtra -= 10000;
    *extra = packedExtra;
    *kind = g_mapRecords[record].entry.route[step].z;
    *value = g_mapRecords[record].entry.route[step].code;
    int next = step;
    int stepKind = *kind;
    int opacity;
    switch (stepKind)
    {
    case SCRIPT_STEP_FADE_IN_OUT:
    {
        int frame = ++g_charState[index].values[SCRIPT_WAIT_VALUE];
        opacity = frame > STEP_FRAMES(*value) - frame ? STEP_FRAMES(*value) - frame : frame;
        if (opacity > SCRIPT_VISIBLE_FADE_IN_OUT)
            visible = 1;
        break;
    }
    case SCRIPT_STEP_HIDDEN:
        ++g_charState[index].values[SCRIPT_WAIT_VALUE];
        opacity = 0;
        break;
    case SCRIPT_STEP_KEEP:
        opacity = *value;
        if (g_charState[index].values[SCRIPT_KEEP_VALUE] > 0)
            g_charState[index].values[SCRIPT_KEEP_VALUE]--;
        if (g_scriptPlayback == 2)
            g_scriptPlayback = 1;
        break;
    case SCRIPT_STEP_FADE_IN:
        opacity = ++g_charState[index].values[SCRIPT_WAIT_VALUE] > SCRIPT_MIN_STEP_FRAMES
                      ? SCRIPT_MIN_STEP_FRAMES
                      : g_charState[index].values[SCRIPT_WAIT_VALUE];
        if (opacity > SCRIPT_VISIBLE_FADE)
            visible = 1;
        break;
    case SCRIPT_STEP_FADE_OUT:
    {
        int frame = ++g_charState[index].values[SCRIPT_WAIT_VALUE];
        opacity = STEP_FRAMES(*value) - frame > 0 ? STEP_FRAMES(*value) - frame : 0;
        if (opacity > SCRIPT_VISIBLE_FADE)
            visible = 1;
        break;
    }
    case SCRIPT_STEP_SHOW:
        if (g_scriptSpeaker == index && (g_scriptPlayback != 1 || g_scriptVoiceStarted != index))
        {
            g_scriptVoiceStarted = index;
            Voice_StartEvent(*value, 1);
        }
        opacity = SCRIPT_MIN_STEP_FRAMES;
        visible = 1;
        break;
    }
    g_characterModels[index].SetPosition(x, y, z);
    if (g_charState[index].values[SCRIPT_WAIT_VALUE] >= STEP_FRAMES(*value) ||
        (g_charState[index].values[SCRIPT_WAIT_VALUE] >= *value && *kind == SCRIPT_STEP_HIDDEN) ||
        *kind == SCRIPT_STEP_KEEP || *kind == SCRIPT_STEP_SHOW)
    {
        *doneStep = step;
        g_charState[index].values[SCRIPT_WAIT_VALUE] = 0;
        switch (*kind)
        {
        case SCRIPT_STEP_FADE_IN_OUT:
        case SCRIPT_STEP_HIDDEN:
        case SCRIPT_STEP_FADE_IN:
        case SCRIPT_STEP_FADE_OUT:
        case SCRIPT_STEP_SHOW:
            next = step + 1;
            break;
        case SCRIPT_STEP_KEEP:
            next = 0;
            break;
        }
        if (*kind == SCRIPT_STEP_SHOW)
            visible = 1;
        else if (*kind == SCRIPT_STEP_FADE_IN)
        {
            opacity = *value;
            visible = 1;
        }
        else if (*kind == SCRIPT_STEP_KEEP)
        {
            opacity = (int)(g_characterModels[index].alpha * SCRIPT_ALPHA_TENTHS);
            if (opacity > SCRIPT_VISIBLE_FADE)
                visible = 1;
        }
        else
        {
            visible = 0;
            opacity = 0;
        }
        int nextX = g_mapRecords[record].entry.route[next].x / SCRIPT_PACK_BASE;
        if (nextX > 5000)
            nextX -= 10000;
        x = (float)nextX;
        int nextY = g_mapRecords[record].entry.route[next].x % SCRIPT_PACK_BASE;
        if (nextY > 5000)
            nextY -= 10000;
        y = (float)nextY;
        int nextZ = g_mapRecords[record].entry.route[next].y / SCRIPT_PACK_BASE;
        if (nextZ > 5000)
            nextZ -= 10000;
        z = (float)nextZ;
        int nextExtra = g_mapRecords[record].entry.route[next].y % SCRIPT_PACK_BASE;
        if (nextExtra > 5000)
            nextExtra -= 10000;
        *extra = nextExtra;
        *kind = g_mapRecords[record].entry.route[next].z;
        *value = g_mapRecords[record].entry.route[next].code;
    }
    *nextStep = next;
    position->x = x;
    position->y = y;
    position->z = z;
    *value = opacity;
    *kind = stepKind;
    return visible;
}
