// The map cursor step (EN build; JP twin APP.dll 10051190): moves a marker along a map record's route and follows a
// rider that lands on it.
#include "anim_model.h"
#include "char.h"
#include "map_cursor.h"
#include "map_records.h"

// See the declaration in map_cursor.h.
extern "C" int MapCursor_Step(int index, int step, int *outStep, float *x, float *y, int mode, int target, int unused,
                              int *kind, int *previousStep, int *wasRidden)
{
    if (wasRidden == 0)
        return 0;
    int landed = Char_IsStandingOn(index, 0, CURSOR_RIDER_HEIGHT, CURSOR_RIDER_DISTANCE_SQUARED);
    int held = landed;
    if (landed != 0)
        g_charState[index].values[CURSOR_VALUE_HOLD] = CURSOR_ARRIVAL_HOLD_FRAMES;
    if (g_charState[index].values[CURSOR_VALUE_HOLD] > 0)
    {
        held = 1;
        g_charState[index].values[CURSOR_VALUE_HOLD]--;
    }
    if (g_charRecords[index].defeated != 0 && held != 0)
        g_characterModels[index].speedScale = g_characterModels[index].speedScale * 2.0f;
    if (kind != 0 && previousStep != 0 && g_charState[index].values[CURSOR_VALUE_HOLD] > 0)
    {
        if (held != 0 && *wasRidden == 0 && landed != 0)
        {
            if (*previousStep > step && *kind == CURSOR_NODE_RIDER_STOP)
                step = *previousStep;
            else if (*previousStep == step && *kind == CURSOR_NODE_RIDER_STOP)
                step = step + 1;
        }
        else if (landed == 0 && g_charState[index].values[CURSOR_VALUE_HOLD] == 1)
        {
            g_charState[index].values[CURSOR_VALUE_HOLD] = 0;
            if (*previousStep < step && *kind == CURSOR_NODE_RIDER_STOP)
                step = *previousStep;
            else if (*previousStep == step && *kind == CURSOR_NODE_RIDER_STOP)
                step = step - 1;
        }
        *wasRidden = landed;
        g_mapCursorRidden = landed != 0;
    }
    float nodeX = (float)g_mapRecords[g_charRecords[index].routeRecord].points[step].x;
    float nodeY = (float)g_mapRecords[g_charRecords[index].routeRecord].points[step].y;
    if (nodeX - CURSOR_NODE_RANGE < *x && nodeX + CURSOR_NODE_RANGE > *x && nodeY - CURSOR_NODE_RANGE < *y &&
        nodeY + CURSOR_NODE_RANGE > *y)
    {
        if (previousStep != 0)
            *previousStep = step;
        if (kind != 0)
            *kind = g_mapRecords[g_charRecords[index].routeRecord].points[step].z;
        if (mode == CURSOR_MODE_JUMP)
        {
            if (target != CURSOR_TARGET_NONE)
                step = target;
            else if (g_mapRecords[g_charRecords[index].routeRecord].points[step].z == CURSOR_NODE_JUMP &&
                     g_mapRecords[g_charRecords[index].routeRecord].points[step].code > -1)
                step = g_mapRecords[g_charRecords[index].routeRecord].points[step].code;
        }
        else if (mode == CURSOR_MODE_ENTRY)
        {
            if (target != CURSOR_TARGET_NONE)
                step = target;
            else if (g_mapRecords[g_charRecords[index].routeRecord].points[step].z == CURSOR_NODE_ENTRY)
            {
                int entry = g_mapRecords[g_charRecords[index].routeRecord].points[step].code;
                step = entry < 0 ? 0 : entry;
            }
        }
        else
        {
            int nodeKind = g_mapRecords[g_charRecords[index].routeRecord].points[step].z;
            if (nodeKind == CURSOR_NODE_ENTRY)
            {
                int entry = g_mapRecords[g_charRecords[index].routeRecord].points[step].code;
                step = entry < 0 ? 0 : entry;
            }
            else if (nodeKind == CURSOR_NODE_RIDER_STOP)
            {
                g_charRecords[index].defeated = 1;
                g_charRecords[index].stats[CURSOR_STAT_RELEASE] = -1;
                g_charGroup[index] = -1;
                if (held != 0 && g_charState[index].values[CURSOR_VALUE_HOLD] > CURSOR_RIDER_SPEED_HOLD)
                {
                    g_characterModels[index].speedScale = g_characterModels[index].speedScale * CURSOR_RIDER_SPEED_FACTOR;
                    step = step + 1;
                }
                else
                {
                    g_charState[index].values[CURSOR_VALUE_HOLD] = 0;
                    step = step - 1;
                }
            }
            else if (nodeKind == CURSOR_NODE_BACK)
                step = step - 1;
            else if (nodeKind != CURSOR_NODE_PORTAL_FORWARD)
                step = step + 1;
        }
        int landedKind = g_mapRecords[g_charRecords[index].routeRecord].points[step].z;
        if (landedKind == CURSOR_NODE_PORTAL)
        {
            int link = g_mapRecords[g_charRecords[index].routeRecord].points[step].code;
            step = SignedField(link % CURSOR_PORTAL_DIGITS);
            g_charRecords[index].routeRecord = SignedField(link / CURSOR_PORTAL_DIGITS);
            if (previousStep != 0)
                *previousStep = step - 1;
            if (kind != 0)
                *kind = g_mapRecords[g_charRecords[index].routeRecord].points[step].z;
        }
        else if (landedKind == CURSOR_NODE_PORTAL_FORWARD)
        {
            int link = g_mapRecords[g_charRecords[index].routeRecord].points[step].code;
            step = SignedField(link % CURSOR_PORTAL_DIGITS);
            g_charRecords[index].routeRecord = SignedField(link / CURSOR_PORTAL_DIGITS);
            if (previousStep != 0)
                *previousStep = step + 1;
            if (kind != 0)
                *kind = g_mapRecords[g_charRecords[index].routeRecord].points[step].z;
        }
        nodeX = (float)g_mapRecords[g_charRecords[index].routeRecord].points[step].x;
        nodeY = (float)g_mapRecords[g_charRecords[index].routeRecord].points[step].y;
    }
    *outStep = step;
    *x = nodeX;
    *y = nodeY;
    return 0;
}
