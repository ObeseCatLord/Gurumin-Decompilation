// Greeting voices (run from 0x100f5a80): when the player walks up to a character of kind 0x140 (a talking town
// character), that character's voice line plays once.

#include <math.h>
#include "world.h"

#include "vec4.h"

#include "anim_model.h"
#include "char_state.h"
#include "char_slot.h"

// Absolute value of a float (an inline helper: its argument is rounded to float first).
inline float AbsFloat(float value)
{
    return (float)fabs(value);
}

#include "map_records.h"
#include "char_vars.h"
#include "map_vars.h"

extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound

// 0x100ba040 (Npc_PlayGreeting). While the player stands on the ground (m_239c == 1): if the character greeted
// last is still within 125 units in x/y and 15 in height nothing changes; otherwise the first kind-0x140
// character within 125 units and 25 in height becomes the greeted one and its voice (by script node and state,
// modulo 10000) plays on the player. Walking away from all of them clears the greeting after 30 frames, and a
// player over a pit clears it at once. Returns 0.
extern "C" int FUN_100ba040(void)
{
    if (DAT_104c0378 > 0 && --DAT_104c0378 == 0)
        DAT_104c0368 = 0;
    if (DAT_10db6ee8[0].m_238c == 1)
    {
        Vec4 player;
        Vec4 other;
        player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
        if (DAT_104c0364 != 0)
        {
            other.Copy(DAT_10db6ee8[DAT_104c0364].matrix.GetTranslation());
            float height = AbsFloat(other.z - player.z);
            float distance = (other - player).Length2D();
            if (height < 15.0f && distance < 125.0f)
                goto closeBy;
        }
        int i;
        for (i = 0x5c; i < 0x80; i++)
        {
            if (DAT_10daaae8[i].kind != 0x140)
                continue;
            other.Copy(DAT_10db6ee8[i].matrix.GetTranslation());
            float height = AbsFloat(other.z - player.z);
            float distance = (other - player).Length2D();
            if (height < 25.0f && distance < 125.0f)
                goto found;
        }
        if (DAT_104c0364 != 0)
            DAT_104c0378 = 0x1e;
        DAT_104c0364 = 0;
        DAT_104c0370 = 0;
        goto done;
    found:
        if (DAT_104c0364 != i)
        {
            DAT_104c0364 = i;
            int voice = DAT_10210fc0[DAT_10db6ee8[i].switchNode].points[DAT_1020b430[i].values[1]].y % 10000;
            if (voice > 5000)
                voice -= 10000;
            DAT_104c0368 = voice;
            FUN_10065b10(0, DAT_104c0368 + 0x240, 1, 0);
        }
    closeBy:
        DAT_104c0378 = 0;
    }
done:
    if (DAT_10db6ee8[0].mode == 2)
    {
        DAT_104c0364 = 0;
        DAT_104c0368 = 0;
        DAT_104c0370 = 0;
    }
    return 0;
}
