#pragma once
// Globals and helpers of the texture file loader (texture_load_iti.cpp).

// 1 = textures are loaded at half the file size (the smaller texture quality setting).
extern "C" int DAT_1101d840;
// Non-zero = the image loader below is not asked to generate the alpha channel itself.
extern "C" int DAT_1101d844;
// Image file decoder of the engine library (JPEG or similar): reads the file `path` into the RGB buffer `rgb` and,
// when `alpha` is not 0, the alpha buffer; `flag` 1 = the decoder only reads what it needs for a generated alpha.
extern "C" int (*DAT_11b1bb7c)(const char *path, void *rgb, void *alpha, int flag);
// 0x10161770: halves a `width` x `height` RGB image in place; returns 0 for a null image, else 1.
extern "C" int FUN_10161770(void *rgb, int width, int height);
// 0x1013edb0: error report (empty in the release build).
extern "C" int FUN_1013edb0(const char *format, ...);
