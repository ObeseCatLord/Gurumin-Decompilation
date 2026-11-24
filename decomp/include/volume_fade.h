// Music volume fades (volume_fade.cpp).
#ifndef GURUMIN_VOLUME_FADE_H
#define GURUMIN_VOLUME_FADE_H

// 0x1000db30 (Bgm_FadeGroup100): fades music group `id` to `value` over `frames` frames.
extern "C" int FUN_1000db30(int id, int value, int frames);

#endif
