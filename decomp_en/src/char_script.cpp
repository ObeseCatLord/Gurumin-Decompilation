// Scripted character steps (EN build). Ported from the matched JP source decomp/src/char_script.cpp.
#include "anim_model.h"
#include "char.h"
#include "camera.h"
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

// Indices in CharState::values of the script variables of a scripted character (the frame counter is SCRIPT_WAIT_VALUE).
const int SCRIPT_STEP_VALUE = 1;       // step the script is at
const int SCRIPT_REGISTERED_VALUE = 2; // 1 once the character's position was added to the camera list
const int SCRIPT_OPACITY_VALUE = 4;    // opacity in tenths
const int SCRIPT_RESULT_VALUE = 6;     // result of the last step (SCRIPT_STEP_KIND_RUNNING = 5)
// Step result that makes the script run the next step in the same frame.
const int SCRIPT_RESULT_CONTINUE = 5;
// Value of SCRIPT_RESULT_VALUE after the script ended.
const int SCRIPT_RESULT_ENDED = 1;
// Where a finished character is parked (far outside the world).
const float SCRIPT_PARKED_POSITION = -5000.0f;

// Char_RunScript: runs the script of character `index` for this frame: steps follow each other at once while a step
// returns 5. When the script ends (a result other than 2 and no frames kept after the script) the character is reset
// and moved out of the world. On the first call its position is added to the camera list. The opacity follows the
// script value (tenths). Returns 0. Called every frame for the scripted scene characters.
extern "C" int Char_RunScript(int index)
{
    Vec4 position;
    Vec4 unused;
    int doneStep;
    int extra;
    int nextStep;
    int result;
    position.Copy(g_characterModels[index].matrix.GetTranslation());
    g_characterModels[index].mode = 0;
    int value = g_charState[index].values[SCRIPT_OPACITY_VALUE];
    do
    {
        Char_RunScriptStep(index, &doneStep, g_charState[index].values[SCRIPT_STEP_VALUE], &nextStep, &position, &result,
                           &value, &extra);
        g_charState[index].values[SCRIPT_OPACITY_VALUE] = value;
        g_charState[index].values[SCRIPT_RESULT_VALUE] = result;
        g_charState[index].values[SCRIPT_STEP_VALUE] = nextStep;
    } while (result == SCRIPT_RESULT_CONTINUE);
    if (g_charState[index].values[SCRIPT_KEEP_VALUE] == 0 && result != SCRIPT_STEP_KEEP)
    {
        g_charState[index].values[SCRIPT_RESULT_VALUE] = SCRIPT_RESULT_ENDED;
        g_charState[index].values[SCRIPT_WAIT_VALUE] = 0;
        g_charState[index].values[SCRIPT_STEP_VALUE] = 0;
        g_charState[index].values[SCRIPT_OPACITY_VALUE] = 0;
        g_characterModels[index].alpha = 0;
        g_characterModels[index].SetPosition(SCRIPT_PARKED_POSITION, SCRIPT_PARKED_POSITION, SCRIPT_PARKED_POSITION);
    }
    if (g_charState[index].values[SCRIPT_REGISTERED_VALUE] == 0)
    {
        g_charState[index].values[SCRIPT_REGISTERED_VALUE] = 1;
        g_scriptCharacterRegistered = 1;
        g_trackedPositions[g_trackedCount] = &g_characterModels[index].matrix.m[MATRIX_TRANSLATION_X];
        g_trackedFlags[g_trackedCount] = 0;
        g_trackedCount++;
    }
    g_characterModels[index].alpha = (float)value / SCRIPT_ALPHA_TENTHS;
    return 0;
}
