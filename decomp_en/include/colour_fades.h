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

// Advances the two colour blends by one frame each. EN 0x77cee0 (JP 0x1013f9a0).
extern "C" void ColourBlends_Update(void);

#endif
