// Mesh, MeshHolder, MeshOwner: declared here once (first used by src/app_0008.cpp).
#ifndef GURUMIN_APP_0008_H
#define GURUMIN_APP_0008_H

struct Mesh
{
    void M_1019e9a0();
};
struct MeshHolder
{
    Mesh *mesh;
    char pad_0004[0x28];
    int m_002c;
    int m_0030;
    char pad_0034[0xad4];
};
struct MeshOwner
{
    int pad_0000;
    Mesh *mesh;
    char pad_0008[0x28];
    int m_0030;
    int m_0034;
    int M_1000be10();
};

#endif
