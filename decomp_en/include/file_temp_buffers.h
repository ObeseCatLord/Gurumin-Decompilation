// The two 2 MB scratch buffers the engine's file loader works in (EN build). This is the one declaration of them in
// decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_FILE_TEMP_BUFFERS_H
#define GURUMIN_EN_FILE_TEMP_BUFFERS_H

// Bytes of each scratch buffer.
const int FILE_TEMP_BUFFER_SIZE = 0x200000;

extern "C" void *g_fileTempBuffer1;   // EN 0x022010f8: first scratch buffer, tagged "FILETMP1" (0 = not allocated)
extern "C" void *g_fileTempBuffer2;   // EN 0x022010fc: second scratch buffer, tagged "FILETMP2" (0 = not allocated)

// Allocates whichever scratch buffer does not exist yet (see file_temp_buffers.cpp). Returns 0. EN 0x7a8a00.
// Called by Render_CreateDevice (EN 0x7a13e0).
extern "C" int FileTempBuffers_Allocate(void);
// Frees both scratch buffers. Returns 0. EN 0x7a8a50. Called by the shutdown at EN 0x7a3ad0.
extern "C" int FileTempBuffers_Free(void);

#endif
