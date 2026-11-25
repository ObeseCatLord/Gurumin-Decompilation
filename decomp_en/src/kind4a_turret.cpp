// Behaviour of the kind 0x4a turret character (EN build). Ported from decomp/src/kind4a_turret.cpp; the EN version also
// plays the shot sound only when the turret is near the camera target (or is one of the first slots).
#include "camera.h"
#include "char.h"
#include "char_dispatch.h"
#include "effect_manager.h"
#include "engine.h"
#include "sound_effects.h"

// CharState::values indices of the turret: frames of cooldown left, 1 while attacking, frames of the attack left.
const int TURRET_COOLDOWN = 0;
const int TURRET_ATTACKING = 1;
const int TURRET_ATTACK_TIMER = 2;
// Cooldown after an attack and length of an attack, in frames.
const int TURRET_COOLDOWN_FRAMES = 100;
const int TURRET_ATTACK_FRAMES = 0x1e;
// The shot is fired and the sound played when this many frames of the attack are left.
const int TURRET_FIRE_FRAME = 0x14;
// Motions: the idle motion, and the first of the three attack motions.
const int TURRET_MOTION_IDLE = 2;
const int TURRET_MOTION_ATTACK_FIRST = 4;
const int TURRET_ATTACK_MOTION_COUNT = 3;
// Motion blend arguments passed to PlayMotion, and its speed.
const int TURRET_MOTION_BLEND = 5;
const float TURRET_MOTION_SPEED = 1.0f;
// Chance per frame (percent, tested as roll <= 3) that an idle turret with no cooldown attacks.
const int TURRET_ATTACK_ROLL_RANGE = 100;
const int TURRET_ATTACK_ROLL_MAX = 3;
// Sound of the shot, and the camera distance squared within which it is heard (slots below the first enemy slot always).
const int TURRET_SHOT_SOUND = 0x154;
const float TURRET_SOUND_RANGE_SQUARED = 6250000.0f;
// Effect of the shot, its scale and its effect list.
const int TURRET_SHOT_EFFECT = 0x352;
const float TURRET_SHOT_SCALE = 1.5f;
const char TURRET_SHOT_LIST = 6;
// Effect start arguments meaning "no parent".
const int TURRET_NO_PARENT = -1;

// CharKind4a_Update: one frame of character `slot`: when hit it attacks at once; otherwise it idles in motion 2, counting
// the cooldown down, and with a 4 percent chance per frame attacks once the cooldown is over. During an attack the timer
// runs down (ending the attack at 0); at 20 frames left it plays sound 0x154 and fires effect 0x352 (list 6, scale
// 1.5) attached to its matrix. Returns 0. Called by the character dispatcher for kind 0x4a.
extern "C" int CharKind4a_Update(int slot)
{
    g_charTalkPending[slot] = 0;
    g_characterModels[slot].idling = 1;
    if (g_charState[slot].values[TURRET_ATTACKING] != 1 && g_charHitLevel[slot] != 0)
    {
        g_charHitLevel[slot] = 0;
        g_characterModels[slot].PlayMotion(Random() % TURRET_ATTACK_MOTION_COUNT + TURRET_MOTION_ATTACK_FIRST, 1,
                                           TURRET_MOTION_BLEND, TURRET_MOTION_SPEED);
        g_charState[slot].values[TURRET_COOLDOWN] = TURRET_COOLDOWN_FRAMES;
        g_charState[slot].values[TURRET_ATTACKING] = 1;
        g_charState[slot].values[TURRET_ATTACK_TIMER] = TURRET_ATTACK_FRAMES;
    }
    if (g_charState[slot].values[TURRET_ATTACKING] == 1)
    {
        if (g_charState[slot].values[TURRET_ATTACK_TIMER] != 0)
            g_charState[slot].values[TURRET_ATTACK_TIMER]--;
        if (g_charState[slot].values[TURRET_ATTACK_TIMER] == 0)
            g_charState[slot].values[TURRET_ATTACKING] = 0;
        if (g_charState[slot].values[TURRET_ATTACK_TIMER] == TURRET_FIRE_FRAME)
        {
            Vec4 position;
            position.Fill(0.0f);
            EffectOptions shot;
            shot.m_0000 = 0;
            shot.colour = g_charRecords[slot].stats[STAT_EFFECT_COLOUR];
            shot.scale = TURRET_SHOT_SCALE;
            shot.owner = slot;
            shot.m_000e = TURRET_SHOT_LIST;
            float distance = (g_characterModels[slot].matrix.GetTranslation() - g_cameraTargetNow).LengthSquared3();
            if (distance < TURRET_SOUND_RANGE_SQUARED || slot < ENEMY_SLOT_FIRST)
            {
                Vec4 soundPosition;
                soundPosition.Set(g_characterModels[slot].matrix.m[MATRIX_TRANSLATION_X],
                                  g_characterModels[slot].matrix.m[MATRIX_TRANSLATION_Y],
                                  g_characterModels[slot].matrix.m[MATRIX_TRANSLATION_Z], 0);
                Sound_PlayEffect(TURRET_SHOT_SOUND, &soundPosition, 1.0f);
            }
            g_effectManager.Start(TURRET_SHOT_EFFECT, position, TURRET_NO_PARENT, TURRET_NO_PARENT, 0, 0, 0,
                                  &g_characterModels[slot].matrix, &shot, &g_charEffectHandle[slot]);
        }
    }
    else
    {
        g_characterModels[slot].PlayMotion(TURRET_MOTION_IDLE, 0, TURRET_MOTION_BLEND, TURRET_MOTION_SPEED);
        if (g_charState[slot].values[TURRET_COOLDOWN] != 0)
            g_charState[slot].values[TURRET_COOLDOWN]--;
        if (Random() % TURRET_ATTACK_ROLL_RANGE <= TURRET_ATTACK_ROLL_MAX &&
            g_charState[slot].values[TURRET_COOLDOWN] == 0)
        {
            g_characterModels[slot].PlayMotion(Random() % TURRET_ATTACK_MOTION_COUNT + TURRET_MOTION_ATTACK_FIRST, 1,
                                               TURRET_MOTION_BLEND, TURRET_MOTION_SPEED);
            g_charState[slot].values[TURRET_COOLDOWN] = TURRET_COOLDOWN_FRAMES;
            g_charState[slot].values[TURRET_ATTACKING] = 1;
            g_charState[slot].values[TURRET_ATTACK_TIMER] = TURRET_ATTACK_FRAMES;
        }
    }
    return 0;
}
