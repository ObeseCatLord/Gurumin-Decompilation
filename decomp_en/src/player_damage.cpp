// The player taking damage (EN build; JP twin: decomp/src/player_damage.cpp, called by enemy attacks, traps and map
// hazards): health and the anti-tamper copies, the hurt voice, rumble, the game over movie and the hit effect.
#include "anim_model.h"
#include "anti_tamper.h"
#include "camera.h"
#include "char.h"
#include "char_attach.h"
#include "char_dispatch.h"
#include "char_status.h"
#include "dialogs.h"
#include "effect_manager.h"
#include "engine.h"
#include "event_slots.h"
#include "game.h"
#include "game_cursor.h"
#include "game_vars.h"
#include "head_look.h"
#include "input_device.h"
#include "listbank.h"
#include "marker_rider.h"
#include "mmv_player.h"
#include "player_damage.h"
#include "sound_effects.h"
#include "switches.h"
#include "texture_mgr.h"
#include "vec4.h"

// Game mode (costume) in which a hit does 1 damage at most.
const int COSTUME_ONE_DAMAGE = 12;
// Hurt voices: 13 voices from 0x76 without a helmet variant; otherwise 5 voices from 0x8d, 15 apart per variant.
const int HURT_VOICE_COUNT = 13;
const int HURT_VOICE_FIRST = 0x76;
const int HURT_VOICE_VARIANT_COUNT = 5;
const int HURT_VOICE_VARIANT_FIRST = 0x8d;
const int HURT_VOICE_VARIANT_STRIDE = 15;
// Extra life state: used up by this hit.
const int EXTRA_LIFE_USED = 2;
// Rumble of a hit: divisor and milliseconds.
const int HIT_RUMBLE_MILLISECONDS = 100;
// Share of a hit taken in two thirds mode.
const int REDUCED_DAMAGE_NUMERATOR = 2;
const int REDUCED_DAMAGE_DENOMINATOR = 3;
// Drill gauge: points per hit health point (percent) and the least loss of a hit.
const int GAUGE_LOSS_SCALE = 100;
const int GAUGE_LOSS_LEAST = 2;
// Game over movie, its sound, the motion the player falls with and its blend.
const char GAMEOVER_MOVIE[] = "gameover.mmv";
const int GAMEOVER_MOVIE_FOLDER = 0;
const int SOUND_GAME_OVER = 0x335;
const int MOTION_DEFEATED = 0x84;
const int MOTION_DEFEATED_BLEND = 5;
// Frames the pose state and the display state the player gets after a hit.
const int HIT_POSE_FRAMES = 60;
// Hit effect and its height above the player.
const int EFFECT_HIT = 0xc;
const float EFFECT_HIT_HEIGHT = 50.0f;

// Sound_PlayVoice: plays voice line `sound` at `position` unless the voices are off. Called for the hurt voices.
extern "C" int Sound_PlayVoice(int sound, const Vec4 *position, float volume)
{
    if (g_voicesOff == 0)
        return Sound_PlayEffect(sound, position, volume);
    return 0;
}

// Player_TakeDamage: takes `damage` from the player's health unless a scene, menu or other pause state blocks it (none
// while invulnerable; at most 1 in costume 12, two thirds in two thirds mode). A hit plays a hurt voice and rumbles,
// shows the damage and lowers the drill gauge by the damage's share of health (at least 2). At 0 health an extra life
// refills it, else the game over movie and sound start. Always starts the hit effect 50 units above the player.
// Returns 0. Called by enemy attacks, traps and map hazards.
extern "C" int Player_TakeDamage(int damage)
{
    if (g_updatesSuspended != 0)
        return 0;
    if (g_overlayOpen != 0)
        return 0;
    if (g_overlayFlag != 0)
        return 0;
    if (g_gameBuiltinsRegistered == 0)
        return 0;
    if (g_soundsMuted != 0)
        return 0;
    if (g_warpBoarded != 0)
        return 0;
    if (g_extraLife == EXTRA_LIFE_USED)
        return 0;
    if (g_sceneTimer != 0)
        return 0;
    if (g_sceneFlag9e33fa != 0)
        return 0;
    if (g_invulnerable != 0)
        damage = 0;
    else if (damage > 0)
    {
        if (g_guardedValue9 == COSTUME_ONE_DAMAGE)
            damage = 1;
        if (g_helmetKind == 0)
        {
            Sound_PlayVoice(Random() % HURT_VOICE_COUNT + HURT_VOICE_FIRST,
                            &g_characterModels[0].matrix.GetTranslation(), 1.0f);
        }
        else
        {
            Sound_PlayVoice(Random() % HURT_VOICE_VARIANT_COUNT + HURT_VOICE_VARIANT_FIRST +
                                g_helmetKind * HURT_VOICE_VARIANT_STRIDE,
                            &g_characterModels[0].matrix.GetTranslation(), 1.0f);
        }
    }
    if (g_damageReduced != 0)
    {
        damage = damage * REDUCED_DAMAGE_NUMERATOR / REDUCED_DAMAGE_DENOMINATOR;
        if (damage == 0)
            damage = 1;
    }
    if (damage != 0)
        g_input.Rumble(1.0f, HIT_RUMBLE_MILLISECONDS);
    EventSlot_Add(0, damage, 2);
    int health = g_charRecords[0].stats[STAT_HEALTH] - damage;
    int level = g_guardedValue6;
    int gauge = g_guardedValue7;
    g_charRecords[0].stats[STAT_HEALTH] = health;
    if (g_gaugeFrozen == 0 && damage != 0)
    {
        int loss = damage * GAUGE_LOSS_SCALE / (g_charRecords[0].stats[STAT_MAX_HEALTH] / (level + 1));
        gauge -= loss > GAUGE_LOSS_LEAST ? loss : GAUGE_LOSS_LEAST;
    }
    g_guardedValue7 = gauge;
    AntiTamper_StoreValues();
    if (health <= 0)
    {
        if (g_extraLife != 0)
        {
            g_extraLife = EXTRA_LIFE_USED;
            g_charRecords[0].stats[STAT_HEALTH] = g_charRecords[0].stats[STAT_MAX_HEALTH];
            g_game.RaiseGameOverEvents();
            AntiTamper_StoreValues();
        }
        else
        {
            g_moviePlayer.Open(Path_Data(GAMEOVER_MOVIE, GAMEOVER_MOVIE_FOLDER));
            g_moviePlayer.DrawToTexture(&g_textures[g_movieTextureSlot], 0);
            Sound_PlayEffect(SOUND_GAME_OVER, &g_characterModels[0].matrix.GetTranslation(), 1.0f);
            g_moviePlayer.state = MMV_PLAYING;
            g_moviePlayer.loop = 0;
            g_updatesSuspended = 1;
            g_characterModels[0].PlayMotion(MOTION_DEFEATED, 1, MOTION_DEFEATED_BLEND, 1.0f);
        }
    }
    g_charPoseState[PLAYER_SLOT] = HIT_POSE_FRAMES;
    g_charDisplayState[PLAYER_SLOT] = CHAR_DISPLAY_NEAR;
    Vec4 position = g_characterModels[0].matrix.GetTranslation();
    position.z += EFFECT_HIT_HEIGHT;
    g_effectManager.Start(EFFECT_HIT, position, -1, 0, 0, 0, 0, 0, 0, 0);
    return 0;
}
