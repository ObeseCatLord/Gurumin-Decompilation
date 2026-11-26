// A character held in place by a spell (EN build). Ported from the matched JP source decomp/src/npc_frozen.cpp.
#include "anim_model.h"
#include "char.h"
#include "char_move.h"
#include "effect_manager.h"
#include "effect_options.h"
#include "engine.h"
#include "listbank.h"
#include "node.h"

// CharState::state values: not started, and held.
const int HELD_STATE_START = 0;
const int HELD_STATE_HELD = 1;
// CharState::values entries: frames since the hold started and the CharRecord::heldFlag saved on the first frame.
const int HELD_VALUE_WAIT = 0;
const int HELD_VALUE_SAVED = 2;
// The held character shows its sparkle again every this many frames.
const int HELD_SPARKLE_PERIOD = 8;
// Effects: the hold effect on the first frame, the sparkle that repeats, the flash when the group releases the
// character, and the burst when it is freed. The flash starts this far above the character.
const int HELD_EFFECT_START = 0x36c6;
const int HELD_EFFECT_SPARKLE = 0x36c7;
const int HELD_EFFECT_FLASH = 0x2265;
const int HELD_EFFECT_FREED = 0x1390;
const float HELD_FLASH_HEIGHT = 50.0f;
// Name of the node the hold effect follows, and the owner offset of a character (CharState index + 100).
const char HELD_FOLLOW_NODE[] = "Bip01 Spine";
const int HELD_OWNER_BASE = 100;
// Effect parent and unused arguments meaning "none".
const int HELD_EFFECT_NO_PARENT = -1;
// The effects are turned about z by a random angle: whole degrees below this count, and degrees per half turn.
const int HELD_TURN_DEGREES = 360;
const float HELD_HALF_TURN_DEGREES = 180.0f;
// CharRecord::m_0098 value while held.
const int HELD_RECORD_STATE = 0x1d;
// Motion layer, blend frames and flag the freed character plays its idle motion with.
const int HELD_IDLE_MOTION = 2;
const int HELD_IDLE_ONCE = 1;
const int HELD_IDLE_BLEND_FRAMES = 5;
// CharRecord::m_0084 value that releases the hold from its group.
const int HELD_GROUP_RELEASED = 1;
// AnimModel::mode of a character that walks again, and of one that stands held.
const int HELD_MODE_FREE = 1;
const int HELD_MODE_HELD = 0;

// Npc_Held: on its first frame it saves the slot's heldFlag and starts effect 0x36c6 (turned at random, following the
// model's "Bip01 Spine" node or else the character); every eighth frame after that it adds sparkle 0x36c7. While the
// scripts run it stays held (idling, walkingRoute, record held and m_0098 0x1d) until g_charReleaseNow is set or its
// group releases it (record m_0084 1, with flash 0x2265 50 units above it): then its effects stop, effect 0x1390
// plays on it, the record gets its heldFlag back and its state is reset. Returns 0. Called by the behaviour dispatch.
int Npc_Held(int index)
{
    if (g_charState[index].state == HELD_STATE_START)
    {
        g_charState[index].state = HELD_STATE_HELD;
        g_charState[index].values[HELD_VALUE_SAVED] = g_charRecords[index].heldFlag;
        EffectOptions options;
        Matrix4 matrix;
        Vec4 position;
        options.owner = index + HELD_OWNER_BASE;
        options.flags |= EFFECT_OPTION_FOLLOWS_OWNER;
        matrix = RotationZ((float)(Random() % HELD_TURN_DEGREES) * ANGLE_HALF_TURN / HELD_HALF_TURN_DEGREES);
        matrix.SetTranslation(g_characterModels[index].matrix.GetTranslation());
        if (g_characterModels[index].root->Find(HELD_FOLLOW_NODE, 0) != 0)
            g_effectManager.Start(HELD_EFFECT_START, position, HELD_EFFECT_NO_PARENT, HELD_EFFECT_NO_PARENT,
                                  &g_characterModels[index].root->Find(HELD_FOLLOW_NODE, 0)->worldMatrix, 0, 0, &matrix,
                                  &options, 0);
        else
            g_effectManager.Start(HELD_EFFECT_START, position, HELD_EFFECT_NO_PARENT, HELD_EFFECT_NO_PARENT,
                                  &g_characterModels[index].matrix, 0, 0, &matrix, &options, 0);
        g_characterModels[index].mode = HELD_MODE_HELD;
        g_characterModels[index].idling = 1;
        g_characterModels[index].walkingRoute = 1;
        g_charRecords[index].held = 1;
        g_charRecords[index].m_0098 = HELD_RECORD_STATE;
        g_charTalkPending[index] = 0;
    }
    else if (++g_charState[index].values[HELD_VALUE_WAIT] % HELD_SPARKLE_PERIOD == 0)
    {
        Matrix4 matrix;
        Vec4 position;
        EffectOptions options;
        options.flags |= EFFECT_OPTION_FOLLOWS_OWNER;
        matrix = RotationZ((float)(Random() % HELD_TURN_DEGREES) * ANGLE_HALF_TURN / HELD_HALF_TURN_DEGREES);
        matrix.SetTranslation(g_characterModels[index].matrix.GetTranslation());
        g_effectManager.Start(HELD_EFFECT_SPARKLE, position, HELD_EFFECT_NO_PARENT, HELD_EFFECT_NO_PARENT, 0, 0, 0,
                              &matrix, &options, 0);
    }
    if (g_gameBuiltinsRegistered != 0)
    {
        g_characterModels[index].mode = HELD_MODE_HELD;
        g_characterModels[index].idling = 1;
        g_characterModels[index].walkingRoute = 1;
        g_charRecords[index].held = 1;
        g_charRecords[index].m_0098 = HELD_RECORD_STATE;
        g_charTalkPending[index] = 0;
        if (g_charReleaseNow[index] == 0 && g_charRecords[index].m_0084 != HELD_GROUP_RELEASED)
            return 0;
        if (g_charReleaseNow[index] == 0)
        {
            Vec4 position;
            position.Copy(g_characterModels[index].matrix.GetTranslation());
            position.z += HELD_FLASH_HEIGHT;
            g_effectManager.Start(HELD_EFFECT_FLASH, position, HELD_EFFECT_NO_PARENT, HELD_EFFECT_NO_PARENT, 0, 0, 0, 0,
                                  0, 0);
        }
        g_charRecords[index].heldFlag = (char)g_charState[index].values[HELD_VALUE_SAVED];
        g_characterModels[index].mode = HELD_MODE_FREE;
        g_characterModels[index].idling = 0;
        g_charRecords[index].held = 0;
        g_charRecords[index].m_0098 = 0;
        g_effectManager.RemoveOwnedBy(index + HELD_OWNER_BASE);
        Vec4 position;
        g_effectManager.Start(HELD_EFFECT_FREED, position, HELD_EFFECT_NO_PARENT, HELD_EFFECT_NO_PARENT, 0, 0, 0,
                              &g_characterModels[index].matrix, 0, 0);
        g_charReleaseNow[index] = 0;
        g_characterModels[index].walkingRoute = 0;
        g_characterModels[index].PlayMotion(HELD_IDLE_MOTION, HELD_IDLE_ONCE, HELD_IDLE_BLEND_FRAMES, 1.0f);
        Char_ResetState(index);
        g_charTalkPending[index] = 1;
    }
    return 0;
}
