// Per-frame countdowns of the key prompts and the corner banner (EN build). The state they work on is in dialogs.h.
#ifndef GURUMIN_EN_PROMPT_FADES_H
#define GURUMIN_EN_PROMPT_FADES_H

#include "dialogs.h"

// Counts down the display time of every shown key prompt and hides the ones whose time ran out. Returns 0. EN 0x5ac8d0.
// Called by the per-frame update (0x60cb10).
extern "C" int KeyPrompt_Tick(void);

// Moves the banner fade-in counter g_bannerFade one step: up while g_overlayClosed holds 1, down while it holds 2. When the
// counter is 0 afterwards the mode is cleared. Returns 0. EN 0x5a9e00. Called by the per-frame update (0x60cb10).
extern "C" int Banner_UpdateFade(void);

#endif
