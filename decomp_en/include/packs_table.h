// More entries of the Packs function table of the EN build (the table g_packsReader in small_b11.h belongs to) and the
// image size the preview reader reports. This is the one declaration of these in decomp_en: include this header,
// never redeclare them.
#ifndef GURUMIN_EN_PACKS_TABLE_H
#define GURUMIN_EN_PACKS_TABLE_H

// The compressor entry of the table: packs `length` bytes of `input` into `output` and returns the packed size.
// EN 0x02226900 (it points at Packs_Compress).
typedef int (*PacksCompressFunction)(const unsigned char *input, int *output, int length);
extern "C" PacksCompressFunction g_packsCompress;

// Width and height the image reader reports for the image it opened: they go in and out of the reader's calls. EN
// 0x02226924 and 0x02226928.
extern "C" int g_packsImageWidth;
extern "C" int g_packsImageHeight;

// Runs the image reader of the table with `command` on the scratch block (rows PACKS_READER_ROW_BYTES wide), letting it
// update g_packsImageWidth and g_packsImageHeight. Returns 1 when the reader succeeded, 0 when there is no scratch block
// or the reader failed (see packs_reader_run.cpp). EN 0x7a9a10.
extern "C" int Packs_RunImageReader(int command);

#endif
