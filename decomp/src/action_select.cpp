// The player's action selector: picks the motion of the next step of an action chain.
#include "action_select_vars.h"

#include "char_vars.h"
#include "game_vars.h"
#include "map_vars.h"
#include "anim_model.h"
#include "matrix4.h"
#include "vec4.h"

#define STATE_FINISHER 10          // state of the finishing action
#define STATE_FINISHER_VARIANT 11  // its variant
#define DEFAULT_STEP_LIMIT 10      // number of steps of a state unless a state sets another
#define TARGET_NEAR_RANGE 220.0f   // distance beyond which the lower height band of the target is ignored
#define TARGET_LOW_BAND 50.0f      // target height allowance above the player
#define TARGET_HIGH_BAND 200.0f    // highest reachable target above the player; also the long lunge distance
#define MOTION_NONE -1

// 0x10011520 (Player_SelectActionMotion). Picks the motion for the next step of the player's action chain, from the
// chain state DAT_104c015c, the step DAT_104c0160 and the step limit DAT_104c0164 (motions in DAT_102024d0[state][step]).
// `*lockedFlag` non-zero = the chain is locked; `restart` non-zero only reports a restart. input1..input3 choose between
// the ground, air and armed chains. Sets *resetRequest to 1 when the chain restarts. Returns the motion or a special
// motion id (0x16..0x31), 0x2f while DAT_104c01e0 blocks the selector, -1 for none.
extern "C" int FUN_10011520(int unused, int *lockedFlag, int *resetRequest, int input1, int input2, int input3,
                            int restart)
{
    int locked = *lockedFlag;
    *resetRequest = 0;
    DAT_1049ef8c = 0;
    int count = (DAT_104c0158[0] != 0) + (DAT_104c0158[1] != 0) + (DAT_104c0158[2] != 0) + (DAT_104c0158[3] != 0);
    if (restart == 0)
    {
        int state = DAT_104c015c;
        int step = DAT_104c0160;
        do
        {
            if (DAT_104c01e0 != 0)
                return 0x2f;
            if (state == 0)
            {
                DAT_104c0164 = DEFAULT_STEP_LIMIT;
                DAT_104c01ec = 0;
                if (DAT_10db712a >= 3 && DAT_10db9274 == 0 && DAT_10db6f74 >= 2)
                {
                    if (DAT_10c64f10 == 0 && DAT_104687b4 > 11 && DAT_10c64abc != 0 && locked == 0 && count > 2)
                    {
                        DAT_104c009c = 3;
                        return 0x28;
                    }
                    if (DAT_104c0154 != 0 && DAT_10db712a > 3 && DAT_102024ac > 0)
                    {
                        DAT_1049ef8c = 1;
                        float reach = (DAT_1020e338[DAT_102024ac]->GetTranslation() -
                                       DAT_10db6ee8[0].matrix.GetTranslation()).Length2D();
                        if ((reach >= TARGET_NEAR_RANGE ||
                             DAT_10db9248 + TARGET_LOW_BAND >= DAT_1020e338[DAT_102024ac]->m[MATRIX_TRANSLATION_Z]) &&
                            DAT_10db9248 + TARGET_HIGH_BAND >= DAT_1020e338[DAT_102024ac]->m[MATRIX_TRANSLATION_Z])
                        {
                            float distance = (DAT_1020e338[DAT_102024ac]->GetTranslation() -
                                              DAT_10db6ee8[0].matrix.GetTranslation()).Length2D();
                            if (distance >= TARGET_HIGH_BAND)
                                return 0x23;
                            return 0x26;
                        }
                        return 0x2a;
                    }
                    if (locked != 0 && input3 == 0)
                    {
                        return MOTION_NONE;
                    }
                    if (input1 == 0 && input2 == 0)
                    {
                        DAT_104c009c = 3;
                        return 0x25;
                    }
                    step = 0;
                    state = 8;
                    DAT_104c0160 = 0;
                }
                else
                {
                    if (DAT_104c0156 > -4)
                    {
                        if (DAT_10c64f10 == 0 && (DAT_10c64ab4 != 0 || DAT_10c64ab8 != 0 || DAT_10c64aac != 0) &&
                            locked == 0 && count > 2)
                        {
                            if (DAT_10c64ab4 != 0 && DAT_10c64ad4 >= 2)
                                return 0x19;
                            if (DAT_10c64ab8 != 0 && DAT_10c64ad4 > 0)
                                return 0x16;
                            if (DAT_10c64aac == 0 || DAT_10c64ad4 < 0)
                                return 0x18;
                            return 0x17;
                        }
                        step = 0;
                        state = 6;
                        DAT_104c0160 = 0;
                    }
                    else
                    {
                        if (DAT_10c64f10 == 0 && DAT_10c64ac0 != 0 && locked == 0 && count > 3)
                        {
                            state = STATE_FINISHER;
                            DAT_104c015c = state;
                            if (DAT_10c64ac4 != 0)
                            {
                                state = STATE_FINISHER_VARIANT;
                                DAT_104c015c = state;
                                DAT_104c0164 = 1;
                                DAT_104c0172 = 1;
                                DAT_104c01ec = 3;
                            }
                            step = 0;
                            DAT_104c0160 = 0;
                            goto check_limit;
                        }
                        step = 0;
                        state = (input1 == 0 && input2 == 0) ? 2 : 4;
                        DAT_104c0160 = 0;
                    }
                }
                DAT_104c015c = state;
            }
            else
            {
        check_limit:
                if (DAT_104c0164 == step)
                    return DAT_102024d0[state][step];
                if (DAT_104c0164 < step)
                {
                    state = state / 2 * 2;
                    DAT_104c015c = state;
                }
            }
            if (DAT_102024d0[state][step] != MOTION_NONE)
            {
                if (locked != 0)
                {
                    if (DAT_10db9274 == 0 && DAT_10db712a > 3)
                    {
                        if (input3 == 0)
                            return 0x31;
                        return DAT_102024d0[state][step];
                    }
                    if (DAT_104c02b4 == 0)
                    {
                        DAT_104c015c = 0;
                        *resetRequest = 1;
                        return 0x24;
                    }
                    DAT_104c0160 = 0;
                    DAT_104c015c = 2;
                    return DAT_10202510;
                }
                return DAT_102024d0[state][step];
            }
            state = 0;
            DAT_104c015c = 0;
        } while (true);
    }
    if (DAT_10db712a >= 3 && DAT_10db9274 == 0 && DAT_10db6f74 >= 2)
        return 0x31;
    DAT_104c015c = 0;
    *resetRequest = 1;
    return 0x2b;
}
