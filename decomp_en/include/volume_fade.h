// Music volume fade state of the EN build (JP: volume_fade.cpp). This is the one declaration of these globals in
// decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_VOLUME_FADE_H
#define GURUMIN_EN_VOLUME_FADE_H

// Added to a track id by the fade functions of group 100.
const int BGM_GROUP_100 = 100;

extern "C" int g_bgmFadeFramesLeft;      // EN 0x00a20cc4 (JP DAT_104bf8e4): frames left in the volume fade
extern "C" float g_bgmFadeTarget;        // EN 0x00a20ccc (JP DAT_104bf8e8): volume the fade ends on
extern "C" int g_bgmFadeTrack;           // EN 0x00a20cd8 (JP DAT_104bf8ec): track the fade applies to (id plus group)
extern "C" float g_bgmFadeVolume;        // EN 0x00a20ce0 (JP DAT_104bf8f0): current volume
extern "C" float g_bgmFadeStep;          // EN 0x00a20cf0 (JP DAT_1032d4b0): volume change per frame

// Starts a fade of track group 100 (id + 100) to `value` over `frames` frames (0 = at once). Returns 0. EN 0x646450.
extern "C" int Bgm_FadeGroup100(int id, int value, int frames);
// Added to a track id by Bgm_FadeGroup300.
const int BGM_GROUP_300 = 300;
// Starts a fade of the tracks of group 0 (the id itself) to `value` over `frames` frames (0 = at once). Returns 0.
// EN 0x646370 (EN-only).
extern "C" int Bgm_FadeGroup0(int id, int value, int frames);
// Starts a fade of the tracks of group 300 (id + 300) and stores `colour` as the colour of screen fade kind 3. Returns 0.
// EN 0x6463e0 (EN-only).
extern "C" int Bgm_FadeGroup300(int id, int value, int frames, int colour);
// True when the point (x, y) is inside the rectangle, edges included. EN 0x5f4ad0 (the map labels it a fade).
extern "C" int __stdcall Point_InRect(int x, int y, int left, int top, int right, int bottom);

#endif
