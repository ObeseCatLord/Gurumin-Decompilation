// Per-frame aging of the texture slots (EN build, EN-only: no JP twin found).
#include "frame_pacer.h"
#include "buffer_pool.h"

// TextureSlots_Age: adds one frame to the age counter of every texture slot. Returns 1. Called once per drawn frame
// by Frame_Pace (EN 0x7a8460).
extern "C" int TextureSlots_Age(void)
{
    for (int slot = 0; slot < SURFACE_SLOT_COUNT; slot++)
        g_slotAge[slot]++;
    return 1;
}
