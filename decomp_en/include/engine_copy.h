// The engine's memory copy function of the EN build: a pointer filled in at start-up (next to g_engineAlloc and
// g_engineFree in engine_memory.h). This is the one declaration of it in decomp_en: include this header.
#ifndef GURUMIN_EN_ENGINE_COPY_H
#define GURUMIN_EN_ENGINE_COPY_H

// Copies `size` bytes from `source` to `destination` (memcpy semantics). EN 0x0218ee88 (JP DAT_11b07530).
extern "C" void *(*g_engineCopy)(void *destination, const void *source, int size);

#endif
