// MMV movie player and the texture name registry (EN build). Ported from the matched JP source
// decomp/src/mmv_texreg.cpp.
#include <string.h>

#include "engine_memory.h"
#include "mmv_player.h"
#include "render.h"

// Decodes the next frame (looping or one-shot variant depending on loop). Called once per frame while a movie plays.
void MmvPlayer::DecodeNextFrame()
{
    if (loop != 0)
        ReadFrame();
    else
        ReadFrameOnce();
}

// Closes the movie file and frees its buffers.
void MmvPlayer::Close()
{
    if (playing)
    {
        if (isOpen && m_003c != 0)
            file.Close();
        isOpen = 0;
        if (memory != 0)
            g_engineFree(memory);
        if (frame != 0 && ownsFrame)
            g_engineFree(frame);
        memory = 0;
        frame = 0;
        playing = 0;
    }
}

// Render_InitDevice: creates the Direct3D device and empties both texture name tables (all slots free). Returns the
// device creation result. Called at start-up.
extern "C" int Render_InitDevice(void)
{
    int result = Render_CreateDevice();
    memset(g_textureNames, 0, sizeof(g_textureNames));
    memset(g_textureNamesAux, 0, sizeof(g_textureNamesAux));
    return result;
}

// Texture_FindByName: returns the slot holding file `name` (case-insensitive), or -1.
extern "C" int Texture_FindByName(const char *name)
{
    if (*name == 0)
        return -1;
    for (int i = 0; i < TEXTURE_SLOT_COUNT; i++)
    {
        if (_stricmp(g_textureNames[i], name) == 0)
            return i;
    }
    return -1;
}
