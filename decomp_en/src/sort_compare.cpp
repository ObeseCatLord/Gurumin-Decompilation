// qsort comparators of the status screen item list and the character draw order (EN build, EN-only: no JP twin found).
#include "char.h"
#include "item_info.h"

// Orders item numbers by the sort key of their ItemInfo, ascending. Passed to qsort by the status screen (0x5d55a0).
extern "C" int ItemIndex_CompareBySortKey(const int *a, const int *b)
{
    if (g_itemTable[*a].sortKey < g_itemTable[*b].sortKey)
        return -1;
    return 1;
}

// Orders character slot numbers by their squared distance to the camera target, nearest first. Passed to qsort by the
// character draw pass (0x7198f0).
extern "C" int CharSlot_CompareByDistance(const int *a, const int *b)
{
    if (g_charCameraDistance[*a] < g_charCameraDistance[*b])
        return -1;
    return 1;
}
