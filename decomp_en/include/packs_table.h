// More entries of the Packs function table of the EN build (the table g_packsReader in small_b11.h belongs to) and the
// image size the preview reader reports. This is the one declaration of these in decomp_en: include this header,
// never redeclare them.
#ifndef GURUMIN_EN_PACKS_TABLE_H
#define GURUMIN_EN_PACKS_TABLE_H

// The compressor entry of the table: packs `length` bytes of `input` into `output` and returns the packed size.
// EN 0x02226900 (it points at Packs_Compress).
typedef int (*PacksCompressFunction)(const unsigned char *input, int *output, int length);
extern "C" PacksCompressFunction g_packsCompress;

// The decompressor entry of the table: unpacks `length` packed bytes of `input` into `output`. EN 0x02226904.
typedef int (*PacksUncompressFunction)(const unsigned char *input, unsigned char *output, int length);
extern "C" PacksUncompressFunction g_packsUncompress;

// The picture decoder entry of the table: decodes `length` bytes of packed picture data into `output` (3 bytes per
// pixel), reporting the width and height through the last two arguments. EN 0x02226908.
typedef int (*PacksPictureDecodeFunction)(const unsigned char *input, int length, unsigned char *output, int *width, int *height);
extern "C" PacksPictureDecodeFunction g_packsPictureDecode;

// The picture encoder entry of the table: packs `width` x `height` pixels into `output` and stores the packed size in
// `packedSize`; `quality` is always PACKS_PICTURE_QUALITY. EN 0x0222690c.
typedef int (*PacksPictureEncodeFunction)(const unsigned char *pixels, int width, int height, int quality, void *output, int *packedSize);
extern "C" PacksPictureEncodeFunction g_packsPictureEncode;
const int PACKS_PICTURE_QUALITY = 0x100;   // fourth argument of the picture encoder

// Width and height the image reader reports for the image it opened: they go in and out of the reader's calls. EN
// 0x02226924 and 0x02226928.
extern "C" int g_packsImageWidth;
extern "C" int g_packsImageHeight;

// Set to 0 by the image preview reader whenever it reopens; no reader of it found. EN 0x0222692c.
extern "C" int g_packsImageState;

// Size of the scratch block the preview reader allocates (a 640x480 picture at 3 bytes per pixel, rounded up).
const int PACKS_SCRATCH_BLOCK_BYTES = 0xe1000;
// Frees the old scratch block, allocates a new one, closes the image file and runs the reader with `command`.
// Returns 1 on success, 0 when there is no block or the reader fails. EN 0x7a9a70 (packs_reader_open.cpp).
extern "C" int Packs_OpenImageReader(int command);

// Runs the image reader of the table with `command` on the scratch block (rows PACKS_READER_ROW_BYTES wide), letting it
// update g_packsImageWidth and g_packsImageHeight. Returns 1 when the reader succeeded, 0 when there is no scratch block
// or the reader failed (see packs_reader_run.cpp). EN 0x7a9a10.
extern "C" int Packs_RunImageReader(int command);

#endif
