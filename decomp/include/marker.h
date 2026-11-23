// Marker: declared here once (first used by src/char_retire.cpp).
#ifndef GURUMIN_MARKER_H
#define GURUMIN_MARKER_H

// 16-byte per-character marker records at 0x10867e18 (only the y value is used here).
struct Marker
{
    float y;
    char pad_0004[0xc];
};

#endif
