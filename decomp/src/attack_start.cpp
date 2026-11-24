// The player starting an attack (0x1005ce80): the request is refused in some states, otherwise the attack
// chain advances, the attack motion starts, the attack event is queued and the attack voice is played.

#include "vec4.h"

#include "matrix4.h"
#include "anim_model.h"

#include "char_slot.h"

#include "char_tables.h"
#include "char_vars.h"
#include "game_vars.h"
#include "action_select_vars.h"
#include "attack_start.h"

extern "C" int FUN_1003ad20(int sound, const Vec4 &position, float volume);  // Sound_PlayEffect
extern "C" int FUN_1013f880(void);                                          // the game's random number

#define ATTACK_EVENT_LEAD 0x1e0            // the queue is searched for this many frames ahead of the current frame
#define ATTACK_BEAT_FRAMES 15              // frames per beat of the queued event
#define ATTACK_TARGET_NONE (-100)          // targets at or below this are queued requests
#define ATTACK_COMBO_FINISH 0x2b
#define ATTACK_COMBO_FINISH_MAX 3          // the finishing attack runs three times in a row at most
#define ATTACK_QUEUE_BLEND 0x1e
#define ATTACK_LONG_FRAMES 0x2d            // attacks this long queue a long event

// Plays the shout of the player's face variant for the heavy attacks.
static __forceinline void PlayHeavyVoice()
{
    if (DAT_104c033a == 0)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 4 + 0x69;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
    if (DAT_104c033a == 1)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 2 + 0x9a;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
    if (DAT_104c033a == 2)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 2 + 0xa9;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
    if (DAT_104c033a == 3)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 2 + 0xb8;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
    if (DAT_104c033a == 4)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 2 + 0xc7;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
    if (DAT_104c033a == 5)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 2 + 0xd6;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
    if (DAT_104c033a == 6)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 3 + 0xe4;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
}

// Plays the grunt of the player's face variant for the light attacks.
static __forceinline void PlayLightVoice()
{
    if (DAT_104c033a == 0)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 5 + 100;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
    if (DAT_104c033a == 1)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 3 + 0x97;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
    if (DAT_104c033a == 2)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 3 + 0xa6;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
    if (DAT_104c033a == 3)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 3 + 0xb5;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
    if (DAT_104c033a == 4)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 3 + 0xc4;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
    if (DAT_104c033a == 5)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 3 + 0xd3;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
    if (DAT_104c033a == 6)
    {
        const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
        int voice = FUN_1013f880() % 2 + 0xe2;
        if (DAT_104c0332 == 0)
            FUN_1003ad20(voice, position, 1.0f);
    }
}

// 0x1005ce80 (Player_StartAttack). Starts attack kind `attack` (the table DAT_108a6a58; `target` is the value the
// caller attaches and `flag` is kept in DAT_104c0118). Returns 0 and changes nothing when the request is refused:
// the finishing attack 0x2b after it ran three times, a negative kind in the other mode (DAT_10447dd0 or
// DAT_1049ef8c) or while DAT_104c0128 is set (with greetings on and no cutscene a negative kind instead queues an
// event), or while greetings are on and the player is busy. Otherwise it advances the chain step DAT_104c0160
// (back to 0 when the chain's motion row ends), resets the attack state, plays the attack motion, sets the
// greeting counter DAT_104c00e0 from the motion's length, queues the attack event, computes the stamina scale
// DAT_103088b4 and plays the attack voice. Returns 1.
extern "C" int FUN_1005ce80(int attack, int target, char flag)
{
    int heavy = 0;
    if (attack == 0x10 || attack == 0x14 || attack == 0x28 || attack == 0x19 || attack == 0x17 || attack == 0x16)
        heavy = 1;
    if ((attack == ATTACK_COMBO_FINISH && DAT_104c0330 >= ATTACK_COMBO_FINISH_MAX) ||
        (DAT_104c01f8 = 0, DAT_10447dd0 != 0 && attack < 0) || (DAT_1049ef8c != 0 && attack < 0))
        return 0;
    if (DAT_104c0128 == 0)
    {
        if (attack < 0 && DAT_104c00e0 != 0)
        {
            int slot = 0;
            if (DAT_10254110[0].key != -1)
            {
                ActionEvent *event = DAT_10254110;
                do
                {
                    if ((int)event >= (int)(DAT_10254110 + ACTION_EVENT_COUNT) || event->key == DAT_1032d6a0 + ATTACK_EVENT_LEAD)
                        break;
                    event++;
                    slot++;
                } while (event->key != -1);
            }
            int frame = DAT_1032d6a0 + 5;
            DAT_10254110[slot].kind = 0;
            DAT_10254110[slot].key = (frame / ATTACK_BEAT_FRAMES) * ATTACK_BEAT_FRAMES;
            DAT_10254110[slot].length = ATTACK_QUEUE_BLEND;
            return 0;
        }
    }
    else if (attack < 0)
        return 0;
    if (attack == 0x24)
        DAT_104c02b4 = 0x1e;
    if (target < ATTACK_TARGET_NONE && DAT_104c00e0 != 0)
        return 0;
    if (DAT_104c00e8 == 0x13 || DAT_104c00e8 == 0x15 || DAT_104c00e8 == 0x16 || DAT_104c00e8 == 0x17 ||
        DAT_104c00e8 == 0x18 || DAT_104c00e8 == 0x19)
    {
        if (target > ATTACK_TARGET_NONE && DAT_104c00e0 > 4)
        {
            DAT_104c01f8 = 0;
            return 0;
        }
    }
    else if ((DAT_104c0160 > 0 ||
              (!(attack == 0x23 || attack == 0x26 || attack == 0x2a || attack == 0x2b) || DAT_104c00e8 == attack ||
               DAT_104c023e > 2)) &&
             target > ATTACK_TARGET_NONE && DAT_104c00e0 > 4)
        return 0;
    if (DAT_10db6ee8[0].m_229c != 0)
    {
        if (DAT_10db6f74 < 2 || (attack != 0x2b && attack != 0x2a && attack != 0x26 && attack != 0x23))
        {
            if (DAT_10db6f74 > 1 || (attack != 0x2b && attack != 0x2a && attack != 0x26 && attack != 0x23))
                goto skipRestore;
            DAT_104687b0 = 1;
            DAT_10db9274 = 1;
            DAT_10db9254 = 0;
        }
        DAT_10db6ee8[0].m_229c = 0;
        DAT_10db6f74 = 0;
    }
skipRestore:
    DAT_104c0160 = DAT_104c0160 + 1;
    DAT_104c02ee = 0;
    DAT_104c01a4 = 0;
    DAT_103088b4 = 0;
    DAT_10468128 = 0;
    DAT_104c0168 = 0;
    if (DAT_102024d0[DAT_104c0160 + DAT_104c015c * 8][0] == -1)
    {
        DAT_104c015c = 0;
        DAT_104c0160 = 0;
    }
    DAT_104c0118 = flag;
    DAT_104c0155 = 0;
    DAT_104c0154 = 0;
    DAT_104c00f4 = 0;
    DAT_104c0170 = 1;
    DAT_104c00e8 = attack;
    DAT_10447dd0 = DAT_1049ef8c;
    if (DAT_1049ef8c != 0)
        DAT_104c0132 = 0;
    if (attack != 0x31 && attack < 0x2b)
    {
        if (DAT_1049ef8c == 0)
            DAT_10c64018 = DAT_10c64018 + 1;
        else
            DAT_10c6401c = DAT_10c6401c + 1;
    }
    DAT_104c00ec = 1.0f;
    DAT_104c00f0 = 1.0f;
    DAT_10c67f30.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    DAT_102024c0 = -1;
    if (DAT_104bfe20 != 0 && DAT_102024ac > 0)
    {
        DAT_102024c0 = DAT_102024ac;
        DAT_108029a8.Copy(DAT_1020e338[DAT_102024ac]->GetTranslation());
    }
    if (attack == ATTACK_COMBO_FINISH)
    {
        if (DAT_104c0330 == 0)
            DAT_10db6ee8[0].PlayMotion(0x40, 1, 2, 1.0f);
        else if (DAT_104c0330 == 1)
            DAT_10db6ee8[0].PlayMotion(0x5e, 1, 2, 1.0f);
        else if (DAT_104c0330 == 2)
            DAT_10db6ee8[0].PlayMotion(0x5f, 1, 2, 1.0f);
        DAT_104c0330 = DAT_104c0330 + 1;
        DAT_104c0331 = 0;
    }
    else
    {
        DAT_104c0331 = 0;
        DAT_104c0330 = 0;
        int blendKind;
        if (attack == 0x31)
        {
            DAT_104c015c = 0;
            blendKind = 4;
        }
        else
            blendKind = 2;
        DAT_10db6ee8[0].PlayMotion(DAT_108a6a58[attack].motion, 1, blendKind, 1.0f);
    }
    DAT_10db6ee8[0].motion->m_0008 = 1.0f;
    char otherMode = DAT_10447dd0;
    if (otherMode == 0)
        DAT_104c00e0 = (int)((float)DAT_108a6a58[attack].length + 2.0f);
    else
        DAT_104c00e0 = 0x15;
    int frame = DAT_1032d6a0;
    int slot = 0;
    if (DAT_10254110[0].key != -1)
    {
        ActionEvent *event = DAT_10254110;
        do
        {
            if ((int)event >= (int)(DAT_10254110 + ACTION_EVENT_COUNT) || event->key == frame + ATTACK_EVENT_LEAD)
                break;
            event++;
            slot++;
        } while (event->key != -1);
    }
    short cutscene = DAT_104c0128;
    if (cutscene == 0 && otherMode == 0)
    {
        DAT_10254110[slot].key = target;
        int length = DAT_108a6a58[attack].length;
        DAT_10254110[slot].kind = 1;
        if (length >= ATTACK_LONG_FRAMES)
            DAT_10254110[slot].kind = 2;
        DAT_10254110[slot].length = length - 1;
    }
    int overload = DAT_104c0108 - 10;
    DAT_103088b4 = ((overload > 0 ? overload : 0) * 100) / 0x5a + 100;
    DAT_102024c8 = DAT_104c0160;
    if (DAT_104c0160 > 0 && DAT_104c015c != 0)
    {
        if (DAT_104c0160 == 1)
        {
            int phase = (frame + 0x1a4) % 0xf;
            if (phase < 5 || (DAT_104c0173 = 0, phase > 10))
                DAT_104c0173 = 1;
            if (cutscene != 0)
                DAT_104c0173 = 1;
        }
        if (attack != ATTACK_COMBO_FINISH)
        {
            if (heavy)
                PlayHeavyVoice();
            else if (DAT_104c0173 != 0 && DAT_104c0160 == 1)
                PlayLightVoice();
        }
    }
    else if (attack <= 0x2a)
    {
        int phase = (frame + 0x1a4) % 0xf;
        if (phase < 5 || phase > 10)
            DAT_104c0173 = 1;
        else
            DAT_104c0173 = 0;
        if (cutscene != 0)
            DAT_104c0173 = 1;
        if (heavy)
            PlayHeavyVoice();
        else if (DAT_104c0173 != 0)
            PlayLightVoice();
        DAT_10446ea8 = -1;
    }
    DAT_102024c4 = 0xc;
    if (attack != ATTACK_COMBO_FINISH)
        DAT_104c023e = 1;
    DAT_102024c8 = DAT_104c0160;
    return 1;
}
