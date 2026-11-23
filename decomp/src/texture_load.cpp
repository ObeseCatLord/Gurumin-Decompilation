// Texture loading by file name: the out-of-line copy of LoadTexture (texture_load.h), plus loaders of fixed
// textures that expand it inline.

#include "texture_load.h"

#include "map_vars.h"
#include "render_vars.h"

extern "C" void FUN_10023d30(void);

// Out-of-line copy of LoadTexture for callers that pass a variable name (same body; a plain forwarder would not
// get the inline expanded here).
extern "C" int FUN_1001e1a0(const char *name, const char *alphaName)
{
    if (name[strlen(name) - 3] == 'i' && name[strlen(name) - 2] == 't' && name[strlen(name) - 1] == 'i')
        return FUN_10174ce0(FUN_1013f8a0(name, 0), 0, 0);
    if (alphaName != 0)
        return FUN_10173550(FUN_1013f8a0(name, 0), FUN_1013f8a0(alphaName, 0), 0);
    return FUN_10173550(FUN_1013f8a0(name, 0), 0, 0);
}

// Resets the count-down state (FUN_10023d30) and loads the count-down digit texture "count_d.iti". Returns 0.
extern "C" int FUN_10050460(void)
{
    DAT_10c636c4 = 0;
    FUN_10023d30();
    DAT_102028c8 = LoadTexture("count_d.iti", 0);
    return 0;
}
