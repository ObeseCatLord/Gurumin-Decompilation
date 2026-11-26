// Writer of the Packs "BMP" image container (EN build: Packs.dll is linked into the exe). Counterpart of
// Packs_LoadImageFile in packs_bmp.cpp.
#include "jpeg_lib.h"
#include "packs_save.h"

// Writes the file header, a JPEG stream of the colour image and (when present) one of the alpha image, patching the
// stream size in front of each stream once the stream is written, then appends both rasters uncompressed.
// Returns 1 on success, 0 when the file cannot be created. Called by the picture savers.
int Packs_SaveImageFile(const char *path, const unsigned char *colourPixels, const unsigned char *alphaPixels,
                        int width, int height)
{
    GameFile file;
    int saved;
    if (!file.Open(path, PACKS_SAVE_MODE))
    {
        saved = 0;
    }
    else
    {
        int magic = (alphaPixels != 0) + PACKS_MAGIC_COLOUR;
        file.Write(&magic, 1, sizeof(magic));
        file.Write(&width, 1, sizeof(width));
        file.Write(&height, 1, sizeof(height));
        int sizeSlot = file.Size();
        file.Write(&magic, 1, sizeof(magic));
        int streamLength = file.Size();
        Jpeg_WriteRgb(file.handle, colourPixels, width, height, PACKS_SAVE_JPEG_QUALITY, 0);
        int streamEnd = file.Size();
        streamLength = streamEnd - streamLength;
        file.Seek(sizeSlot, 0);
        file.Write(&streamLength, 1, sizeof(streamLength));
        file.Seek(streamEnd, 0);

        if (alphaPixels != 0)
        {
            sizeSlot = file.Size();
            file.Write(&magic, 1, sizeof(magic));
            streamLength = file.Size();
            Jpeg_WriteRgb(file.handle, alphaPixels, width, height, PACKS_SAVE_JPEG_QUALITY, 0);
            streamEnd = file.Size();
            streamLength = streamEnd - streamLength;
            file.Seek(sizeSlot, 0);
            file.Write(&streamLength, 1, sizeof(streamLength));
            file.Seek(streamEnd, 0);
        }
        file.Write(colourPixels, 1, width * height * PACKS_IMAGE_BYTES_PER_PIXEL);
        if (alphaPixels != 0)
            file.Write(alphaPixels, 1, width * height * PACKS_IMAGE_BYTES_PER_PIXEL);
        file.Close();
        saved = 1;
    }
    return saved;
}
