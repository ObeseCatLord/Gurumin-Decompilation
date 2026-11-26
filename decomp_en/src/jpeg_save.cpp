// JPEG writer of the Packs code (EN build; in JP it lived in Packs.dll). No JP twin mapped: ported from the EN bytes.
#include <stdio.h>
#include <stdlib.h>

#include "jpeg_lib.h"

// Bytes per pixel of the image the writer reads and of the scanline it hands to libjpeg.
const int JPEG_SAVE_BYTES_PER_PIXEL = 3;
// Rows of the source image are padded to this many bytes.
const int JPEG_SAVE_ROW_ALIGNMENT = 4;

// Jpeg_SaveBgr: see the declaration in jpeg_lib.h. Converts each BGR row to RGB and compresses it.
extern "C" int Jpeg_SaveBgr(const char *path, const unsigned char *pixels, int width, int height, int quality, int progressive)
{
    jpeg_error_mgr errorManager;
    jpeg_compress_struct cinfo;
    FILE *file = fopen(path, "wb");
    if (file == 0)
        return 0;
    if (pixels == 0)
        return 0;
    cinfo.err = jpeg_std_error(&errorManager);
    jpeg_CreateCompress(&cinfo, JPEG_LIB_VERSION, sizeof(cinfo));
    jpeg_stdio_dest(&cinfo, file);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = JPEG_SAVE_BYTES_PER_PIXEL;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, 1);
    if (progressive != 0)
        jpeg_simple_progression(&cinfo);
    jpeg_start_compress(&cinfo, 1);
    JSAMPROW row = (JSAMPROW)malloc(width * JPEG_SAVE_BYTES_PER_PIXEL);
    int pitch = (width * JPEG_SAVE_BYTES_PER_PIXEL + JPEG_SAVE_ROW_ALIGNMENT - 1) / JPEG_SAVE_ROW_ALIGNMENT * JPEG_SAVE_ROW_ALIGNMENT;
    while (cinfo.next_scanline < cinfo.image_height)
    {
        JSAMPROW out = row;
        const unsigned char *in = pixels;
        for (int remaining = width; remaining > 0; remaining--)
        {
            out[0] = in[2];
            out[1] = in[1];
            out[2] = in[0];
            out += JPEG_SAVE_BYTES_PER_PIXEL;
            in += JPEG_SAVE_BYTES_PER_PIXEL;
        }
        pixels += pitch;
        JSAMPROW scanline = row;
        jpeg_write_scanlines(&cinfo, &scanline, 1);
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    free(row);
    fclose(file);
    return 1;
}
