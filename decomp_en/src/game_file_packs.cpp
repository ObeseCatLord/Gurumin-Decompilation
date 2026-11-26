// GameFile blocks packed with the Packs functions (EN build, EN-only: no JP twin found): the 4-byte packed size and the
// packed data, through the two scratch buffers of the file loader.
#include <string.h>

#include "file_temp_buffers.h"
#include "game_file.h"
#include "packs_table.h"

// Bytes of one decoded picture pixel (red, green, blue).
const int PICTURE_PIXEL_BYTES = 3;

// GameFile::ReadCompressed: reads the packed size and then the packed data into the first scratch buffer, unpacks it
// into the second and copies `count` * `size` bytes of the result to `destination` (nothing is copied when it is 0).
// Returns what the read of the packed data returned, or 0 when no file is open. Called by the loaders.
size_t GameFile::ReadCompressed(void *destination, int count, int size)
{
    if (handle != 0)
    {
        int packedSize;
        Read(&packedSize, 1, sizeof(packedSize));
        size_t bytesRead = Read(g_fileTempBuffer1, 1, packedSize);
        g_packsUncompress((const unsigned char *)g_fileTempBuffer1, (unsigned char *)g_fileTempBuffer2, packedSize);
        if (destination != 0)
            memcpy(destination, g_fileTempBuffer2, count * size);
        return bytesRead;
    }
    return 0;
}

// GameFile::ReadPicture: reads a packed picture block, decodes it into the second scratch buffer and copies
// `width` * `height` pixels of PICTURE_PIXEL_BYTES bytes to `destination`. Returns what the read of the packed data
// returned, or 0 when no file is open. Called by the loaders.
size_t GameFile::ReadPicture(void *destination, int width, int height)
{
    if (handle != 0)
    {
        int packedSize;
        int decodedHeight;
        int decodedWidth;
        Read(&packedSize, 1, sizeof(packedSize));
        size_t bytesRead = Read(g_fileTempBuffer1, 1, packedSize);
        g_packsPictureDecode((const unsigned char *)g_fileTempBuffer1, packedSize, (unsigned char *)g_fileTempBuffer2, &decodedWidth, &decodedHeight);
        memcpy(destination, g_fileTempBuffer2, width * height * PICTURE_PIXEL_BYTES);
        return bytesRead;
    }
    return 0;
}

// GameFile::WritePicture: packs the picture with the picture encoder into the first scratch buffer, then writes the
// packed size as a 4-byte value followed by the packed bytes. Returns what the last write returned, or 0 when no file
// is open. Called by the picture savers.
size_t GameFile::WritePicture(const unsigned char *pixels, int width, int height)
{
    if (handle != 0)
    {
        int packedSize;
        g_packsPictureEncode(pixels, width, height, PACKS_PICTURE_QUALITY, g_fileTempBuffer1, &packedSize);
        if (handle != 0)
            fwrite(&packedSize, 1, sizeof(packedSize), handle);
        if (handle != 0)
            return fwrite(g_fileTempBuffer1, 1, packedSize, handle);
    }
    return 0;
}
