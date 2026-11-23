// The registered name substitutions of src_od/names_od.cpp: FUN_1013e6f0 stores a name and the path that replaces
// it, Names_Expand (FUN_1013e790) rewrites a text with them. This is the one declaration of these globals.
#ifndef GURUMIN_NAME_PATHS_H
#define GURUMIN_NAME_PATHS_H

// Characters in a registered name or path, including the terminator.
#define NAME_PATH_SIZE 0x20
// Number of name slots Names_Expand searches.
#define NAME_PATH_SLOTS 4

extern "C" char DAT_10f0ec60[][NAME_PATH_SIZE];   // the registered names (slot = FUN_1013e6f0's index)
extern "C" char DAT_10f0d548[][NAME_PATH_SIZE];   // the path that replaces each name

#endif
