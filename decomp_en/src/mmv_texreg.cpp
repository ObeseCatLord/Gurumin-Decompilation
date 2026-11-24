// MMV movie player and the texture name registry (EN build). Ported from the matched JP source
// decomp/src/mmv_texreg.cpp.
#include <string.h>

#include "engine_memory.h"
#include "mmv_player.h"
#include "render.h"
#include "texture_mgr.h"

// Decodes the next frame (looping or one-shot variant depending on loop). Called once per frame while a movie plays.
void MmvPlayer::DecodeNextFrame()
{
    if (loop != 0)
        ReadFrame();
    else
        ReadFrameOnce();
}

// MmvPlayer::DrawToTexture: copies the current frame, clipped to the size of `target`, into the texture through the
// first palette, or through the second one when `secondPalette` is non-zero. Called every frame while a scene movie
// plays.
int MmvPlayer::DrawToTexture(const TextureObject *target, int useSecondPalette)
{
    if (frame != 0)
    {
        const unsigned short *colours = palette;
        if (useSecondPalette != 0)
            colours = secondPalette;
        int h = height;
        if (target->height < h)
            h = target->height;
        int w = width;
        if (target->width < w)
            w = target->width;
        const_cast<TextureObject *>(target)->WritePaletted(0, 0, w, h, width, static_cast<const unsigned char *>(frame), colours);
    }
    return 1;
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
