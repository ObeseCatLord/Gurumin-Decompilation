// Inserting into the timeline of a MotionList (EN build, EN-only: no JP twin found).
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
