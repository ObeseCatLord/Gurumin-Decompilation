// Inserting into the timeline of a MotionList (EN build, EN-only: no JP twin found).
#include <string.h>
#include "motion_list.h"

// Position that appends an item at the end of the timeline.
const int MOTION_APPEND = -1;

// MotionList::InsertItem: stores `startTime` in m_0208 first (even when nothing is inserted), then puts `item` into the
// timeline before item `position` (MOTION_APPEND = at the end) and moves the later items up. Returns 0 when `position` is
// beyond the last item or the timeline is full, else 1. Called by ApplyCommandText at EN 0x76b780.
int MotionList::InsertItem(int position, int item, int startTime)
{
    m_0208 = startTime;
    if (count < position)
        return 0;
    if (count > MOTION_ITEM_COUNT)
        return 0;
    if (position == MOTION_APPEND)
    {
        items[count] = item;
        count++;
        return 1;
    }
    for (int i = count; i >= position; i--)
        items[i + 1] = items[i];
    items[position] = item;
    count++;
    return 1;
}

// MotionList::RemoveEntry: takes entry `index` out of the entry table (id, value, extra and name) by moving the later
// entries down one place and counts it. Returns 0 when `index` is not an entry, otherwise 1. No direct caller.
int MotionList::RemoveEntry(int index)
{
    if (entryCount <= index)
        return 0;
    if (index < 0)
        return 0;
    for (int i = index; i < entryCount - 1; i++)
    {
        ids[i] = ids[i + 1];
        values[i] = values[i + 1];
        extras[i] = extras[i + 1];
        strcpy(names[i], names[i + 1]);
    }
    entryCount--;
    return 1;
}

// MotionList::InsertByName: looks up the entry called `name` and inserts its index into the timeline before item
// `position` (MOTION_APPEND = at the end), recording `startTime` (the work of InsertItem, which EN inlines here).
// Returns 0 when there is no entry of that name or the insert fails, otherwise 1. Called by the node animation code.
int MotionList::InsertByName(int position, const char *name, int startTime)
{
    int entry;
    for (entry = 0; entry < entryCount; entry++)
    {
        if (strcmp(name, names[entry]) == 0)
            break;
    }
    if (entry == entryCount)
        return 0;
    return InsertItem(position, entry, startTime);
}
