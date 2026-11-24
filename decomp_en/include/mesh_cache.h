// Shared texture files and model files (EN build): per file name a reference count and the name that is freed when the
// count reaches 0. This is the one declaration of these types and globals in decomp_en.
#ifndef GURUMIN_EN_MESH_CACHE_H
#define GURUMIN_EN_MESH_CACHE_H

// Entries of the texture file table.
const int TEXTURE_FILE_COUNT = 0x400;

// Bytes of a file name in either table.
const int FILE_NAME_SIZE = 0x20;

// A shared texture reference: the cache slot it holds.
struct TextureRef
{
    int slot;   // 0x00: index in the texture file table (negative = none)
};

extern "C" int g_textureFileRefCount[TEXTURE_FILE_COUNT];                 // EN 0x01636f68 (JP DAT_10faae88)
extern "C" char g_textureFileNames[TEXTURE_FILE_COUNT][FILE_NAME_SIZE];   // EN 0x0162ef68 (JP DAT_10ff65f8)

// Releases one reference to a shared texture file (freeing the name at 0). Returns the remaining count. EN 0x71cf00.
extern "C" int TextureFile_Release(const TextureRef *texture);

#include "slot_table.h"

// Entries of the model file table.
const int MODEL_FILE_COUNT = 0xc80;

extern "C" int g_modelFileRefCount[MODEL_FILE_COUNT];                     // EN 0x01653f68 (JP DAT_10feb3e0)
extern "C" char g_modelFileNames[MODEL_FILE_COUNT][FILE_NAME_SIZE];       // EN 0x0163af68 (JP DAT_10fada90)

// Releases one reference to the shared model file a mesh was loaded from (freeing the name at 0). Returns the
// remaining count. EN 0x71cf40.
extern "C" int MeshCache_Release(const SlotTable *mesh);

#endif
