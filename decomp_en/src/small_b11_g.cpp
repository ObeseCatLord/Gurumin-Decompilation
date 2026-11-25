// EN-only tiny container methods without a JP twin (batch 11, section 4).
#include "index_list.h"

// Constructor of an index or sphere list: no entries, no buffer. Runs for the lists inside static tables.
IndexList::IndexList()
{
    count = 0;
    entries = 0;
}

// Clears the buffer pointer of a sphere list that lives in raw memory. Called before the list is filled.
void IndexList::InitSpheres()
{
    spheres = 0;
}
