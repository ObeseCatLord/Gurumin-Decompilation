// Loading a texture from a .bmp file (or a scrambled .itm copy, a .g32 image, or one of the formats of the host's
// image decoder) into a texture slot.

#include <stdio.h>
#include <string.h>

#include "engine_memory.h"
#include "game_file.h"
#include "map_vars.h"
#include "render_vars.h"
#include "texture_load_bmp.h"
#include "texture_load.h"
#include "texture_load_iti.h"
#include "texture_object.h"

extern "C" int FUN_10161120(char *out, const char *path);   // Path_GetBaseName
extern "C" int FUN_10161920(const char *name);              // Texture_FindByName

#define SEARCH_FOLDER_COUNT 4       // the extra folders in DAT_10202b40
#define BMP_DATA_OFFSET 0x0a        // BMP header: offset of the pixels
#define BMP_WIDTH_OFFSET 0x12
#define BMP_HEIGHT_OFFSET 0x16
#define BMP_BITS_OFFSET 0x1c
#define BMP_PALETTE_OFFSET 0x36     // BMP header: the 8-bit palette (BGRA entries)
#define ITM_TRAILER_BYTES 0x0c      // content type, byte sum and xor check after an .itm body
#define G32_HEADER_BYTES 0x10       // width, height and two more words before the packed planes

// True when the last three characters of `name` are `ext` (any case).
#define HAS_EXTENSION(name, ext) (_strnicmp((name) + strlen(name) - 3, ext, 3) == 0)

// Sets `v` to the first free texture slot, TEXTURE_SLOT_COUNT when there is none (open-coded in the original: the
// compiler unrolls the loop at every site).
#define FIND_FREE_TEXTURE_SLOT(v) for (v = 0; v < TEXTURE_SLOT_COUNT; v++) { if (DAT_10ffe5f8[v][0] == 0) break; }

// 0x10173550 (Texture_LoadBmp). Loads the texture file `path` and returns its slot, or -1 (0 when a file was found but
// unusable). `alphaPath` names a second image whose pixels become the alpha channel (0 = none). A texture that is
// loaded already is returned as it is; with `reload` set it is loaded again into its slot. The image comes from
// <path without extension>.g32 when that exists, else from the host's decoder for psd/jpg/tga/png, else from the
// .bmp (or its scrambled .itm copy), searched in the extra folders; 8-bit BMPs are expanded to RGB and flipped.
// Pixels of the key colours (magenta, blue) get alpha 0 when DAT_1101d848 is set. C++ linkage on purpose: the
// GameFile local gives the function an exception frame.
int FUN_10173550(const char *path, const char *alphaPath, int reload)
{
    char fullPath[256];
    char alphaFull[256];
    char baseName[256];
    char alphaBase[256];
    char nameCopy[256];
    char tempA[256];
    char tempB[256];
    char tempC[256];
    char tempD[256];
    char tempE[256];
    char extension[8];
    int width;
    int height;
    unsigned char *rgb;
    unsigned char *alpha;
    void *rgbOwner;
    void *alphaOwner;
    char *alphaName;
    int slot;
    int itm;
    int folder;
    int size;
    int type;
    int sum;
    unsigned int check;
    unsigned char *bmp;
    strcpy(fullPath, path);
    alphaName = 0;
    folder = 0;
    if (alphaPath != 0)
    {
        strcpy(alphaFull, alphaPath);
        alphaName = alphaFull;
    }
    baseName[0] = 0;
    memset(baseName + 1, 0, 255);
    alphaBase[0] = 0;
    memset(alphaBase + 1, 0, 255);
    if (fullPath[0] == 0)
        return -1;
    FUN_10161120(baseName, fullPath);
    if (alphaPath != 0)
        FUN_10161120(alphaBase, alphaPath);
    slot = FUN_10161920(baseName);
    if (reload == 0)
    {
        if (slot != -1)
            return slot;
        FIND_FREE_TEXTURE_SLOT(slot);
        if (slot == TEXTURE_SLOT_COUNT)
            return -1;
    }
    else if (slot == -1)
    {
        FIND_FREE_TEXTURE_SLOT(slot);
        if (slot == TEXTURE_SLOT_COUNT)
            return -1;
    }
    else
        DAT_118d0ba8[slot].Unload();
    if (DAT_10f65538 != 0)
        DAT_10f65538(0, 0);
    alphaOwner = 0;
    rgbOwner = 0;
    alpha = 0;
    strcpy(nameCopy, fullPath);
    char *dot = strrchr(nameCopy, '.');
    if (dot != 0)
        *dot = 0;
    strcat(nameCopy, ".g32");
    GameFile file;
    if (file.Open(nameCopy, "rb") != 0)
    {
        file.Seek(0, 2);
        size = file.Size();
        file.Seek(0, 0);
        int *packed = (int *)DAT_11b0b938(size, "pp");
        file.Read(packed, 1, size);
        file.Close();
        width = packed[0];
        height = packed[1];
        int pixels = height * width;
        unsigned char *planes = (unsigned char *)DAT_11b0b938(pixels * 4, "tmp");
        if (FUN_101612b0(planes, pixels * 4, (unsigned char *)(packed + 4), size - G32_HEADER_BYTES) == 0)
        {
            FUN_1013edb0("G32\x82\xcc\x93W\x8aJ\x82\xc9\x8e\xb8\x94s", fullPath, 0);
            return 0;
        }
        rgb = (unsigned char *)DAT_11b0b938(pixels * 3, "rgb");
        alpha = (unsigned char *)DAT_11b0b938(pixels * 3, "al");
        unsigned int plane = (unsigned int)(pixels * 4) >> 2;
        unsigned char *src = planes;
        unsigned char *outRgb = rgb;
        unsigned char *outAlpha = alpha;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                outAlpha[0] = src[plane * 3];
                outAlpha[1] = src[plane * 3];
                outAlpha[2] = src[plane * 3];
                outRgb[0] = src[0];
                outRgb[1] = src[plane];
                outRgb[2] = src[plane * 2];
                src++;
                outRgb += 3;
                outAlpha += 3;
            }
        }
        DAT_11b1ba40(packed);
        DAT_11b1ba40(planes);
        rgbOwner = rgb;
        alphaOwner = alpha;
        goto imageLoaded;
    }
    if (!HAS_EXTENSION(path, "bmp") &&
        (HAS_EXTENSION(path, "psd") || HAS_EXTENSION(path, "jpg") || HAS_EXTENSION(path, "tga") ||
         HAS_EXTENSION(path, "png")))
    {
        if (HAS_EXTENSION(path, "psd"))
            strcpy(extension, ".psd");
        if (HAS_EXTENSION(path, "jpg"))
            strcpy(extension, ".jpg");
        if (HAS_EXTENSION(path, "tga"))
            strcpy(extension, ".tga");
        if (HAS_EXTENSION(path, "png"))
            strcpy(extension, ".png");
        if (DAT_1101d84c != 0)
        {
            sprintf(tempA, path);
            if (file.Open(fullPath, "rb") == 0)
            {
                do
                {
                    if (folder < SEARCH_FOLDER_COUNT)
                        sprintf(fullPath, "%s\\%s", DAT_10202b40[folder], tempA);
                    else if (folder < 2 * SEARCH_FOLDER_COUNT)
                    {
                        FUN_10161120(tempB, tempA);
                        sprintf(fullPath, "%s\\%s%s", DAT_10202b40[folder - SEARCH_FOLDER_COUNT], tempB, extension);
                    }
                    else if (folder < 3 * SEARCH_FOLDER_COUNT)
                    {
                        FUN_10161120(tempC, tempA);
                        sprintf(fullPath, "%s\\images\\%s%s", DAT_10202b40[folder - 2 * SEARCH_FOLDER_COUNT], tempC,
                                extension);
                    }
                    else
                        goto openAsItm;
                    folder++;
                } while (file.Open(fullPath, "rb") == 0);
            }
            file.Close();
            if (DAT_1101d84c(fullPath, &width, &height, &rgb, &alpha) == 0)
                return 0;
            if (alpha == 0)
                goto imageLoaded;
            goto createTexture;
        }
    }
    itm = 0;
    if (HAS_EXTENSION(path, "bmp") || HAS_EXTENSION(path, "itm"))
    {
        strncpy(fullPath + strlen(fullPath) - 3, "itm", 3);
openAsItm:
        if (file.Open(fullPath, "rb") == 0)
        {
            do
            {
                if (folder < SEARCH_FOLDER_COUNT)
                {
                    FUN_10161120(tempE, path);
                    sprintf(fullPath, "%s\\%s.itm", DAT_10202b40[folder], tempE);
                }
                else if (folder < 2 * SEARCH_FOLDER_COUNT)
                {
                    FUN_10161120(tempC, path);
                    sprintf(fullPath, "%s\\%s.itm", DAT_10202b40[folder - SEARCH_FOLDER_COUNT], tempC);
                }
                else if (folder < 3 * SEARCH_FOLDER_COUNT)
                {
                    FUN_10161120(tempD, path);
                    sprintf(fullPath, "%s\\images\\%s.itm", DAT_10202b40[folder - 2 * SEARCH_FOLDER_COUNT], tempD);
                }
                else
                {
                    strcpy(fullPath, path);
                    goto searchBmp;
                }
                folder++;
            } while (file.Open(fullPath, "rb") == 0);
        }
        itm = 1;
        goto readMain;
    }
searchBmp:
    folder = 0;
    itm = 0;
searchBmpFolders:
    if (file.Open(fullPath, "rb") == 0)
    {
        do
        {
            if (folder < SEARCH_FOLDER_COUNT)
                sprintf(fullPath, "%s\\%s", DAT_10202b40[folder], path);
            else if (folder < 2 * SEARCH_FOLDER_COUNT)
            {
                FUN_10161120(tempA, path);
                sprintf(fullPath, "%s\\%s.bmp", DAT_10202b40[folder - SEARCH_FOLDER_COUNT], tempA);
            }
            else if (folder < 3 * SEARCH_FOLDER_COUNT)
            {
                FUN_10161120(tempB, path);
                sprintf(fullPath, "%s\\images\\%s.bmp", DAT_10202b40[folder - 2 * SEARCH_FOLDER_COUNT], tempB);
            }
            else
            {
                FUN_1013edb0("\x83" "e\x83N\x83X\x83`\x83\x83\x82\xaa\x82\xc8\x82\xa2", fullPath, 0);
                goto failMinusOne;
            }
            folder++;
        } while (file.Open(fullPath, "rb") == 0);
    }
readMain:
    file.Seek(0, 2);
    size = file.Size();
    file.Seek(0, 0);
    if (itm != 0)
        size -= ITM_TRAILER_BYTES;
    alphaOwner = 0;
    bmp = (unsigned char *)DAT_11b0b938(size, "p");
    rgbOwner = bmp;
    file.Read(bmp, 1, size);
    if (itm != 0)
    {
        file.Read(&type, 1, 4);
        file.Read(&sum, 1, 4);
        file.Read(&check, 1, 4);
        if (type != 0 || FUN_101615d0(bmp, size, baseName, sum, check) == 0)
        {
            DAT_11b1ba40(bmp);
            goto failZero;
        }
    }
    file.Close();
    width = *(int *)(bmp + BMP_WIDTH_OFFSET);
    height = *(int *)(bmp + BMP_HEIGHT_OFFSET);
    rgb = bmp + *(int *)(bmp + BMP_DATA_OFFSET);
    if (*(short *)(bmp + BMP_BITS_OFFSET) == 8)
    {
        unsigned char *expanded =
            (unsigned char *)DAT_11b0b938(*(int *)(bmp + BMP_HEIGHT_OFFSET) * *(int *)(bmp + BMP_WIDTH_OFFSET) * 3, "ppp");
        for (int i = 0; i < *(int *)(bmp + BMP_HEIGHT_OFFSET) * *(int *)(bmp + BMP_WIDTH_OFFSET); i++)
        {
            expanded[i * 3] = bmp[rgb[i] * 4 + BMP_PALETTE_OFFSET];
            expanded[i * 3 + 1] = bmp[rgb[i] * 4 + BMP_PALETTE_OFFSET + 1];
            expanded[i * 3 + 2] = bmp[rgb[i] * 4 + BMP_PALETTE_OFFSET + 2];
        }
        DAT_11b1ba40(bmp);
        rgb = expanded;
        rgbOwner = expanded;
    }
    FUN_10161200(rgb, width, height);
imageLoaded:
    if (DAT_1101d848 != 0 && alpha == 0)
    {
        alpha = (unsigned char *)DAT_11b0b938(height * width * 3, "ppp");
        int keyed = 0;
        alphaOwner = alpha;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                if (rgb[(y * width + x) * 3] == 0xff && rgb[(y * width + x) * 3 + 1] == 0 &&
                    rgb[(y * width + x) * 3 + 2] == 0xff)
                {
                    alpha[(y * width + x) * 3] = 0;
                    alpha[(y * width + x) * 3 + 1] = 0;
                    alpha[(y * width + x) * 3 + 2] = 0;
                    rgb[(y * width + x) * 3] = 0;
                    rgb[(y * width + x) * 3 + 1] = 0;
                    rgb[(y * width + x) * 3 + 2] = 0;
                    keyed = 1;
                }
                else if (rgb[(y * width + x) * 3] == 0 && rgb[(y * width + x) * 3 + 1] == 0 &&
                         rgb[(y * width + x) * 3 + 2] == 0xff)
                {
                    alpha[(y * width + x) * 3] = 0;
                    alpha[(y * width + x) * 3 + 1] = 0;
                    alpha[(y * width + x) * 3 + 2] = 0;
                    rgb[(y * width + x) * 3] = 0;
                    rgb[(y * width + x) * 3 + 1] = 0;
                    rgb[(y * width + x) * 3 + 2] = 0;
                    keyed = 1;
                }
                else
                {
                    alpha[(y * width + x) * 3] = 0xff;
                    alpha[(y * width + x) * 3 + 1] = 0xff;
                    alpha[(y * width + x) * 3 + 2] = 0xff;
                }
            }
        }
        if (keyed == 0)
        {
            DAT_11b1ba40(alpha);
            alpha = 0;
            alphaOwner = 0;
        }
    }
    if (alphaName == 0 || alpha != 0)
        goto createTexture;
    int alphaItm;
    int k;
    if (!HAS_EXTENSION(alphaName, "bmp") && !HAS_EXTENSION(alphaName, "itm") &&
        (HAS_EXTENSION(alphaName, "psd") || HAS_EXTENSION(alphaName, "jpg") || HAS_EXTENSION(alphaName, "tga") ||
         HAS_EXTENSION(alphaName, "png")))
    {
        if (HAS_EXTENSION(alphaName, "psd"))
            strcpy(extension, ".psd");
        if (HAS_EXTENSION(alphaName, "jpg"))
            strcpy(extension, ".jpg");
        if (HAS_EXTENSION(alphaName, "tga"))
            strcpy(extension, ".tga");
        if (HAS_EXTENSION(alphaName, "png"))
            strcpy(extension, ".png");
        if (DAT_1101d84c != 0)
        {
            sprintf(tempA, alphaName);
            folder = 0;
            for (;;)
            {
                if (file.Open(alphaName, "rb") != 0)
                {
                    file.Close();
                    if (DAT_1101d84c(alphaName, &width, &height, &alpha, 0) != 0)
                        goto createTexture;
                    break;
                }
                if (folder < SEARCH_FOLDER_COUNT)
                    sprintf(alphaName, "%s\\%s", DAT_10202b40[folder], tempA);
                else if (folder < 2 * SEARCH_FOLDER_COUNT)
                {
                    FUN_10161120(tempB, tempA);
                    sprintf(alphaName, "%s\\%s%s", DAT_10202b40[folder - SEARCH_FOLDER_COUNT], tempB, extension);
                }
                else if (folder < 3 * SEARCH_FOLDER_COUNT)
                {
                    FUN_10161120(tempC, tempA);
                    sprintf(alphaName, "%s\\images\\%s%s", DAT_10202b40[folder - 2 * SEARCH_FOLDER_COUNT], tempC,
                            extension);
                }
                else
                    goto searchBmpFolders;
                folder++;
            }
        }
    }
    if (HAS_EXTENSION(alphaName, "bmp") || HAS_EXTENSION(alphaName, "itm"))
    {
        strncpy(alphaName + strlen(alphaName) - 3, "itm", 3);
        if (file.Open(alphaName, "rb") == 0)
        {
            k = 0;
            do
            {
                if (k < SEARCH_FOLDER_COUNT)
                {
                    FUN_10161120(tempD, alphaPath);
                    sprintf(alphaName, "%s\\%s.itm", DAT_10202b40[k], tempD);
                }
                else if (k < 2 * SEARCH_FOLDER_COUNT)
                {
                    FUN_10161120(tempE, alphaPath);
                    sprintf(alphaName, "%s\\%s.itm", DAT_10202b40[k - SEARCH_FOLDER_COUNT], tempE);
                }
                else if (k < 3 * SEARCH_FOLDER_COUNT)
                {
                    FUN_10161120(tempB, alphaPath);
                    sprintf(alphaName, "%s\\images\\%s.itm", DAT_10202b40[k - 2 * SEARCH_FOLDER_COUNT], tempB);
                }
                else
                {
                    strcpy(alphaName, alphaPath);
                    goto alphaBmp;
                }
                k++;
            } while (file.Open(alphaName, "rb") == 0);
        }
        alphaItm = 1;
    }
    else
    {
alphaBmp:
        alphaItm = 0;
        if (file.Open(alphaName, "rb") == 0)
        {
            k = 0;
            do
            {
                if (k < SEARCH_FOLDER_COUNT)
                    sprintf(alphaName, "%s\\%s", DAT_10202b40[k], path);
                else if (k < 2 * SEARCH_FOLDER_COUNT)
                {
                    FUN_10161120(tempC, alphaPath);
                    sprintf(alphaName, "%s\\%s.bmp", DAT_10202b40[k - SEARCH_FOLDER_COUNT], tempC);
                }
                else if (k < 3 * SEARCH_FOLDER_COUNT)
                {
                    FUN_10161120(tempD, alphaPath);
                    sprintf(alphaName, "%s\\images\\%s.bmp", DAT_10202b40[k - 2 * SEARCH_FOLDER_COUNT], tempD);
                }
                else
                {
                    DAT_11b1ba40(rgbOwner);
                    FUN_1013edb0("\x83" "e\x83N\x83X\x83`\x83\x83\x82\xaa\x82\xc8\x82\xa2", alphaPath, 0);
                    goto failZero;
                }
                k++;
            } while (file.Open(alphaName, "rb") == 0);
        }
    }
    file.Seek(0, 2);
    size = file.Size();
    file.Seek(0, 0);
    if (alphaItm != 0)
        size -= ITM_TRAILER_BYTES;
    bmp = (unsigned char *)DAT_11b0b938(size, "p2");
    alphaOwner = bmp;
    file.Read(bmp, 1, size);
    if (alphaItm != 0)
    {
        file.Read(&type, 1, 4);
        file.Read(&sum, 1, 4);
        file.Read(&check, 1, 4);
        if (type != 0 || FUN_101615d0(bmp, size, alphaBase, sum, check) == 0)
        {
            DAT_11b1ba40(rgbOwner);
            DAT_11b1ba40(bmp);
failZero:
            return 0;
        }
    }
    file.Close();
    alpha = bmp + *(int *)(bmp + BMP_DATA_OFFSET);
    if (*(short *)(bmp + BMP_BITS_OFFSET) == 8)
    {
        unsigned char *expanded =
            (unsigned char *)DAT_11b0b938(*(int *)(bmp + BMP_WIDTH_OFFSET) * *(int *)(bmp + BMP_HEIGHT_OFFSET) * 3, "ppp");
        for (int i = 0; i < *(int *)(bmp + BMP_WIDTH_OFFSET) * *(int *)(bmp + BMP_HEIGHT_OFFSET); i++)
        {
            expanded[i * 3] = bmp[alpha[i] * 4 + BMP_PALETTE_OFFSET];
            expanded[i * 3 + 1] = bmp[alpha[i] * 4 + BMP_PALETTE_OFFSET + 1];
            expanded[i * 3 + 2] = bmp[alpha[i] * 4 + BMP_PALETTE_OFFSET + 2];
        }
        DAT_11b1ba40(bmp);
        alpha = expanded;
        alphaOwner = expanded;
    }
    FUN_10161200(alpha, width, height);
createTexture:
    if (DAT_118d0ba8[slot].Create(width, height, rgb, (int)alpha, 0) != 0)
    {
        strcpy(DAT_10ffe5f8[slot], baseName);
        strcpy(DAT_10f8dbb8[slot], alphaBase);
        if (rgbOwner != 0)
            DAT_11b1ba40(rgbOwner);
        if (alphaOwner != 0)
            DAT_11b1ba40(alphaOwner);
        DAT_10fdeb90[slot] = 1;
        return slot;
    }
    if (rgbOwner != 0)
        DAT_11b1ba40(rgbOwner);
    if (alphaOwner != 0)
        DAT_11b1ba40(alphaOwner);
failMinusOne:
    return -1;
}
