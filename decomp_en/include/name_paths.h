// The registered name substitutions: Names_Set stores a name and the path that replaces it, Names_Expand rewrites a
// text with them (JP: decomp/include/name_paths.h). This is the one declaration of these globals in decomp_en.
#ifndef GURUMIN_EN_NAME_PATHS_H
#define GURUMIN_EN_NAME_PATHS_H

// Characters in a registered name or path, including the terminator.
const int NAME_PATH_SIZE = 0x20;
// Number of name slots Names_Expand searches.
const int NAME_PATH_SLOTS = 4;

extern "C" char g_registeredNames[NAME_PATH_SLOTS][NAME_PATH_SIZE];   // EN 0x021e8d78 (JP DAT_10f0ec60): the registered names
extern "C" char g_registeredPaths[NAME_PATH_SLOTS][NAME_PATH_SIZE];   // EN 0x021e8df8 (JP DAT_10f0d548): the path that replaces each name

// Registers `name` in slot `index` with the path that replaces it. EN 0x7a5ca0 (JP 0x1013e6f0).
extern "C" int Names_Set(int index, const char *name, const char *path);

// Copies `text` to `out`, replacing every registered name by its path. EN 0x7a5d40 (JP 0x1013e790).
extern "C" int Names_Expand(char *out, const char *text);

#endif
