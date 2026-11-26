// Preview picture of a save slot (EN build, EN-only: no JP twin found).
#include <stdio.h>
#include "hud_fade.h"
#include "save_slot.h"
#include "texture_mgr.h"

// Name of the generated preview texture.
#define SAVE_PREVIEW_TEXTURE_NAME "LOADPRE"
// Size of the file name buffer.
const int SAVE_PREVIEW_NAME_SIZE = 0x100;

// SavePreview_Show: loads the preview picture of the save in slot `slot` of the current page and makes it the texture
// of the fade panel (g_fadePanelTexture): the old preview texture is released first. When the save has no preview the
// panel texture becomes -1. The slot is remembered, so showing the same slot again does nothing. Returns 0. No direct
// caller (reached through a pointer).
extern "C" int SavePreview_Show(int slot)
{
    char name[SAVE_PREVIEW_NAME_SIZE];
    unsigned char pixels[SAVE_PREVIEW_BYTES];
    if (g_savePreviewSlot != slot)
    {
        sprintf(name, "gdata%02d.sav", g_savePage * SAVE_SLOTS_PER_PAGE + slot);
        if (Save_ReadPreview(name, pixels) != 0)
        {
            Texture_Release(g_fadePanelTexture, 1);
            g_fadePanelTexture = Texture_CreateFromPixels(SAVE_PREVIEW_TEXTURE_NAME, SAVE_PREVIEW_SIZE, SAVE_PREVIEW_SIZE, pixels, 0, 0);
            g_savePreviewSlot = slot;
            return 0;
        }
        Texture_Release(g_fadePanelTexture, 1);
        g_fadePanelTexture = -1;
        g_savePreviewSlot = slot;
    }
    return 0;
}
