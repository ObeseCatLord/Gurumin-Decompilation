// Map-specific per-frame updates of the special maps (EN build). C++ rather than extern "C": the function keeps an
// exception frame for its Matrix4 local.
#include "map_special.h"
#include "effect_manager.h"
#include "game.h"
#include "char.h"
#include "game_vars.h"

// While the map timer runs, calls the map's own update (maps 0x15 to 0x18). When it reaches 0 the time-up event is
// raised; afterwards the timer keeps counting down, and on the football map the marker effect is started
// FOOTBALL_MARKER_HEIGHT above the ball every frame. Returns 0. Called every frame by the map update.
// EN 0x635480 (JP 0x100bd960).
int Map_UpdateSpecial()
{
    if (g_mapTimer > 0)
    {
        switch (g_currentMap)
        {
        case MAP_TARGET_RACE:
            Map15_UpdateTargetRace();
            break;
        case MAP_DODGE_GAME:
            Map16_UpdateDodgeGame();
            break;
        case MAP_MOLE_GAME:
            Map17_UpdateMoleGame();
            break;
        case MAP_FOOTBALL_GAME:
            Map18_UpdateFootballGame();
            break;
        }
    }
    if (g_mapTimer == 0)
    {
        g_mapTimer = -1;
        g_game.RegisterEvent(EVENT_MAP_TIME_UP);
    }
    if (g_mapTimer < 0)
    {
        g_mapTimer--;
        if (g_currentMap == MAP_FOOTBALL_GAME)
        {
            Vec4 position;
            Matrix4 placement;
            placement.SetIdentity();
            placement.SetTranslation(g_characterModels[FOOTBALL_BALL_SLOT].matrix.GetTranslation());
            placement.m[MATRIX_TRANSLATION_Z] += FOOTBALL_MARKER_HEIGHT;
            g_effectManager.Start(EFFECT_FOOTBALL_MARKER, position, -1, -1, 0, 0, 0, &placement, 0, 0);
        }
    }
    return 0;
}
