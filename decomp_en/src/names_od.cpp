// Functions from a translation unit compiled without optimisation but with intrinsics (/Od /Oi): frame pointer,
// every local on the stack, strcpy expanded inline. JP twin: src_od/names_od.cpp.
// Build with: set GURUMIN_EN_CFLAGS=/Od /Oi, then build.bat names_od.cpp
#include <string.h>

#include "name_paths.h"

// Registers `name` in slot `index` with the path that replaces it in Names_Expand. Returns 0.
extern "C" int Names_Set(int index, const char *name, const char *path)
{
    strcpy(g_registeredNames[index], name);
    strcpy(g_registeredPaths[index], path);
    return 0;
}
