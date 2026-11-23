// FontSet: declared here once (first used by src/input_font.cpp).
#ifndef GURUMIN_FONT_SET_H
#define GURUMIN_FONT_SET_H

#include "dinput_min.h"

struct FontSet
{
    HFONT fonts[3];
    void *glyphs[0x20];
    char pad_008c[0x2300];
    void *buffer;
    void *buffer2;
    int Release();
};

#endif
