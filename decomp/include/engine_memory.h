// The engine's memory functions: pointers filled in at start-up (allocate, free, copy).
// Every file that allocates through the engine includes this header instead of declaring the pointers itself.
#ifndef GURUMIN_ENGINE_MEMORY_H
#define GURUMIN_ENGINE_MEMORY_H

// Allocates size bytes; tag names the caller for the engine's leak report.
extern "C" void *(*DAT_11b0b938)(int size, const char *tag);
// Frees a block returned by DAT_11b0b938.
extern "C" void (*DAT_11b1ba40)(void *block);
// Copies size bytes from src to dst (the engine's memcpy); returns dst.
extern "C" void *(*DAT_11b07530)(void *dst, const void *src, int size);

#endif
