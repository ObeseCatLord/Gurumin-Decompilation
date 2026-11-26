// Loader of the Packs "BMP" image container (EN build: Packs.dll is linked into the exe). JP twin: Packs.dll 10005c00.
#include "packs_codec.h"

// Opens a Packs image file and reads one or two RGB images from it. The file starts with a magic number that says
// whether a second (alpha) image follows. When `decodeJpeg` is 0 the images are JPEG streams that are decoded into
// `colourPixels` (and `alphaPixels`); otherwise they are raw width * height * 3 byte rasters read straight into the
// buffers. Returns 1 on success, 0 when the file cannot be opened or the magic number is unknown.
int Packs_LoadImageFile(const char *path, unsigned char *colourPixels, unsigned char *alphaPixels, int decodeJpeg)
{
    GameFile file;
    int loaded;
    if (!file.Open(path, PACKS_OPEN_MODE))
    {
        loaded = 0;
    }
    else
    {
        int field;   // the magic number first, then the size of each JPEG stream
        int hasAlpha;
        file.Read(&field, 1, sizeof(field));
        if (field == PACKS_MAGIC_COLOUR)
            hasAlpha = 0;
        else if (field == PACKS_MAGIC_COLOUR_ALPHA)
            hasAlpha = 1;
        else
        {
            file.Close();
            loaded = 0;
            return loaded;
        }

        int width, height;
        file.Read(&width, 1, sizeof(width));
        file.Read(&height, 1, sizeof(height));
        file.Read(&field, 1, sizeof(field));
        int dataStart = file.Size();

        if (decodeJpeg != 0)
        {
            file.Seek(dataStart + field, 0);
            if (hasAlpha)
            {
                file.Read(&field, 1, sizeof(field));
                file.Seek(file.Size() + field, 0);
            }
            file.Read(colourPixels, 1, height * width * PACKS_IMAGE_BYTES_PER_PIXEL);
            if (hasAlpha)
                file.Read(alphaPixels, 1, height * width * PACKS_IMAGE_BYTES_PER_PIXEL);
        }
        else
        {
            int outWidth, outHeight;
            Packs_DecodeJpeg(file.handle, colourPixels, 0, &outWidth, &outHeight, 0);
            file.Seek(dataStart + field, 0);
            if (hasAlpha)
            {
                file.Read(&field, 1, sizeof(field));
                Packs_DecodeJpeg(file.handle, alphaPixels, 0, &outWidth, &outHeight, 0);
            }
        }
        file.Close();
        loaded = 1;
    }
    return loaded;
}
