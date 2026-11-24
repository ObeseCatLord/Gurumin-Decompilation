#pragma once
// Helpers of the BMP texture loader (texture_load_bmp.cpp).

// 0x101612b0 (Rle_Decode): unpacks `packedSize` bytes at `packed` into `size` bytes at `out`; 0 on failure.
extern "C" int FUN_101612b0(unsigned char *out, int size, const unsigned char *packed, int packedSize);
// 0x10161200 (Bmp_FlipVertical): swaps the rows of a bottom-up 24-bit image in place.
extern "C" int FUN_10161200(unsigned char *pixels, int width, int height);
// 0x101615d0 (ItmDescramble): reverses the .itm scrambling in place; 1 when the checksums match.
extern "C" int FUN_101615d0(unsigned char *data, int size, char *name, int sum, unsigned int check);
// Image decoder of the host for the formats the loader itself does not read (psd, jpg, tga, png), 0 = none: reads
// the file `path` and returns its size, its RGB pixels and, when `alpha` is not 0, its alpha pixels; 0 on failure.
extern "C" int (*DAT_1101d84c)(const char *path, int *width, int *height, unsigned char **rgb, unsigned char **alpha);
// Non-zero = pixels of the colour key (magenta, or blue) become transparent: an alpha image is generated for them.
extern "C" int DAT_1101d848;
