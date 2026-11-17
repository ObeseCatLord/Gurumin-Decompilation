// Texture loading by file name: ".iti" names go to the ITI loader, everything else to the BMP loader (which also
// finds the scrambled ".itm" form, see itm_scramble.cpp).

#include <string.h>

extern "C" const char *FUN_1013f8a0(const char *name, int folder);  // builds the data path of `name`
extern "C" int FUN_10173550(const char *path, const char *alphaPath, int flags);  // Texture_LoadBmp
extern "C" int FUN_10174ce0(const char *path, int a, int b);                      // Texture_LoadIti

// Loads texture `name` and returns its texture handle. BMP textures can take a separate alpha image
// `alphaName` (0 = none); ITI textures carry their own alpha.
extern "C" int FUN_1001e1a0(const char *name, const char *alphaName)
{
    if (name[strlen(name) - 3] == 'i' && name[strlen(name) - 2] == 't' && name[strlen(name) - 1] == 'i')
        return FUN_10174ce0(FUN_1013f8a0(name, 0), 0, 0);
    if (alphaName != 0)
        return FUN_10173550(FUN_1013f8a0(name, 0), FUN_1013f8a0(alphaName, 0), 0);
    return FUN_10173550(FUN_1013f8a0(name, 0), 0, 0);
}
