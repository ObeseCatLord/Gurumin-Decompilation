// Rect, PanelVertex, ShaderVertex: declared here once (first used by src/texture_bind.cpp).
#ifndef GURUMIN_TEXTURE_BIND_H
#define GURUMIN_TEXTURE_BIND_H

// A RECT (left, top, right, bottom).
struct Rect
{
    long left;
    long top;
    long right;
    long bottom;
};
// A pre-transformed vertex with every field (FVF 0x1c4), 0x20 bytes.
struct PanelVertex
{
    float x;
    float y;
    float z;
    float rhw;
    unsigned int colour;
    unsigned int specular;
    float u;
    float v;
};
// A pre-transformed vertex with four texture coordinate pairs (FVF 0x4c4), 0x38 bytes.
struct ShaderVertex
{
    float x;
    float y;
    float z;
    float rhw;
    unsigned int colour;
    unsigned int specular;
    float u0, v0;
    float u1, v1;
    float u2, v2;
    float u3, v3;
};

#endif
