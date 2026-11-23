// Character kind 0x4a: a stationary attacker that idles in motion 2 and now and then (or when hit) plays one of
// its attack motions 4-6; 20 frames before an attack ends it fires effect 0x352 from its own matrix.

#include "vec4.h"

#include "effect_manager.h"

#include "anim_model.h"

#include "matrix4.h"

#include "char_state.h"

#include "char_slot.h"

extern "C" char DAT_10255a88[0x80];
extern "C" char DAT_102550e0[0x80];              // 1 = the character was hit (attack at once)
extern "C" int FUN_1013f880(void);               // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_1001d8b0(int index, int effect, Vec4 position, int a, int b, int c, int d, int e, int f, int g);

// 0x1009ddd0 (Kind4a_Update). One frame of character `slot`: when hit it attacks at once; otherwise it idles in
// motion 2, counting the cooldown down, and with a 4 % chance per frame attacks once the cooldown is over. During an
// attack the timer runs down (ending the attack at 0); at 20 frames left it plays sound 0x154 and fires effect
// 0x352 (kind 6, scale 1.5) attached to its matrix. Returns 0.
extern "C" int FUN_1009ddd0(int slot)
{
    DAT_10255a88[slot] = 0;
    DAT_10db6ee8[slot].m_0267 = 1;
    if (DAT_1020b430[slot].attacking != 1 && DAT_102550e0[slot] != 0)
    {
        // hit: one of the three attack motions (4-6), 100 frames of cooldown, 30 frames of attack
        DAT_102550e0[slot] = 0;
        DAT_10db6ee8[slot].PlayMotion(FUN_1013f880() % 3 + 4, 1, 5, 1.0f);
        DAT_1020b430[slot].cooldown = 100;
        DAT_1020b430[slot].attacking = 1;
        DAT_1020b430[slot].attackTimer = 0x1e;
    }
    if (DAT_1020b430[slot].attacking == 1)
    {
        if (DAT_1020b430[slot].attackTimer != 0)
            DAT_1020b430[slot].attackTimer--;
        if (DAT_1020b430[slot].attackTimer == 0)
            DAT_1020b430[slot].attacking = 0;
        if (DAT_1020b430[slot].attackTimer == 0x14)
        {
            Vec4 position;
            position.Fill(0.0f);
            EffectOptions shot;
            shot.m_0000 = 0;
            shot.m_0004 = DAT_10daaae8[slot].stats[STAT_EFFECT_PARAM];
            shot.m_0008 = 1.5f;
            shot.owner = slot;
            shot.m_000e = 6;
            FUN_10065b10(slot, 0x154, 0, 0);
            FUN_1001d8b0(slot, 0x352, position, -1, -1, 0, 0, 0, (int)&DAT_10db6ee8[slot].matrix, (int)&shot);
        }
    }
    else
    {
        DAT_10db6ee8[slot].PlayMotion(2, 0, 5, 1.0f);
        if (DAT_1020b430[slot].cooldown != 0)
            DAT_1020b430[slot].cooldown--;
        if (FUN_1013f880() % 100 <= 3 && DAT_1020b430[slot].cooldown == 0)
        {
            DAT_10db6ee8[slot].PlayMotion(FUN_1013f880() % 3 + 4, 1, 5, 1.0f);
            DAT_1020b430[slot].cooldown = 100;
            DAT_1020b430[slot].attacking = 1;
            DAT_1020b430[slot].attackTimer = 0x1e;
        }
    }
    return 0;
}
