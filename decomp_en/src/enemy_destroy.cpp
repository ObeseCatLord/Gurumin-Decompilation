// Destroying an enemy (EN build). Ported from decomp/src/enemy_destroy.cpp.
#include "char.h"
#include "effect_manager.h"
#include "effect_model.h"
#include "engine.h"
#include "event_slots.h"
#include "game.h"
#include "game_vars.h"
#include "listbank.h"
#include "sound_effects.h"
#include "texture_mgr.h"

// Sound played when the enemy is destroyed, and when its remains appear.
const int DESTROY_SOUND = 0x336;
const int REMAINS_SOUND = 0x337;
// Effect started above the enemy, and how far above it (units).
const int DESTROY_EFFECT = 0x28a;
const float DESTROY_EFFECT_HEIGHT = 50.0f;
const int DESTROY_EFFECT_NO_PARENT = -1;
// AnimModel::defeatState value of a destroyed character.
const int DEFEAT_STATE_DESTROYED = 6;
// Maps below this number add an event slot when an enemy is destroyed; the event and its flag.
const int DESTROY_EVENT_MAP_LIMIT = 100;
const int DESTROY_EVENT_ID = 10;
const char DESTROY_EVENT_FLAG = 1;
// g_charKind value of the remains left behind.
const char CHAR_KIND_REMAINS = (char)0xaf;
// Value the destroyed enemy's script flag is set to.
const int DESTROY_FLAG_VALUE = 0x29a;
// Degrees in half a turn (the heading is in radians).
const float DEGREES_HALF_TURN = 180.0f;
// x position the character is moved to, out of the world.
const float DESTROY_OFFWORLD_X = -250000.0f;
// Volume of the destroy sounds.
const float DESTROY_SOUND_VOLUME = 1.0f;

// Enemy_Destroy: destroys character `index`: plays sound 0x336 and starts effect 0x28a 50 units above it, and on maps
// below 100 adds event slot (10, index). Then, unless its defeat-cancel flag was set again meanwhile, its slot kind
// becomes the remains kind, sound 0x337 plays, its left-behind model is placed where it stood facing its heading, its
// script flag is set to 0x29a and the character is moved out of the world. Returns 0. Called by the enemy kinds when
// their defeat finishes.
extern "C" int Enemy_Destroy(int index)
{
    g_characterModels[index].defeatState = DEFEAT_STATE_DESTROYED;
    g_characterModels[index].defeatCancel = 0;
    Sound_PlayEffect(DESTROY_SOUND, &g_characterModels[index].matrix.GetTranslation(), DESTROY_SOUND_VOLUME);
    Vec4 top = g_characterModels[index].matrix.GetTranslation();
    top.z += DESTROY_EFFECT_HEIGHT;
    g_effectManager.Start(DESTROY_EFFECT, top, DESTROY_EFFECT_NO_PARENT, 0, 0, 0, 0, 0, 0, 0);
    if (g_currentMap < DESTROY_EVENT_MAP_LIMIT)
        EventSlot_Add(index, DESTROY_EVENT_ID, DESTROY_EVENT_FLAG);
    if (g_characterModels[index].defeatCancel == 0)
    {
        Vec4 position = g_characterModels[index].matrix.GetTranslation();
        g_charKind[index] = CHAR_KIND_REMAINS;
        Sound_PlayEffect(REMAINS_SOUND, &g_characterModels[index].matrix.GetTranslation(), DESTROY_SOUND_VOLUME);
        int model = g_characterModels[index].leftModel;
        Vec4 colour;
        colour.Set(1.0f, 1.0f, 1.0f, 1.0f);
        EffectModel_PlaceColoured(g_leftModelPaths[model], position.x, position.y, position.z, 0, 0, 0,
                                  g_characterModels[index].targetHeading * DEGREES_HALF_TURN / ANGLE_HALF_TURN, colour);
        g_textureManager.ResetFrameTimer();
        int flag = g_characterModels[index].scriptFlag;
        if (flag <= SCRIPT_FLAG_LAST && flag != 0)
            g_scriptFlags[flag] = DESTROY_FLAG_VALUE;
        g_characterModels[index].SetPosition(DESTROY_OFFWORLD_X, 0, 0);
    }
    return 0;
}
