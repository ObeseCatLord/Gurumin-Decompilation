// The two colour blends (light/ambient colours) of the EN build. Declared once here (JP: render_vars.h).
#ifndef GURUMIN_EN_COLOUR_FADES_H
#define GURUMIN_EN_COLOUR_FADES_H

#include "vec4.h"

// Number of simultaneous colour blends.
const int COLOUR_BLEND_COUNT = 2;

extern "C" int g_colourBlendFrames[COLOUR_BLEND_COUNT];   // EN 0x021e3608 (JP DAT_10f08d30): frames left per blend (0 = idle)
extern "C" Vec4 g_colourBlendStep[COLOUR_BLEND_COUNT];    // EN 0x021ffd68 (JP DAT_10f657c4): per-frame colour step
extern "C" Vec4 g_colourBlendTarget[COLOUR_BLEND_COUNT];  // EN 0x021ffe78 (JP DAT_10f65828): colour set exactly on the last frame
extern "C" Vec4 g_colourBlendCurrent[COLOUR_BLEND_COUNT]; // EN 0x02200f88 (JP DAT_10f66978): current colour

extern "C" int g_colourBlendBigCount;      // EN 0x00909474 (JP DAT_10203424): counts colour blend starts whose last argument is 530 or more
extern "C" int g_colourEffectActive;       // EN 0x01664688 (JP DAT_10ff1df0): 1 while the scene colour effect is on
extern "C" int g_colourEffectPhase;        // EN 0x0166468c (JP DAT_11018624): phase of the colour effect, 0 when it starts
extern "C" int g_colourEffectMode;         // EN 0x01664690 (JP DAT_11018730): mode the colour effect runs in
extern "C" Vec4 g_colourEffectColour;      // EN 0x0217b630 (JP DAT_11af3f48): colour of the effect (r, g, b, 0)

// Starts colour blend `index` towards (x, y, z, w) over `frames` frames; with 1 frame or fewer the colour is set at
// once. Note the argument order: z, y, x, w. EN 0x7a59f0 (JP 0x1013fa10).
extern "C" void ColourBlend_Start(int index, int frames, float z, float y, float x, float w);
// Turns the scene colour effect on with `mode` and colour (r, g, b), or off when mode is negative. Returns 1.
// EN 0x745600 (JP 0x1016afa0).
extern "C" int ColourEffect_Set(int mode, int r, int g, int b);

// Advances the two colour blends by one frame each. EN 0x77cee0 (JP 0x1013f9a0).
extern "C" void ColourBlends_Update(void);

#endif
