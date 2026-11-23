// CharMatrixView: declared here once (first used by src/char_status.cpp).
#ifndef GURUMIN_CHAR_MATRIX_VIEW_H
#define GURUMIN_CHAR_MATRIX_VIEW_H

#include "matrix4.h"

// Character record viewed from its world matrix (0x10db6ee8 + 0x2328), 0x249c bytes.
struct CharMatrixView
{
    Matrix4 matrix;
    char pad_0040[0x249c - 0x40];
};

#endif
