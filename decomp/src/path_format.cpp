// Data path formatting.

#include <stdio.h>

#include "map_vars.h"

// Returns "<folder>\\<name>" for data folder number `folder` (folder names in DAT_10202b40). The result is one
// of 16 rotating 256-byte buffers, so it stays valid for the next 15 calls. Used for nearly every file load.
extern "C" const char *FUN_1013f8a0(const char *name, int folder)
{
    DAT_10f1fd24++;
    sprintf(DAT_10f19860[DAT_10f1fd24 & 15], "%s\\%s", DAT_10202b40[folder], name);
    return DAT_10f19860[DAT_10f1fd24 & 15];
}
