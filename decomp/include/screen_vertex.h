#ifndef GURUMIN_SCREEN_VERTEX_H
#define GURUMIN_SCREEN_VERTEX_H

// A screen vertex (0x20 bytes): position, then colour and texture coordinates.
struct ScreenVertex
{
    float x;
    float y;
    float rest[6];            // 0x08: z, rhw, colour, texture coordinates (FVF 0x1c4)
};

#endif
