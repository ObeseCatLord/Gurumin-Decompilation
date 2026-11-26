// Scratch buffers of the file loader (EN build, EN-only: no JP twin found).
#include "file_temp_buffers.h"
#include "engine_memory.h"

// FileTempBuffers_Allocate: allocates each of the two FILE_TEMP_BUFFER_SIZE scratch buffers that is not allocated yet.
// Returns 0. Called by Render_CreateDevice (EN 0x7a13e0).
extern "C" int FileTempBuffers_Allocate(void)
{
    if (g_fileTempBuffer1 == 0)
        g_fileTempBuffer1 = g_engineAlloc(FILE_TEMP_BUFFER_SIZE, "FILETMP1");
    if (g_fileTempBuffer2 == 0)
        g_fileTempBuffer2 = g_engineAlloc(FILE_TEMP_BUFFER_SIZE, "FILETMP2");
    return 0;
}

// FileTempBuffers_Free: frees both scratch buffers (those that exist) and clears the pointers. Returns 0. Called by
// the shutdown at EN 0x7a3ad0.
extern "C" int FileTempBuffers_Free(void)
{
    if (g_fileTempBuffer1 != 0)
        g_engineFree(g_fileTempBuffer1);
    if (g_fileTempBuffer2 != 0)
        g_engineFree(g_fileTempBuffer2);
    g_fileTempBuffer1 = 0;
    g_fileTempBuffer2 = 0;
    return 0;
}
