// Game-over start (EN build). Ported from the matched JP source decomp/src/engine_base.cpp.
#include "anim_model.h"
#include "char.h"
#include "game_vars.h"
#include "sound_effects.h"

// Sound effect played when the game over sequence starts.
const int SOUND_GAME_OVER = 0x355;
// Volume of that effect.
const float GAME_OVER_SOUND_VOLUME = 1.0f;
// Scene timer value of the game over scene.
const int SCENE_TIMER_GAME_OVER = 0x816;
// Motion of the player character's defeat (layer, and its two playback arguments).
const int MOTION_PLAYER_DEFEAT = 0x59;
const int DEFEAT_MOTION_FIRST = 1;
const int DEFEAT_MOTION_BLEND = 5;
// The music is lowered by (100 - (setting * 5 + 50) / 2) percent of its level, given in hundredths.
const int MUSIC_SETTING_STEP = 5;
const int MUSIC_SETTING_OFFSET = 50;
const int MUSIC_PERCENT = 100;

// Game_StartGameOver: starts the game-over sequence: plays the game over sound effect, lowers the music unless sound
// is muted, stores `value`, sets the scene timer and plays the player's defeat motion. Returns 0. Called when the
// player's health runs out.
extern "C" int Game_StartGameOver(int value)
{
    Sound_PlayEffect(SOUND_GAME_OVER, 0, GAME_OVER_SOUND_VOLUME);
    if (g_soundMuted == 0)
        g_soundArcSetMusicAttenuation((MUSIC_PERCENT - (g_musicSetting * MUSIC_SETTING_STEP + MUSIC_SETTING_OFFSET) / 2) * MUSIC_PERCENT);
    g_gameOverValue = value;
    g_sceneTimer = SCENE_TIMER_GAME_OVER;
    g_characterModels[0].PlayMotion(MOTION_PLAYER_DEFEAT, DEFEAT_MOTION_FIRST, DEFEAT_MOTION_BLEND, 1.0f);
    return 0;
}
