// Characters driven by a step script (FUN_10029cf0 runs one step). The character records are viewed from their
// model part at 0x10db6ee8 here (like char_move.cpp), where all the fields used lie inside one record.

#include "vec4.h"

#include "vec3.h"

#include "matrix4.h"

#include "anim_model.h"

#include "char_state.h"

#include "char_vars.h"
#include "map_vars.h"

#include "map_records.h"

#include "char_script.h"

// The larger of a script step's length and SCRIPT_MIN_STEP_FRAMES (written out twice in the original, like a macro).
#define STEP_FRAMES(length) ((length) > SCRIPT_MIN_STEP_FRAMES ? (length) : SCRIPT_MIN_STEP_FRAMES)

// 0x10029cf0 (Char_RunScriptStep). Runs step `step` of the script of character `index`: the script is map record
// AnimModel::bankIndex, one RoutePoint per step packing x * 10000 + y and z * 10000 + extra (each part wrapped to
// -4999..5000), the step kind and its length or parameter. The character is placed at the step's position, and the
// kinds change the opacity value (*value, tenths): 0 fades in and out over the step, 1 holds it at 0, 2 keeps the
// current opacity (counting keepAfterScript down), 3 fades in over 30 frames, 4 fades out, 5 shows it at once and
// starts the voice event `length` of the speaking character. Once the step's frames are up (or at once for kinds 1
// with a reached length, 2 and 5) *doneStep = step, the frame counter restarts and the next step (0 after kind 2) is
// read. Writes the next step to *nextStep, the position to `position`, the step kind to *kind and the extra value to
// *extra. Returns 1 when the character is (still) visible enough to count, else 0.
extern "C" int FUN_10029cf0(int index, int *doneStep, int step, int *nextStep, Vec4 *position, int *kind, int *value,
                            int *extra)
{
    int record = DAT_10db6ee8[index].bankIndex;
    int packedX = DAT_10210fc0[record].points[step].x / 10000;
    int visible = 0;
    if (packedX > 5000)
        packedX -= 10000;
    float x = (float)packedX;
    int packedY = DAT_10210fc0[record].points[step].x % 10000;
    if (packedY > 5000)
        packedY -= 10000;
    float y = (float)packedY;
    int packedZ = DAT_10210fc0[record].points[step].y / 10000;
    if (packedZ > 5000)
        packedZ -= 10000;
    float z = (float)packedZ;
    int packedExtra = DAT_10210fc0[record].points[step].y % 10000;
    if (packedExtra > 5000)
        packedExtra -= 10000;
    *extra = packedExtra;
    *kind = DAT_10210fc0[record].points[step].z;
    *value = DAT_10210fc0[record].points[step].code;
    int next = step;
    int stepKind = *kind;
    int opacity;
    switch (stepKind)
    {
    case SCRIPT_STEP_FADE_IN_OUT:
    {
        int frame = ++DAT_1020b430[index].wait;
        opacity = frame > STEP_FRAMES(*value) - frame ? STEP_FRAMES(*value) - frame : frame;
        if (opacity > 5)
            visible = 1;
        break;
    }
    case SCRIPT_STEP_HIDDEN:
        ++DAT_1020b430[index].wait;
        opacity = 0;
        break;
    case SCRIPT_STEP_KEEP:
        opacity = *value;
        if (DAT_1020b430[index].keepAfterScript > 0)
            DAT_1020b430[index].keepAfterScript--;
        if (DAT_104c0370 == 2)
            DAT_104c0370 = 1;
        break;
    case SCRIPT_STEP_FADE_IN:
        // the frame count, at most SCRIPT_MIN_STEP_FRAMES
        opacity = ++DAT_1020b430[index].wait > SCRIPT_MIN_STEP_FRAMES ? SCRIPT_MIN_STEP_FRAMES
                                                                        : DAT_1020b430[index].wait;
        if (opacity > 2)
            visible = 1;
        break;
    case SCRIPT_STEP_FADE_OUT:
    {
        int frame = ++DAT_1020b430[index].wait;
        opacity = STEP_FRAMES(*value) - frame > 0 ? STEP_FRAMES(*value) - frame : 0;
        if (opacity > 2)
            visible = 1;
        break;
    }
    case SCRIPT_STEP_SHOW:
        if (DAT_104c0364 == index && (DAT_104c0370 != 1 || DAT_104c0374 != index))
        {
            DAT_104c0374 = index;
            FUN_1001dbd0(*value, 1);
        }
        opacity = SCRIPT_MIN_STEP_FRAMES;
        visible = 1;
        break;
    }
    DAT_10db6ee8[index].SetPosition(x, y, z);
    if (DAT_1020b430[index].wait >= STEP_FRAMES(*value) ||
        (DAT_1020b430[index].wait >= *value && *kind == SCRIPT_STEP_HIDDEN) || *kind == SCRIPT_STEP_KEEP ||
        *kind == SCRIPT_STEP_SHOW)
    {
        // the step is over: read the next one
        *doneStep = step;
        DAT_1020b430[index].wait = 0;
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
            opacity = (int)(DAT_10db6ee8[index].alpha * 10.0f);
            if (opacity > 2)
                visible = 1;
        }
        else
        {
            visible = 0;
            opacity = 0;
        }
        int nextX = DAT_10210fc0[record].points[next].x / 10000;
        if (nextX > 5000)
            nextX -= 10000;
        x = (float)nextX;
        int nextY = DAT_10210fc0[record].points[next].x % 10000;
        if (nextY > 5000)
            nextY -= 10000;
        y = (float)nextY;
        int nextZ = DAT_10210fc0[record].points[next].y / 10000;
        if (nextZ > 5000)
            nextZ -= 10000;
        z = (float)nextZ;
        int nextExtra = DAT_10210fc0[record].points[next].y % 10000;
        if (nextExtra > 5000)
            nextExtra -= 10000;
        *extra = nextExtra;
        *kind = DAT_10210fc0[record].points[next].z;
        *value = DAT_10210fc0[record].points[next].code;
    }
    *nextStep = next;
    position->x = x;
    position->y = y;
    position->z = z;
    *value = opacity;
    *kind = stepKind;
    return visible;
}

// 0x1002a340 (Char_RunScript). Runs the script of character `index` for this frame: steps follow each other at once
// while a step returns 5. When the script ends (a result other than 2 and `keep` not set) the character is reset
// and moved out of the world. On the first call its position is added to the camera list. The opacity follows the
// script value (tenths). Returns 0.
extern "C" int FUN_1002a340(int index)
{
    Vec4 position;
    Vec4 unused;
    int a;
    int b;
    int nextStep;
    int result;
    position.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    DAT_10db6ee8[index].mode = 0;
    int value = DAT_1020b430[index].scriptValue;
    do
    {
        FUN_10029cf0(index, &a, DAT_1020b430[index].scriptStep, &nextStep, &position, &result, &value, &b);
        DAT_1020b430[index].scriptValue = value;
        DAT_1020b430[index].scriptResult = result;
        DAT_1020b430[index].scriptStep = nextStep;
    } while (result == 5);
    if (DAT_1020b430[index].keepAfterScript == 0 && result != 2)
    {
        DAT_1020b430[index].scriptResult = 1;
        DAT_1020b430[index].scriptRunning = 0;
        DAT_1020b430[index].scriptStep = 0;
        DAT_1020b430[index].scriptValue = 0;
        DAT_10db6ee8[index].alpha = 0;
        DAT_10db6ee8[index].SetPosition(-5000.0f, -5000.0f, -5000.0f);
    }
    if (DAT_1020b430[index].registered == 0)
    {
        DAT_1032d5b8[DAT_1049f0f0] = &DAT_10db6ee8[index].matrix.translation;
        DAT_1021bbc0[DAT_1049f0f0] = 0;
        DAT_1049f0f0++;
        DAT_1020b430[index].registered = 1;
        DAT_104c036c = 1;
    }
    DAT_10db6ee8[index].alpha = (float)value / 10.0f;
    return 0;
}
