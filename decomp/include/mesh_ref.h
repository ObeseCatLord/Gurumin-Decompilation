// MeshRef: declared here once (first used by src/mesh_cache.cpp).
#ifndef GURUMIN_MESH_REF_H
#define GURUMIN_MESH_REF_H

struct MeshRef
{
    char pad_0000[0x4c];
    int index;
};

#endif
