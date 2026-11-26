// Rebuilding generated-alpha textures after a device reset (EN build, EN-only: no JP twin found).
#include "texture_mgr.h"

// Textures_RebuildGenerated: for every used slot whose alpha is generated, uploads the kept source image again
// (source size, source pixels, source alpha mode). Returns 1. Called while recovering from a lost device.
extern "C" int Textures_RebuildGenerated(void)
{
    for (int i = 0; i < TEXTURE_SLOT_COUNT; i++)
    {
        TextureObject &texture = g_textures[i];
        if (g_textureNames[i][0] != 0 && texture.generatedAlpha != 0)
            texture.Upload(texture.sourceWidth, texture.sourceHeight, (unsigned char *)texture.sourcePixels,
                           (const unsigned char *)texture.sourceAlpha);
    }
    return 1;
}
