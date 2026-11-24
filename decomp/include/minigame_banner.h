// Banner drawing shared by the mini game HUDs of the maps 0x15..0x18: the count-down digits, the start word and the
// finish word are quads cut from the "minigm" sheet, shrinking towards the centre of the 640x480 layout with an
// additive glow copy.
#ifndef GURUMIN_MINIGAME_BANNER_H
#define GURUMIN_MINIGAME_BANNER_H

#include "map_vars.h"
#include "render_state.h"
#include "texture_object.h"

// Banner timeline: frames 1..89 show the count-down digits (one per second), frames 90..119 the start word.
#define BANNER_DIGIT_FRAMES 0x5a
#define BANNER_END_FRAMES 0x78
#define BANNER_FRAMES_PER_STEP 30
#define BANNER_FRAMES_PER_STEP_F 30.0f

// Centre of the 640x480 layout the banner shrinks towards.
#define BANNER_CENTRE_X 0x140
#define BANNER_CENTRE_Y 0x78

// Digit cells of the sheet: 32 texels wide on a 128 texel sheet, inset by half a texel.
#define SHEET_WIDTH 128.0f
#define DIGIT_CELL_TEXELS 32.0f
#define DIGIT_CELL_FIRST_TEXEL 0.5f
#define DIGIT_CELL_LAST_TEXEL 31.5f

// Rows (in 256ths of the sheet height) of the digits, the start word and the finish word.
#define DIGIT_V0 (129.0f / 256.0f)
#define DIGIT_V1 (191.0f / 256.0f)
#define START_V0 (1.0f / 256.0f)
#define START_V1 (63.0f / 256.0f)
#define FINISH_V0 (65.0f / 256.0f)
#define FINISH_V1 (127.0f / 256.0f)
#define WORD_U0 (1.0f / 256.0f)
#define WORD_U1 (255.0f / 256.0f)

// The banner starts BANNER_GROWTH times larger than its target: size = (base - fade * rate) * BANNER_GROWTH. The glow
// draws it a second time, additive, at GLOW_ALPHA of the alpha with its own rates.
#define GLOW_ALPHA 0.75f
#define BANNER_GROWTH 1.1f
#define DIGIT_BASE 16
#define WORD_BASE 64
#define DIGIT_RATE -12.0f
#define DIGIT_GLOW_RATE -24.0f
#define WORD_WIDTH_RATE -24.0f
#define WORD_HEIGHT_RATE -6.0f
#define WORD_GLOW_WIDTH_RATE -48.0f
#define WORD_GLOW_HEIGHT_RATE -12.0f

#define SCALED(base, fade, rate) ((int)((float)((base) - (int)((fade) * (rate))) * BANNER_GROWTH))

// Draws one banner quad of half size (halfW, halfH) around the layout centre.
#define BANNER_QUAD(slot, halfW, halfH, u0, v0, u1, v1, alpha)                                                      \
    DAT_118d0ba8[slot].DrawTinted(BANNER_CENTRE_X - (halfW), BANNER_CENTRE_Y - (halfH), (halfW) + BANNER_CENTRE_X, \
                                  (halfH) + BANNER_CENTRE_Y, u0, v0, u1, v1, 0.0f, alpha, 0.0f, 0.0f)

// Draws the additive copy of the banner at GLOW_ALPHA of `alpha`, restoring the blend mode afterwards.
#define BANNER_GLOW(halfW, halfH, u0, v0, u1, v1, alpha)                                                            \
    {                                                                                                               \
        int previousMode = DAT_10202b14;                                                                            \
        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);                                                              \
        BANNER_QUAD(DAT_102028cc, halfW, halfH, u0, v0, u1, v1, (alpha) * GLOW_ALPHA);                              \
        FUN_1013f070(RENDER_BLEND_MODE, previousMode, 0);                                                           \
    }

#endif
