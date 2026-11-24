// Globals and helpers of the device set-up Render_OnResetDevice (render_on_reset.cpp).
#ifndef GURUMIN_RENDER_RESET_VARS_H
#define GURUMIN_RENDER_RESET_VARS_H

extern "C" int DAT_10f654a0;                       // non-zero: the gamma ramp is set from the two values below
extern "C" int DAT_10f654ec;                       // crossfade target size (256 or more) or the divisor of the default 512/1024 (0 = 1)
extern "C" int DAT_10f65518;                       // gamma contrast, added to 256 for the slope of the ramp
extern "C" int DAT_10f6551c;                       // gamma brightness
extern "C" unsigned long DAT_10f658a8[0x400];      // table reset to RESET_TABLE_FILL

extern "C" int FUN_1013f000(unsigned long sampler, unsigned long type, unsigned long value);  // Render_SetSamplerState

#endif
