// Character status clamping (EN build). Ported from the matched JP source decomp/src/char_status.cpp.
#include "char_attach.h"
#include "char_status.h"

// Char_ClampStatus: limits the counter (CHAR_COUNTER_MAX) and level (CHAR_LEVEL_MAX), clears the flag and keeps the
// larger wake value. A character whose health has run out is marked defeated. Otherwise, in mode CHAR_MODE_FOLLOW, the
// display state becomes CHAR_DISPLAY_FAR when the near count is above CHAR_NEAR_LIMIT and CHAR_DISPLAY_NEAR when not,
// after refreshing the character's offset from the reference position. Returns 0. Called every frame for each slot.
extern "C" int Char_ClampStatus(int index)
{
    int status = index + CHAR_STATUS_SKEW;
    if (g_charCounter[status] > CHAR_COUNTER_MAX)
        g_charCounter[status] = CHAR_COUNTER_MAX;
    if (g_charLevel[status] > CHAR_LEVEL_MAX)
        g_charLevel[status] = CHAR_LEVEL_MAX;
    if (g_charFlag[index + CHAR_STATUS_SKEW] > 0)
        g_charFlag[index + CHAR_STATUS_SKEW] = 0;
    unsigned char value = g_charWakeValue[index + CHAR_STATUS_SKEW];
    if (value != 0)
        g_charHitLevel[index] = g_charHitLevel[index] < value ? value : g_charHitLevel[index];
    if (g_charRecords[status + CHAR_RECORD_SKEW].stats[STAT_HEALTH] > 0)
    {
        g_charRecords[status + CHAR_RECORD_SKEW].defeated = 0;
        if (g_charMode[index + CHAR_STATUS_SKEW] == CHAR_MODE_FOLLOW)
        {
            if (g_charNearCount[index + CHAR_STATUS_SKEW] > CHAR_NEAR_LIMIT)
            {
                if (g_scopePointNode != 0)
                    g_charOffset[index].Copy(g_referencePosition - g_characterModels[index].matrix.GetTranslation());
                g_charDisplayState[index] = CHAR_DISPLAY_FAR;
                return 0;
            }
            if (g_scopePointNode != 0)
                g_charOffset[index].Copy(g_referencePosition - g_characterModels[index].matrix.GetTranslation());
            g_charDisplayState[index] = CHAR_DISPLAY_NEAR;
            return 0;
        }
    }
    else
    {
        g_charRecords[status + CHAR_RECORD_SKEW].stats[STAT_HEALTH] = 0;
        g_charRecords[status + CHAR_RECORD_SKEW].defeated = 1;
    }
    return 0;
}
