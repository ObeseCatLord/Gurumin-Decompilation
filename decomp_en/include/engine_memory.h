// The engine's memory functions of the EN build (allocate, free, copy): pointers filled in at start-up.
// This is the one declaration of them in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_ENGINE_MEMORY_H
#define GURUMIN_EN_ENGINE_MEMORY_H

// Allocates size bytes; tag names the caller for the engine's leak report. EN 0x0218ee8c (JP DAT_11b0b938).
extern "C" void *(*g_engineAlloc)(unsigned int size, const char *tag);

// Frees a block returned by g_engineAlloc. EN 0x0218ee90 (JP DAT_11b1ba40).
extern "C" void (*g_engineFree)(void *block);

// Copies `size` bytes from `source` to `destination` (memcpy semantics). EN 0x0218ee88 (JP DAT_11b07530).
extern "C" void *(*g_engineCopy)(void *destination, const void *source, int size);

// Blocks the engine allocator hands out and has not got back yet (0 = free slot); its free function finds a block here.
const int ENGINE_BLOCK_SLOTS = 0x1000;
extern "C" void *g_engineBlocks[ENGINE_BLOCK_SLOTS];   // EN 0x0219f498
// Bytes of the caller tag kept for each block slot, and the tags (the `tag` of g_engineAlloc, cut to this size).
const int ENGINE_BLOCK_TAG_SIZE = 16;
extern "C" char g_engineBlockTags[ENGINE_BLOCK_SLOTS][ENGINE_BLOCK_TAG_SIZE];   // EN 0x0218f498

// Writes one line "<slot> <tag> <address>" to the text file `path` for every block still in use; nothing is written
// (no file made) when there are none. EN 0x77bb00 (engine_memory_report.cpp).
extern "C" void Engine_WriteBlockReport(const char *path);

// The engine's free function (the value start-up stores in g_engineFree): forgets `block` in g_engineBlocks when it is
// there and frees it. EN 0x77bc20 (engine_memory.cpp).
extern "C" void Engine_FreeBlock(void *block);

// Largest block the engine allocator hands out (a bigger request returns 0).
const unsigned int ENGINE_ALLOC_MAX_SIZE = 100000000;

// Text of the default tag, written by MemoryTracker_Reset. EN 0x0218f298.
const int ENGINE_DEFAULT_TAG_SIZE = 256;
extern "C" char g_engineDefaultTag[ENGINE_DEFAULT_TAG_SIZE];
// Second text buffer next to the default tag, emptied by MemoryTracker_Reset and never read. EN 0x0218f398.
extern "C" char g_engineSpareText[ENGINE_DEFAULT_TAG_SIZE];
// Four words after the function pointers, cleared by MemoryTracker_Reset and never read. EN 0x0218ee98.
extern "C" int g_engineSpareWords[4];
// A word cleared by MemoryTracker_Reset and never read. EN 0x0218eea8.
extern "C" int g_engineSpareCount;

// The engine's allocate function (the value start-up stores in g_engineAlloc): takes the first free slot of
// g_engineBlocks (the last slot when all are used), keeps the first 15 characters of `tag` for it and stores the block
// from malloc there. Returns the block, or 0 for a size of 0 or above ENGINE_ALLOC_MAX_SIZE. EN 0x77bcb0
// (engine_memory.cpp).
extern "C" void *Engine_AllocBlock(unsigned int size, const char *tag);

#endif
