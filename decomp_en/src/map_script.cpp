// Per-frame map scripts (EN build). Ported from the matched JP source decomp/src/app_0012.cpp.
#include "game_vars.h"
#include "minigame_hud.h"

// Map_RunFrameScript: draws the mini-game HUD of the current map (maps 0x15..0x18); other maps have none.
// Called once per frame by the field update.
extern "C" int Map_RunFrameScript(void)
{
    switch (g_currentMap)
    {
    case MAP_MINIGAME_TWO_SCORES:
        MapMinigame_DrawHudTwoScores();
        break;
    case MAP_MINIGAME_SCORE:
        MapMinigame_DrawHud();
        break;
    case MAP_MINIGAME_SECONDS:
        MapMinigame_DrawTimerHud();
        break;
    case MAP_MINIGAME_MINUTES:
        MapMinigame_DrawHudMinuteTimer();
        break;
    }
    return 0;
}
