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

// Bytes of video memory the engine counts for its textures (16 bit pixels with all mip levels). EN 0x0166d000 (JP DAT_1101d874).
extern "C" int g_textureMemoryBytes;

// The per-frame matrices of a node's animation are shared through three tables that follow the texture file names:
// the texture reference, the matrices and their count of each entry (an entry is a node's file name plus node name).
extern "C" TextureRef *g_motionCacheRef[TEXTURE_FILE_COUNT];      // EN 0x01637f68 (JP DAT_10fdf1d0)
struct Matrix4;
extern "C" Matrix4 *g_motionCacheMatrices[TEXTURE_FILE_COUNT];    // EN 0x01638f68 (JP DAT_1101c820)
extern "C" int g_motionCacheCount[TEXTURE_FILE_COUNT];            // EN 0x01639f68 (JP DAT_11017600)
// Looks up the cached motion of a node by its file name and node name. Returns its index, or -1 and the first free
// index in *freeSlot. EN 0x72c540 (JP 0x10169ab0).
extern "C" int MotionCache_Find(const char *file, const char *nodeName, int *freeSlot);
// Stores the file name and node name of motion cache entry `slot`. EN 0x746230 (JP 0x101699f0).
extern "C" int MotionCache_SetNames(int slot, const char *file, const char *nodeName);
// Adds a reference to a shared texture file and returns the new count. EN 0x70b690.
extern "C" int TextureFile_AddRef(const TextureRef *texture);
// Returns 0 when a free entry exists in the texture file name table, -1 when it is full. EN 0x70bbe0.
extern "C" int TextureNames_FindFree(void);

// Releases one reference to a shared texture file (freeing the name at 0). Returns the remaining count. EN 0x71cf00.
extern "C" int TextureFile_Release(const TextureRef *texture);

#include "slot_table.h"

struct VertexPack;
struct IndexList;

// Entries of the model file table.
const int MODEL_FILE_COUNT = 0xc80;

extern "C" int g_modelFileRefCount[MODEL_FILE_COUNT];                     // EN 0x01653f68 (JP DAT_10feb3e0)
extern "C" char g_modelFileNames[MODEL_FILE_COUNT][FILE_NAME_SIZE];       // EN 0x0163af68 (JP DAT_10fada90)

// Per model file: the vertex packs, index lists and pack count its nodes share (set while loading a model, cleared
// by Node::ReleaseUniqueMeshData when the file has a single user).
extern "C" int g_modelFilePackCount[MODEL_FILE_COUNT];                    // EN 0x01660768 (JP DAT_10fee5e0): packs (-1 = none)
extern "C" VertexPack *g_modelFilePacks[MODEL_FILE_COUNT];                // EN 0x0165a368 (JP DAT_10ff2ff8)
extern "C" IndexList *g_modelFileIndexLists[MODEL_FILE_COUNT];            // EN 0x0165d568 (JP DAT_10fe01d8)
extern "C" SlotTable *g_modelFileMeshes[MODEL_FILE_COUNT];                // EN 0x01657168 (JP DAT_10fa7478): the shared mesh of each model file

// Bytes of the scratch buffer that joins a folder and a name; the joined name is cut to FILE_NAME_SIZE - 1 characters.
const int MODEL_NAME_SCRATCH_SIZE = 256;

// Returns the first free entry of the model file name table, or -1 when it is full. EN 0x70bc00 (JP 0x10169760).
extern "C" int ModelNames_FindFree(void);
// Stores folder + name (cut to 31 characters) as entry `index` of the model file name table. Returns 0.
// EN 0x7462c0 (JP 0x10169780).
extern "C" int ModelNames_SetJoined(int index, const char *folder, const char *name);
// Looks up folder + name (cut to 31 characters) in the model file name table. Returns its index when it is there.
// Otherwise the name is stored in the first free entry (its reference count reset), that index goes to *freeSlot and
// -1 is returned. A full table stops the game on purpose. EN 0x72c650 (JP 0x10169840).
extern "C" int ModelNames_Find(const char *folder, const char *name, int *freeSlot);
// Empties the whole model file table: every reference count and name. Returns 0. EN 0x734b20 (JP 0x10169720).
extern "C" int ModelNames_ClearAll(void);
// Adds a reference to the shared model file of `mesh` and returns the new count. EN 0x70b6b0 (JP 0x10169970).
extern "C" int MeshCache_AddRef(const SlotTable *mesh);

// Releases one reference to the shared model file a mesh was loaded from (freeing the name at 0). Returns the
// remaining count. EN 0x71cf40.
extern "C" int MeshCache_Release(const SlotTable *mesh);

#endif
