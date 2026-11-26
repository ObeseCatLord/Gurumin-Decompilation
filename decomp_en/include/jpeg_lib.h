// The parts of IJG libjpeg 6b (compiled into the EN exe at 0x6f15b0-0x709c9f) that game code uses. This is the one
// declaration of these types in decomp_en: include this header, never redeclare them. Only the compression side that
// the screenshot writer needs is declared; unused fields are padding.
#ifndef GURUMIN_EN_JPEG_LIB_H
#define GURUMIN_EN_JPEG_LIB_H

#include <stdio.h>

struct jpeg_common_struct;

// libjpeg's error manager (0x84 bytes).
struct jpeg_error_mgr
{
    void (*error_exit)(jpeg_common_struct *cinfo);                          // 0x00: stops the program (the default prints and exits)
    void (*emit_message)(jpeg_common_struct *cinfo, int level);             // 0x04: emits a warning or trace message
    void (*output_message)(jpeg_common_struct *cinfo);                      // 0x08: prints the last message
    void (*format_message)(jpeg_common_struct *cinfo, char *buffer);        // 0x0c: formats the last message
    void (*reset_error_mgr)(jpeg_common_struct *cinfo);                     // 0x10: clears the error state
    int msg_code;                                                           // 0x14: code of the last message
    char msg_parm[80];                                                      // 0x18: parameters of the last message
    int trace_level;                                                        // 0x68: how much tracing to do
    long num_warnings;                                                      // 0x6c: warnings so far
    const char *const *jpeg_message_table;                                  // 0x70: library message strings
    int last_jpeg_message;                                                  // 0x74: highest library message code
    const char *const *addon_message_table;                                 // 0x78: application message strings
    int first_addon_message;                                                // 0x7c: first application message code
    int last_addon_message;                                                 // 0x80: last application message code
};

// Colour space and library version values the writer passes.
const int JCS_RGB = 2;                    // jpeg_compress_struct::in_color_space: red, green, blue
const int JPEG_LIB_VERSION = 62;          // version jpeg_CreateCompress checks (libjpeg 6b)

// libjpeg's compression object (0x168 bytes).
struct jpeg_compress_struct
{
    jpeg_error_mgr *err;                  // 0x00: error manager
    char m_0004[0x1c - 0x04];             // 0x04: not used by game code (memory manager, progress, state, destination)
    unsigned int image_width;             // 0x1c: width of the image in pixels
    unsigned int image_height;            // 0x20: height of the image in pixels
    int input_components;                 // 0x24: colour components per pixel of the input
    int in_color_space;                   // 0x28: colour space of the input (JCS_RGB)
    char m_002c[0xd0 - 0x2c];             // 0x2c: not used by game code
    unsigned int next_scanline;           // 0xd0: next row to write
    char m_00d4[0x168 - 0xd4];            // 0xd4: not used by game code
};

// libjpeg's decompression object (0x1b0 bytes).
struct jpeg_decompress_struct
{
    jpeg_error_mgr *err;                  // 0x00: error manager
    char m_0004[0x5c - 0x04];             // 0x04: not used by game code
    unsigned int output_width;            // 0x5c: width of the decoded image in pixels
    unsigned int output_height;           // 0x60: height of the decoded image in pixels
    char m_0064[0x78 - 0x64];             // 0x64: not used by game code
    unsigned int output_scanline;         // 0x78: next row to read
    char m_007c[0x1b0 - 0x7c];            // 0x7c: not used by game code
};

typedef char check_jpeg_decompress_struct[sizeof(jpeg_decompress_struct) == 0x1b0 ? 1 : -1];
typedef char check_jpeg_compress_struct[sizeof(jpeg_compress_struct) == 0x168 ? 1 : -1];
typedef char check_jpeg_error_mgr[sizeof(jpeg_error_mgr) == 0x84 ? 1 : -1];

typedef unsigned char JSAMPLE;
typedef JSAMPLE *JSAMPROW;

extern "C" jpeg_error_mgr *jpeg_std_error(jpeg_error_mgr *err);                                    // EN 0x702040
extern "C" void jpeg_CreateCompress(jpeg_compress_struct *cinfo, int version, unsigned int size);  // EN 0x6f15b0
extern "C" void jpeg_stdio_dest(jpeg_compress_struct *cinfo, FILE *outfile);                       // EN 0x6fa7e0
extern "C" void jpeg_set_defaults(jpeg_compress_struct *cinfo);                                    // EN 0x6f7390
extern "C" void jpeg_set_quality(jpeg_compress_struct *cinfo, int quality, int forceBaseline);     // EN 0x6f74e0
extern "C" void jpeg_simple_progression(jpeg_compress_struct *cinfo);                              // EN 0x6f7550
extern "C" void jpeg_start_compress(jpeg_compress_struct *cinfo, int writeAllTables);              // EN 0x6f1a20
extern "C" unsigned int jpeg_write_scanlines(jpeg_compress_struct *cinfo, JSAMPROW *scanlines, unsigned int count);   // EN 0x6f1b70
extern "C" void jpeg_finish_compress(jpeg_compress_struct *cinfo);                                 // EN 0x6f16f0
extern "C" void jpeg_destroy_compress(jpeg_compress_struct *cinfo);                                // EN 0x6f16e0
extern "C" void jpeg_CreateDecompress(jpeg_decompress_struct *cinfo, int version, unsigned int size);   // EN 0x6f9f30
extern "C" void jpeg_stdio_src(jpeg_decompress_struct *cinfo, FILE *infile);                       // EN 0x6fa920
extern "C" int jpeg_read_header(jpeg_decompress_struct *cinfo, int requireImage);                  // EN 0x6fa2b0
extern "C" int jpeg_start_decompress(jpeg_decompress_struct *cinfo);                               // EN 0x6fa530
extern "C" unsigned int jpeg_read_scanlines(jpeg_decompress_struct *cinfo, JSAMPROW *scanlines, unsigned int count);   // EN 0x6fa490
extern "C" int jpeg_finish_decompress(jpeg_decompress_struct *cinfo);                              // EN 0x6fa160
extern "C" void jpeg_destroy_decompress(jpeg_decompress_struct *cinfo);                            // EN 0x6fa150

// Writes the bottom-up BGR image `pixels` (`width` x `height`, rows padded to four bytes) to the file `path` as a
// JPEG of `quality` (0-100), progressive when `progressive` is set. Returns 1, or 0 when the file cannot be created
// or there are no pixels. EN 0x7be4f0. Called by nothing in the exe.
extern "C" int Jpeg_SaveBgr(const char *path, const unsigned char *pixels, int width, int height, int quality, int progressive);

// Writes the RGB image `pixels` (`width` x `height`, rows padded to four bytes, no channel swap) to the open file
// `file` as a JPEG of `quality`, progressive when `progressive` is set. Returns 1, or 0 when `file` or `pixels` is 0.
// EN 0x7be690. Called by Packs_SaveImageFile.
extern "C" int Jpeg_WriteRgb(FILE *file, const unsigned char *pixels, int width, int height, int quality, int progressive);

// Decodes the JPEG in the open file `file` into `buffer` as RGB rows `stride` bytes apart (0 = width * 3) and stores its
// size in `*width` and `*height`. Returns 1, or 0 when `file` or `buffer` is 0. EN 0x7be1c0 (jpeg_load.cpp).
extern "C" int Jpeg_ReadRgb(FILE *file, unsigned char *buffer, int stride, int *width, int *height);

// Opens the JPEG file `path` and decodes it into `buffer` as tightly packed RGB rows, storing its size in `*width` and
// `*height`. Returns 1, or 0 when the file cannot be opened or `buffer` is 0. The file is not closed. EN 0x7be0c0
// (jpeg_load.cpp).
extern "C" int Jpeg_LoadRgb(const char *path, unsigned char *buffer, int *width, int *height);

#endif
