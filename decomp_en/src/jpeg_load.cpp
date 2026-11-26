// JPEG readers of the Packs code (EN build; in JP they lived in Packs.dll). No JP twin mapped: ported from the EN bytes.
#include <stdio.h>

#include "jpeg_lib.h"

// Bytes per pixel of the rows libjpeg decodes to.
const int JPEG_LOAD_BYTES_PER_PIXEL = 3;
// Rows libjpeg is asked for at a time.
const int JPEG_LOAD_ROWS_PER_CALL = 1;

// Jpeg_ReadRgb: see the declaration in jpeg_lib.h. Decodes row after row into `buffer`.
extern "C" int Jpeg_ReadRgb(FILE *file, unsigned char *buffer, int stride, int *width, int *height)
{
    jpeg_error_mgr errorManager;
    jpeg_decompress_struct cinfo;
    if (file == 0)
        return 0;
    if (buffer == 0)
        return 0;
    cinfo.err = jpeg_std_error(&errorManager);
    jpeg_CreateDecompress(&cinfo, JPEG_LIB_VERSION, sizeof(cinfo));
    jpeg_stdio_src(&cinfo, file);
    jpeg_read_header(&cinfo, 1);
    jpeg_start_decompress(&cinfo);
    *width = cinfo.output_width;
    *height = cinfo.output_height;
    if (stride == 0)
        stride = cinfo.output_width * JPEG_LOAD_BYTES_PER_PIXEL;
    while (cinfo.output_scanline < cinfo.output_height)
    {
        JSAMPROW row = buffer;
        jpeg_read_scanlines(&cinfo, &row, JPEG_LOAD_ROWS_PER_CALL);
        buffer += stride;
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return 1;
}

// Jpeg_LoadRgb: see the declaration in jpeg_lib.h. Opens the file and decodes it row after row into `buffer`.
extern "C" int Jpeg_LoadRgb(const char *path, unsigned char *buffer, int *width, int *height)
{
    jpeg_error_mgr errorManager;
    jpeg_decompress_struct cinfo;
    FILE *file = fopen(path, "rb");
    if (file == 0)
        return 0;
    if (buffer == 0)
        return 0;
    cinfo.err = jpeg_std_error(&errorManager);
    jpeg_CreateDecompress(&cinfo, JPEG_LIB_VERSION, sizeof(cinfo));
    jpeg_stdio_src(&cinfo, file);
    jpeg_read_header(&cinfo, 1);
    jpeg_start_decompress(&cinfo);
    *width = cinfo.output_width;
    *height = cinfo.output_height;
    int stride = cinfo.output_width * JPEG_LOAD_BYTES_PER_PIXEL;
    while (cinfo.output_scanline < cinfo.output_height)
    {
        JSAMPROW row = buffer;
        jpeg_read_scanlines(&cinfo, &row, JPEG_LOAD_ROWS_PER_CALL);
        buffer += stride;
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return 1;
}
