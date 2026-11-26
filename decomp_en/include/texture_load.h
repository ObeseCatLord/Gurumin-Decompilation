// Texture loading by file name (EN build; JP: decomp/include/texture_load.h). Inline in the original: callers with a
// literal or table name expand it, the out-of-line copy is Texture_Load (EN 0x609bf0). Declared once here.
#ifndef GURUMIN_EN_TEXTURE_LOAD_H
#define GURUMIN_EN_TEXTURE_LOAD_H

#include <string.h>

#include "engine.h"
#include "texture_mgr.h"

// Loads texture `name` from the data folder and returns its texture slot: ".iti" names go to the ITI loader, everything
// else to the BMP loader, which can take a separate alpha image `alphaName` (0 = none).
inline int LoadTexture(const char *name, const char *alphaName)
{
    if (name[strlen(name) - 3] == 'i' && name[strlen(name) - 2] == 't' && name[strlen(name) - 1] == 'i')
        return Texture_LoadIti(Path_Data(name, 0), 0, 0);
    if (alphaName != 0)
        return Texture_LoadBmp(Path_Data(name, 0), Path_Data(alphaName, 0), 0);
    return Texture_LoadBmp(Path_Data(name, 0), 0, 0);
}

#endif
