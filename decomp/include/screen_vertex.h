#ifndef GURUMIN_SCREEN_VERTEX_H
#define GURUMIN_SCREEN_VERTEX_H

// A transformed screen vertex (0x20 bytes, FVF 0x1c4): TextureObject::DrawStrip and TriangleBatch take these;
// EffectManager::DrawLight fills the trail strip DAT_110187c0 with them.
struct ScreenVertex
{
    float x;                                         // 0x00: screen x in the 640x480 space
    float y;                                         // 0x04: screen y
    float z;                                         // 0x08: depth (0.9 to 1.0 is visible)
    float rhw;                                       // 0x0c: 1 (already transformed)
    union                                            // 0x10: diffuse colour, ARGB
    {
        unsigned int diffuse;
        struct
        {
            unsigned char blue;
            unsigned char green;
            unsigned char red;
            unsigned char alpha;                     // set to 0 to hide a vertex that is off screen
        };
    };
    unsigned int specular;                           // 0x14: second colour (0x00RRGGBB)
    float u;                                         // 0x18
    float v;                                         // 0x1c
};

#endif
