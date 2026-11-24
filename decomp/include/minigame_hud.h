// State and entry point of the map 0x16 mini game HUD (map_minigame_hud.cpp).
#ifndef GURUMIN_MINIGAME_HUD_H
#define GURUMIN_MINIGAME_HUD_H

extern "C" int DAT_1020c2e8;   // frames since the mini game started (0 = not started; the start banner runs to 119)
extern "C" int DAT_1020c2ec;   // frames left of the score pop effect (0 = none)
extern "C" int DAT_1020c2e4;   // frames left of the second score pop effect (map 0x15; 0 = none)
extern "C" int DAT_1020c324;   // the second score of the map 0x15 mini game
extern "C" int DAT_10c65060;   // the mini game score

extern "C" int FUN_10161920(const char *name);   // 0x10161920: texture slot holding `name`, or -1

extern "C" int FUN_100301a0(void);   // 0x100301a0
extern "C" int FUN_1002f520(void);   // 0x1002f520 (map 0x17 variant with the seconds timer)
extern "C" int FUN_100308e0(void);   // 0x100308e0 (map 0x15 variant with two scores)

#endif
