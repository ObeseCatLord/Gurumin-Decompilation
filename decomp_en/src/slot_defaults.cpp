// Defaults of the page tables of the overlay mode (EN build). Ported from the matched JP source
// decomp/src/slot_defaults.cpp.
#include <string.h>

#include "dialogs.h"

// Mode 0 (new game): pages 1..PAGE_LAST_DEFAULT_ENABLED are enabled.
const int PAGE_LAST_DEFAULT_ENABLED = 0x11;
// The special layout: pages that are disabled (-1), and the ids pages 0x19..0x1e get.
const int PAGE_DISABLED = -1;
const int PAGE_SPECIAL_DISABLED_A = 0x17;
const int PAGE_SPECIAL_DISABLED_B = 0x18;
const int PAGE_SPECIAL_DISABLED_C = 0x1f;
const int PAGE_SPECIAL_FIRST_ID = 0x19;
// Texture slot value meaning "none".
const int OVERLAY_NO_TEXTURE = -1;

// OverlayPages_Reset: resets the two page tables of the overlay mode. Mode 0 (new game) enables pages 1..0x11; any other
// mode sets the special layout (pages 0x17, 0x18 and 0x1f disabled, 0x19..0x1e set to the ids 0x6f4..0x6fa). Returns 0.
// Called when the overlay mode is entered.
extern "C" int OverlayPages_Reset(int mode)
{
    memset(g_overlayPages, 0, sizeof(g_overlayPages));
    for (int i = 0; i < OVERLAY_TEXTURE_COUNT; i++)
        g_overlayTextures[i] = OVERLAY_NO_TEXTURE;
    if (mode == 0)
    {
        for (int i = 1; i <= PAGE_LAST_DEFAULT_ENABLED; i++)
            g_overlayPages[i] = 1;
        return 0;
    }
    // Pages 0x19..0x1e take the script flag numbers 0x6f4, 0x6f7, 0x6f6, 0x6f8, 0x6f9 and 0x6fa.
    g_overlayPages[PAGE_SPECIAL_DISABLED_A] = PAGE_DISABLED;
    g_overlayPages[PAGE_SPECIAL_DISABLED_B] = PAGE_DISABLED;
    g_overlayPages[PAGE_SPECIAL_FIRST_ID] = 0x6f4;
    g_overlayPages[PAGE_SPECIAL_FIRST_ID + 1] = 0x6f7;
    g_overlayPages[PAGE_SPECIAL_DISABLED_C] = PAGE_DISABLED;
    g_overlayPages[PAGE_SPECIAL_FIRST_ID + 2] = 0x6f6;
    g_overlayPages[PAGE_SPECIAL_FIRST_ID + 3] = 0x6f8;
    g_overlayPages[PAGE_SPECIAL_FIRST_ID + 4] = 0x6f9;
    g_overlayPages[PAGE_SPECIAL_FIRST_ID + 5] = 0x6fa;
    return 0;
}
