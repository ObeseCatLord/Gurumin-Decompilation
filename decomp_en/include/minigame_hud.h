// HUDs of the map mini games (maps 0x15-0x18). JP: decomp/include/minigame_hud.h.
#ifndef GURUMIN_EN_MINIGAME_HUD_H
#define GURUMIN_EN_MINIGAME_HUD_H

// Maps whose per-frame script draws a mini-game HUD.
enum MinigameMap
{
    MAP_MINIGAME_TWO_SCORES = 0x15,    // banner and two popping scores
    MAP_MINIGAME_SCORE = 0x16,         // count-down/start/finish banner and score
    MAP_MINIGAME_SECONDS = 0x17,       // seconds timer
    MAP_MINIGAME_MINUTES = 0x18        // minutes timer and a second score
};

extern "C" int MapMinigame_DrawHudTwoScores(void);    // EN 0x5ce680 (JP 0x100308e0)
extern "C" int MapMinigame_DrawHud(void);             // EN 0x5de2d0 (JP 0x100301a0)
extern "C" int MapMinigame_DrawTimerHud(void);        // EN 0x5dd2e0 (JP 0x1002f520)
extern "C" int MapMinigame_DrawHudMinuteTimer(void);  // EN 0x5e06b0 (JP 0x100311b0)

#endif
