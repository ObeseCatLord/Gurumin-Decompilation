// Writer of the Packs "BMP" image container (EN build; the loader is in packs_bmp.cpp). Declared once here.
#ifndef GURUMIN_EN_PACKS_SAVE_H
#define GURUMIN_EN_PACKS_SAVE_H

#include "packs_codec.h"

// File mode of the writer.
#define PACKS_SAVE_MODE "wb"
// Quality handed to the JPEG writer for each image (above 100: libjpeg clamps it to the best quality).
const int PACKS_SAVE_JPEG_QUALITY = 0x80;

// Writes a Packs image file: the magic number (colour image only / colour image and alpha image), the image size, the
// size of each JPEG stream followed by the stream itself, then both rasters uncompressed. `alphaPixels` may be 0.
// Returns 1, or 0 when the file cannot be created. EN 0x7bdee0 (JP twin: Packs.dll 1000_xxxx unmatched).
int Packs_SaveImageFile(const char *path, const unsigned char *colourPixels, const unsigned char *alphaPixels,
                        int width, int height);

#endif
