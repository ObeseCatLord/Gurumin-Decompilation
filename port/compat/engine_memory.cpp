// Storage for the engine's memory function pointers declared in decomp/include/engine_memory.h. The game fills them
// at start-up with its own allocator; the port points them at the C runtime.
#include <stdlib.h>
#include <string.h>

#include "engine_memory.h"

namespace
{
// Allocates `size` bytes; `tag` names the caller in the engine's leak report and is ignored here.
void *PortAllocate(int size, const char *tag)
{
    (void)tag;
    return malloc(size > 0 ? (size_t)size : 1);
}

void PortFree(void *block)
{
    free(block);
}

void *PortCopy(void *dst, const void *src, int size)
{
    return memcpy(dst, src, size > 0 ? (size_t)size : 0);
}
}

// C linkage comes from the declarations in engine_memory.h.
void *(*DAT_11b0b938)(int size, const char *tag) = PortAllocate;
void (*DAT_11b1ba40)(void *block) = PortFree;
void *(*DAT_11b07530)(void *dst, const void *src, int size) = PortCopy;
