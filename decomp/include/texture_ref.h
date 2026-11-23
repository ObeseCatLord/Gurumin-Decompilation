// TextureRef: a reference-counted entry of the shared mesh/texture cache (mesh_cache.cpp). A node's motion points to
// one (Node::m_028c); FUN_10169be0 adds a reference, FUN_10169c00 releases one.
#ifndef GURUMIN_TEXTURE_REF_H
#define GURUMIN_TEXTURE_REF_H

struct TextureRef
{
    int slot;                    // the cache slot this reference holds
};

#endif
