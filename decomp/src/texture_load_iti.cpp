// Loading a texture file (.iti, or the .itp packs of the data folder) into a texture slot.

#include <stdio.h>
#include <string.h>

#include "engine_memory.h"
#include "game_file.h"
#include "map_vars.h"
#include "render_vars.h"
#include "texture_load_iti.h"
#include "texture_object.h"

extern "C" int FUN_10161120(char *out, const char *path);   // Path_GetBaseName
extern "C" int FUN_10161920(const char *name);              // Texture_FindByName

#define ITP_MAGIC_RGB555 999        // .itp: 16-bit pixels, the top bit is the alpha flag
#define ITP_MAGIC_PALETTE 1000      // .itp: 256-entry BGRA palette, then one index per pixel
#define ITI_MAGIC_RGB 0x378         // .iti: RGB pixels, no alpha (the image decoder reads the file)
#define ITI_MAGIC_RGB_ALPHA 0x379   // .iti: RGB pixels and a second image for the alpha
#define ITI_MAGIC_PACKED 0x29a      // .iti: packed (compressed) RGB pixels, no alpha
#define ITI_MAGIC_PACKED_ALPHA 0x29b    // .iti: packed RGB pixels and packed alpha
#define SEARCH_FOLDER_COUNT 4       // the extra folders in DAT_10202b40
#define PALETTE_BYTES 0x400

// Sets `v` to the first free texture slot, TEXTURE_SLOT_COUNT when there is none (open-coded in the original: the
// compiler unrolls the loop at every site, an inline function would not be).
#define FIND_FREE_TEXTURE_SLOT(v) for (v = 0; v < TEXTURE_SLOT_COUNT; v++) { if (DAT_10ffe5f8[v][0] == 0) break; }

// 0x10174ce0 (Texture_LoadIti). Loads texture file `path` and returns its slot, or -1. A texture that is loaded
// already gets one more reference; with `reload` set it is loaded again into the same slot instead. The file is
// opened as given, else as <folder>\<path>.iti in each search folder, else as ./<path>.itp and ./data/<path>.itp;
// the .itp files are read in their own two formats. `flags` goes to TextureObject::Create. The slot is named after
// the file once the texture was created. The ITP branches look for a free slot again after reading (so with `reload`
// the texture may end up in another slot than the one that was unloaded). C++ linkage on purpose: the GameFile
// local gives the function an exception frame, which an extern "C" definition would lose.
int FUN_10174ce0(const char *path, int flags, int reload)
{
    char fullPath[256];
    char baseName[256];
    char palette[PALETTE_BYTES];
    int width;      // ITP size
    int height;
    int imageWidth;     // ITI size
    int imageHeight;
    int packed;     // ITI: 1 = the pixels are stored packed (compressed)
    int format;     // ITI: the file magic, then 0 = no alpha image, 1 = alpha image follows
    int hasAlpha;
    unsigned char *rgb;
    unsigned char *alpha;
    int slot;
    int found;
    strcpy(fullPath, path);
    packed = 0;
    if (fullPath[0] == 0)
        return -1;
    FUN_10161120(baseName, fullPath);
    found = FUN_10161920(baseName);
    if (reload == 0)
    {
        if (found != -1)
        {
            DAT_10fdeb90[found]++;
            return found;
        }
        FIND_FREE_TEXTURE_SLOT(slot);
        if (slot == TEXTURE_SLOT_COUNT)
            return -1;
    }
    else if (found != -1)
    {
        DAT_118d0ba8[found].Unload();
        slot = found;
    }
    else
    {
        FIND_FREE_TEXTURE_SLOT(slot);
        if (slot == TEXTURE_SLOT_COUNT)
            return -1;
    }
    GameFile file;
    if (DAT_10f65538 != 0)
        DAT_10f65538(path, 0);
    int stage = 0;
    if (file.Open(fullPath, "rb") == 0)
    {
        int folder = 0;
        do
        {
            if (folder < SEARCH_FOLDER_COUNT)
            {
                if (DAT_10202b40[folder][0] != 0)
                    sprintf(fullPath, "%s\\%s.iti", DAT_10202b40[folder], path);
                folder++;
            }
            else if (stage == 0)
            {
                sprintf(fullPath, "./%s.itp", path);
                stage = 1;
            }
            else
            {
                if (stage != 1)
                {
                    FUN_1013edb0("\x83" "e\x83N\x83X\x83`\x83\x83\x82\xaa\x82\xc8\x82\xa2", fullPath, 0);
                    return -1;
                }
                sprintf(fullPath, "./data/%s.itp", path);
                stage = 2;
            }
        } while (file.Open(fullPath, "rb") == 0);
    }
    if (stage != 0)
    {
        file.Read(&width, 1, 4);   // the magic; width reads over it below
        if (width == ITP_MAGIC_RGB555)
        {
            file.Read(&width, 1, 4);
            file.Read(&height, 1, 4);
            unsigned short *pixels = (unsigned short *)DAT_11b0b938(width * height * 2, "tmp");
            rgb = (unsigned char *)DAT_11b0b938(width * height * 3, "rgb");
            alpha = (unsigned char *)DAT_11b0b938(width * height * 3, "al");
            file.Read(pixels, 1, width * height * 2);
            file.Close();
            hasAlpha = 0;
            for (int i = 0; i < width * height; i++)
            {
                rgb[i * 3] = (unsigned char)(pixels[i] >> 10) << 3;
                rgb[i * 3 + 1] = (unsigned char)(pixels[i] >> 5) << 3;
                rgb[i * 3 + 2] = (unsigned char)(pixels[i] << 3);
                alpha[i * 3] = (pixels[i] & 0x8000) != 0 ? 0xff : 0;
                alpha[i * 3 + 1] = (pixels[i] & 0x8000) != 0 ? 0xff : 0;
                alpha[i * 3 + 2] = (pixels[i] & 0x8000) != 0 ? 0xff : 0;
                if (alpha[i * 3] != 0xff)
                    hasAlpha = 1;
            }
            if (pixels != 0)
                DAT_11b1ba40(pixels);
            int target; FIND_FREE_TEXTURE_SLOT(target);
            if (target == TEXTURE_SLOT_COUNT)
                return -1;
            if (DAT_1101d840 != 0)
            {
                FUN_10161770(rgb, width, height);
                FUN_10161770(alpha, width, height);
                width = width / 2;
                height = height / 2;
            }
            if (hasAlpha == 0)
            {
                if (alpha != 0)
                    DAT_11b1ba40(alpha);
                alpha = 0;
            }
            if (DAT_118d0ba8[target].Create(width, height, rgb, (int)alpha, flags) != 0)
            {
                strcpy(DAT_10ffe5f8[target], baseName);
                if (rgb != 0)
                    DAT_11b1ba40(rgb);
                if (alpha != 0)
                    DAT_11b1ba40(alpha);
                DAT_10fdeb90[target] = 1;
                return target;
            }
            if (rgb != 0)
                DAT_11b1ba40(rgb);
            if (alpha != 0)
                DAT_11b1ba40(alpha);
            return -1;
        }
        if (width == ITP_MAGIC_PALETTE)
        {
            file.Read(&width, 1, 4);
            file.Read(&height, 1, 4);
            unsigned char *indices = (unsigned char *)DAT_11b0b938(width * height, "tmp");
            rgb = (unsigned char *)DAT_11b0b938(width * height * 3, "rgb");
            alpha = (unsigned char *)DAT_11b0b938(width * height * 3, "al");
            file.Read(palette, 1, PALETTE_BYTES);
            file.Read(indices, 1, width * height);
            file.Close();
            hasAlpha = 0;
            for (int i = 0; i < width * height; i++)
            {
                rgb[i * 3] = palette[indices[i] * 4 + 2];
                rgb[i * 3 + 1] = palette[indices[i] * 4 + 1];
                rgb[i * 3 + 2] = palette[indices[i] * 4];
                alpha[i * 3] = palette[indices[i] * 4 + 3];
                alpha[i * 3 + 1] = palette[indices[i] * 4 + 3];
                alpha[i * 3 + 2] = palette[indices[i] * 4 + 3];
                if (alpha[i * 3] != 0xff)
                    hasAlpha = 1;
            }
            if (indices != 0)
                DAT_11b1ba40(indices);
            int target; FIND_FREE_TEXTURE_SLOT(target);
            if (target == TEXTURE_SLOT_COUNT)
                return -1;
            if (DAT_1101d840 != 0)
            {
                FUN_10161770(rgb, width, height);
                FUN_10161770(alpha, width, height);
                width = width / 2;
                height = height / 2;
            }
            if (hasAlpha == 0)
            {
                if (alpha != 0)
                    DAT_11b1ba40(alpha);
                alpha = 0;
            }
            if (DAT_118d0ba8[target].Create(width, height, rgb, (int)alpha, flags) != 0)
            {
                strcpy(DAT_10ffe5f8[target], baseName);
                if (rgb != 0)
                    DAT_11b1ba40(rgb);
                if (alpha != 0)
                    DAT_11b1ba40(alpha);
                DAT_10fdeb90[target] = 1;
                return target;
            }
            if (rgb != 0)
                DAT_11b1ba40(rgb);
            if (alpha != 0)
                DAT_11b1ba40(alpha);
            return -1;
        }
        file.Close();
        return -1;
    }
    file.Read(&format, 1, 4);
    if (format != ITI_MAGIC_RGB && format != ITI_MAGIC_RGB_ALPHA)
    {
        if (format == ITI_MAGIC_PACKED)
        {
            format = 0;
            packed = 1;
        }
        else if (format == ITI_MAGIC_PACKED_ALPHA)
        {
            format = 1;
            packed = 1;
        }
        file.Read(&imageWidth, 1, 4);
        file.Read(&imageHeight, 1, 4);
        rgb = (unsigned char *)DAT_11b0b938(imageHeight * imageWidth * 3, "rgb");
        if (packed != 0)
            file.ReadPackedImage(rgb, imageWidth, imageHeight);
        else
            file.Read(rgb, 1, imageHeight * imageWidth * 3);
        alpha = 0;
        if (format != 0)
        {
            alpha = (unsigned char *)DAT_11b0b938(imageHeight * imageWidth * 3, "al");
            if (packed != 0)
                file.ReadPackedImage(alpha, imageWidth, imageHeight);
            else
                file.Read(alpha, 1, imageHeight * imageWidth * 3);
        }
        if (packed != 0 && (DAT_1101d844 != 0 || DAT_1101d878 == 0))
        {
            file.Read(rgb, 1, imageHeight * imageWidth * 3);
            if (format != 0)
                file.Read(alpha, 1, imageHeight * imageWidth * 3);
        }
    }
    else
    {
        if (format == ITI_MAGIC_RGB)
            format = 0;
        else
            format = 1;
        file.Read(&imageWidth, 1, 4);
        file.Read(&imageHeight, 1, 4);
        rgb = (unsigned char *)DAT_11b0b938(imageHeight * imageWidth * 3, "rgb");
        alpha = 0;
        if (DAT_1101d844 == 0 && DAT_1101d878 != 0)
        {
            if (format != 0)
            {
                alpha = (unsigned char *)DAT_11b0b938(imageHeight * imageWidth * 3, "al");
                DAT_11b1bb7c(fullPath, rgb, alpha, 0);
            }
            else
                DAT_11b1bb7c(fullPath, rgb, 0, 0);
            if (DAT_1101d844 != 0 || DAT_1101d878 == 0)
            {
                file.Read(rgb, 1, imageHeight * imageWidth * 3);
                if (format != 0)
                    file.Read(alpha, 1, imageHeight * imageWidth * 3);
            }
        }
        else if (format != 0)
        {
            alpha = (unsigned char *)DAT_11b0b938(imageHeight * imageWidth * 3, "al");
            DAT_11b1bb7c(fullPath, rgb, alpha, 1);
        }
        else
            DAT_11b1bb7c(fullPath, rgb, 0, 1);
    }
created:
    file.Close();
    if (DAT_1101d840 != 0)
    {
        FUN_10161770(rgb, imageWidth, imageHeight);
        FUN_10161770(alpha, imageWidth, imageHeight);
        imageWidth = imageWidth / 2;
        imageHeight = imageHeight / 2;
    }
    if (DAT_118d0ba8[slot].Create(imageWidth, imageHeight, rgb, (int)alpha, flags) != 0)
    {
        strcpy(DAT_10ffe5f8[slot], baseName);
        if (rgb != 0)
            DAT_11b1ba40(rgb);
        if (alpha != 0)
            DAT_11b1ba40(alpha);
        DAT_10fdeb90[slot] = 1;
        return slot;
    }
    if (rgb != 0)
        DAT_11b1ba40(rgb);
    if (alpha != 0)
        DAT_11b1ba40(alpha);
    return -1;
}
