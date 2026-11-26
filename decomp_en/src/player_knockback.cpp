// The player being knocked back (EN 0x5e75f0, JP Player_KnockBack): the knock-back motion, rumble and sound, and the
// knock-back timer that restarts the player's position history.
#include "anim_model.h"
#include "carry_stack.h"
#include "char.h"
#include "dialogs.h"
#include "effect_host_hooks.h"
#include "input_device.h"
#include "listbank.h"
#include "npc_greet.h"
#include "player_knockback.h"
#include "scene_start.h"
#include "sound_effects.h"
#include "vec4.h"

// Map area in which greetings do not block the knock-back.
const int AREA_KNOCKBACK_DURING_GREETING = 0x2b;
// Attack overload above which no knock-back happens.
const int KNOCKBACK_MAX_OVERLOAD = 10;
// Player states from which the knock-back is not allowed.
const int KNOCKBACK_BLOCKING_STATE = 8;
// Player motions (AnimModel::PlayMotion layers).
const int MOTION_KNOCKBACK_GET_UP = 2;
const int MOTION_KNOCKBACK = 8;
// Sound effect of the hit.
const int SOUND_KNOCKBACK = 0x35c;
// The knock-back timer counts up from here while the knock-back runs; past KNOCKBACK_TIMER_END it ends.
const int KNOCKBACK_TIMER_START = 9;
const int KNOCKBACK_TIMER_END = 9;
// Motion blend frames and the strength of the rumble.
const int KNOCKBACK_BLEND_FRAMES = 5;
const float KNOCKBACK_RUMBLE_DIVISOR = 1.0f;
const int KNOCKBACK_RUMBLE_MILLISECONDS = 100;
// Offsets probed below and behind the player, and the speed the motion gets back at the end.
const float KNOCKBACK_PROBE_DEPTH = -30.0f;
const float KNOCKBACK_PROBE_BEHIND = -20.0f;
const float KNOCKBACK_RECOVER_SPEED = 35.0f;
// Values the knock-back stores.
const int KNOCKBACK_SHAKE_FRAMES = 15;
const int KNOCKBACK_SHAKE_KIND = 9;
const int KNOCKBACK_SHAKE_OFFSET = -30;
const float KNOCKBACK_MOVE_SCALE = 0.4f;
const int KNOCKBACK_RECOVERED_BLEND_FRAMES = 7;

// EN 0x5e75f0 (JP Player_KnockBack, called by enemy attacks, traps and gimmicks through EffectHook_HostCommand).
// Knocks the player back unless greetings, a scene (attack overload above 10), a menu or another pause state block it.
// A pending get-up plays the get-up motion instead; otherwise the knock-back needs the player's idle flag or ground
// 30 units below / 20 units behind that is not clear. It then rumbles, plays the hit sound at the player, drops anything
// carried, stops the player and plays the knock-back motion, starting the timer. Every call also advances that timer;
// when it runs out the player gets +35 motion speed and the position history is refilled with the current position.
// Returns 0 when blocked by the scene state, else 1.
extern "C" int Player_KnockBack(void)
{
    if (g_battleStateA1513c != 0 && g_greetingFrames != 0 && g_greetingArea != AREA_KNOCKBACK_DURING_GREETING)
        return 0;
    if (g_attackOverload > KNOCKBACK_MAX_OVERLOAD)
        return 0;

    if (g_gameBuiltinsRegistered != 0 && g_modeBlockingKnockBack != 1 && g_stateBlockingKnockBackA == 0 &&
        g_overlayOpen == 0 && g_playerStateByte < KNOCKBACK_BLOCKING_STATE)
    {
        g_playerHitFlagA = 0;
        g_playerHitFlagB = 1;
        if (g_characterModels[0].blendFrames == 0)
        {
            if (g_getUpPending != 0)
            {
                g_getUpPending = 0;
                g_characterModels[g_playerIndex].mode = 1;
                g_characterModels[g_playerIndex].PlayMotion(MOTION_KNOCKBACK_GET_UP, 1, KNOCKBACK_BLEND_FRAMES, 1.0f);
            }
            else if (g_knockBackBlocked == 0)
            {
                int blocked = 1;
                if (g_characterModels[g_playerIndex].idleMotionEnabled == 0)
                {
                    Vec4 offset;
                    offset.Set(0, 0, KNOCKBACK_PROBE_DEPTH, 0);
                    blocked = g_characterModels[g_playerIndex].IsClearOfMap(&offset) == 0;
                    if (!blocked)
                    {
                        offset.Set(0, 0, KNOCKBACK_PROBE_BEHIND, 0);
                        blocked = g_characterModels[g_playerIndex].IsClear(&offset) == 0;
                    }
                }
                if (blocked)
                {
                    g_playerActionFlag = 0;
                    g_input.Rumble(KNOCKBACK_RUMBLE_DIVISOR, KNOCKBACK_RUMBLE_MILLISECONDS);
                    g_hitShakeFrames = KNOCKBACK_SHAKE_FRAMES;
                    g_hitShakeKind = KNOCKBACK_SHAKE_KIND;
                    g_hitShakeStep = 0;
                    g_hitShakeOffset = KNOCKBACK_SHAKE_OFFSET;
                    Sound_PlayEffect(SOUND_KNOCKBACK, &g_characterModels[0].matrix.GetTranslation(), 1.0f);
                    if (g_carryActive != 0)
                    {
                        g_characterModels[g_carriedSlot].carriedFlag = 0;
                        g_carryDropped = 1;
                        g_characterModels[0].carriedFlag = 0;
                        g_carryActive = 0;
                        g_carryState = 0;
                    }
                    g_carryTimer = 0;
                    g_knockBackCountdown = 1;
                    if (g_greetingFrames != 0)
                    {
                        Scene_ResetBattleState();
                        g_sceneResetFlag = 0;
                    }
                    if (g_characterModels[g_playerIndex].idleMotionEnabled == 0)
                    {
                        g_characterModels[g_playerIndex].m_237c = KNOCKBACK_MOVE_SCALE;
                        g_characterModels[g_playerIndex].velocity.Set(0, 0, 0, 0);
                        g_characterModels[g_playerIndex].idleMotionEnabled = 1;
                    }
                    if (g_characterModels[g_playerIndex].idleMotionEnabled != 0)
                        g_idleRestartPending = 1;
                    g_knockBackSoundFlag = 0;
                    g_playerActionFlag = 0;
                    g_characterModels[g_playerIndex].PlayMotion(MOTION_KNOCKBACK, 1, KNOCKBACK_BLEND_FRAMES, 1.0f);
                    g_knockBackStarted = 1;
                    g_battleStateBc8860 = KNOCKBACK_TIMER_START;
                }
            }
        }
    }

    if (g_battleStateBc8860 != 0)
        g_battleStateBc8860++;
    if (g_knockBackCountdown != 0)
        g_knockBackCountdown--;
    if (g_menuBlockingKnockBack != 0)
        g_playerHitFlagA = 0;
    else if (g_stateClearingHitFlag != 0)
        g_playerHitFlagA = 0;
    if (g_knockBackBlocked == 0 && g_battleStateBc8860 > KNOCKBACK_TIMER_END)
    {
        g_battleStateBc8860 = 0;
        g_characterModels[g_playerIndex].AddMotionSpeed(KNOCKBACK_RECOVER_SPEED);
        g_playerRecovered = 0;
        Vec4 position = g_characterModels[0].matrix.GetTranslation();
        g_characterModels[0].blendFrames = KNOCKBACK_RECOVERED_BLEND_FRAMES;
        g_characterModels[0].moveStep.Fill(0);
        g_historyCount = 0;
        g_historyWriteIndex = 0;
        g_historyFlag = 0;
        for (int i = 0; i < POSITION_HISTORY_SIZE; i++)
        {
            g_playerPositionHistory[g_historyWriteIndex].Copy(g_characterModels[0].matrix.GetTranslation());
            g_historyWriteIndex = (g_historyWriteIndex + 1) & POSITION_HISTORY_MASK;
        }
    }
    return 1;
}
