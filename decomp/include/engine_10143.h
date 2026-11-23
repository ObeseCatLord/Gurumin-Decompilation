// SlotTable, ColourVertex: declared here once (first used by src/engine_10143.cpp).
#ifndef GURUMIN_ENGINE_10143_H
#define GURUMIN_ENGINE_10143_H

struct SlotTable
{
    int m_0000;
    int pad_0004[2];
    int m_000c;
    int *m_0010;
    int *m_0014;
    int m_0018;
    int m_001c;
    int m_0020;
    int m_0024;
    int m_0028;
    int m_002c;
    int m_0030;
    int m_0034;
    int m_0038;
    int m_003c;
    int m_0040;
    int m_0044;
    int m_0048;

    void M_10145a10();
    int M_10146d90(int first, int last);
    int M_10147690(int key, int value);
    int M_101476e0(int index, int value);
};
// Pre-transformed vertex with colours and one texture coordinate (FVF 0x1c4), 0x20 bytes.
struct ColourVertex
{
    float x;
    float y;
    float z;
    float rhw;
    unsigned int diffuse;
    unsigned int specular;
    float u;
    float v;
};

#endif
