// Texture loading by file name (EN build; JP: decomp/src/texture_load.cpp).

#include <string.h>

#include "engine.h"
#include "texture_mgr.h"

// Folder argument of Path_Data for the data images.
const int PATH_FOLDER_DATA = 0;
// Length of the "iti" extension that marks an indexed-colour image, and no load option.
const int ITI_EXTENSION_LENGTH = 3;
const int NO_LOAD_OPTION = 0;

// Loads the texture called `name` from the data folder: an .iti file when the name ends in "iti", else a .bmp with
// the optional alpha image `alphaName` (0 = none). Returns its slot. EN 0x609bf0 (JP 0x1001e1a0).
extern "C" int Texture_Load(const char *name, const char *alphaName)
{
    if (name[strlen(name) - ITI_EXTENSION_LENGTH] == 'i' && name[strlen(name) - 2] == 't' &&
        name[strlen(name) - 1] == 'i')
        return Texture_LoadIti(Path_Data(name, PATH_FOLDER_DATA), NO_LOAD_OPTION, NO_LOAD_OPTION);
    if (alphaName != 0)
        return Texture_LoadBmp(Path_Data(name, PATH_FOLDER_DATA), Path_Data(alphaName, PATH_FOLDER_DATA), NO_LOAD_OPTION);
    return Texture_LoadBmp(Path_Data(name, PATH_FOLDER_DATA), 0, NO_LOAD_OPTION);
}
