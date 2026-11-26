// Bounding-sphere flags of the mesh table (EN build, EN-only: no JP twin found).
#include "slot_table.h"

// Sets the bounding-sphere flag of every entry to the low byte of `flag`. Returns 0. No direct caller.
int SlotTable::SetAllPointFlags(int flag)
{
    for (int i = 0; i < count; i++)
        pointFlags[i] = flag;
    return 0;
}
