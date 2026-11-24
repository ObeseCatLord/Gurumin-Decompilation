// The engine's memory functions of the EN build: pointers filled in at start-up.
// This is the one declaration of them in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_ENGINE_MEMORY_H
#define GURUMIN_EN_ENGINE_MEMORY_H

// Allocates size bytes; tag names the caller for the engine's leak report. EN 0x0218ee8c (JP DAT_11b0b938).
extern "C" void *(*g_engineAlloc)(int size, const char *tag);

// Frees a block returned by g_engineAlloc. EN 0x0218ee90 (JP DAT_11b1ba40).
extern "C" void (*g_engineFree)(void *block);

#endif
