// The player being knocked back (0x10055ee0, called by enemy attacks, traps and gimmicks): the knock-back motion,
// rumble and sound, and the knock-back timer DAT_10468128 that restarts the player's position history.

#include "vec4.h"
#include "matrix4.h"
#include "anim_model.h"
#include "dinput_min.h"
#include "char_vars.h"
#include "game_vars.h"
#include "render_vars.h"

extern "C" int FUN_1003ad20(int sound, const Vec4 &position, float volume);  // Sound_PlayEffect
extern "C" int FUN_100114a0(void);                                          // Scene_ResetBattleState

// Map area in which greetings do not block the knock-back (DAT_104c00e8).
#define AREA_KNOCKBACK_DURING_GREETING 0x2b
// Player motions (AnimModel::PlayMotion layers).
#define MOTION_KNOCKBACK_GET_UP 2
#define MOTION_KNOCKBACK 8
// Sound effect of the hit.
#define SOUND_KNOCKBACK 0x35c
// DAT_10468128 counts up from here while the knock-back runs; past KNOCKBACK_TIMER_END it ends.
#define KNOCKBACK_TIMER_START 9
#define KNOCKBACK_TIMER_END 9
// Number of entries in the player position history DAT_10802ad8.
#define POSITION_HISTORY_SIZE 16
#define POSITION_HISTORY_MASK 0xf

// 0x10055ee0 (Player_KnockBack). Knocks the player back unless greetings, a scene (DAT_104c0108 above 10), a menu
// or another pause state block it. A pending get-up (DAT_104bfe30) plays the get-up motion instead; otherwise the
// knock-back needs the player's idle flag (m_238c) or ground 30 units below / 20 units behind that is not clear.
// It then rumbles, plays sound 0x35c at the player, drops anything carried, stops the player (velocity 0) and
// plays the knock-back motion, starting the timer DAT_10468128. Every call also advances that timer; when it runs
// out the player gets +35 motion speed and the position history is refilled with the current position.
// Returns 0 when blocked by the scene state, else 1.
extern "C" int FUN_10055ee0(void)
{
    if ((DAT_104c01a4 != 0 && DAT_104c00e0 != 0 && DAT_104c00e8 != AREA_KNOCKBACK_DURING_GREETING) ||
        DAT_104c0108 > 10)
        return 0;

    if (DAT_1032d69c != 0 && DAT_104c0068 != 1 && DAT_104bfdf0 == 0 && DAT_104bfe1c == 0 &&
        (unsigned char)DAT_104c009a < 8)
    {
        DAT_104c010c = 0;
        DAT_104c0154 = 1;
        if (DAT_10db6ee8[0].m_229c == 0)
        {
            if (DAT_104bfe30 != 0)
            {
                DAT_104bfe30 = 0;
                DAT_10db6ee8[DAT_104bf8b4].mode = 1;
                DAT_10db6ee8[DAT_104bf8b4].PlayMotion(MOTION_KNOCKBACK_GET_UP, 1, 5, 1.0f);
            }
            else if (DAT_104bfe3c == 0)
            {
                int knockBack = 1;
                if (DAT_10db6ee8[DAT_104bf8b4].m_238c == 0)
                {
                    // Knock back only where the ground below or behind the player is not clear.
                    Vec4 offset;
                    offset.Set(0, 0, -30.0f, 0);
                    int blocked = !DAT_10db6ee8[DAT_104bf8b4].IsClearOfMap(&offset);
                    if (!blocked)
                    {
                        offset.Set(0, 0, -20.0f, 0);
                        blocked = !DAT_10db6ee8[DAT_104bf8b4].IsClear(&offset);
                        if (!blocked)
                            knockBack = 0;
                    }
                }
                if (knockBack)
                {
                    DAT_104c009c = 0;
                    DAT_10f08008.Rumble(1.0f, 100);
                    DAT_1020d430 = 15;
                    DAT_1032d6a4 = 9;
                    DAT_1020d4b8 = 0;
                    DAT_104bfdb0 = -30;
                    FUN_1003ad20(SOUND_KNOCKBACK, DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
                    if (DAT_104bfdc8 != 0)
                    {
                        // Drop the carried object.
                        DAT_10db6ee8[DAT_104bfdd8].m_0075 = 0;
                        DAT_104c0090 = 1;
                        DAT_10db6ee8[0].m_0075 = 0;
                        DAT_104bfdc8 = 0;
                        DAT_104bfdcc = 0;
                    }
                    DAT_104bfdd0 = 0;
                    DAT_104c009e = 1;
                    if (DAT_104c00e0 != 0)
                    {
                        FUN_100114a0();
                        DAT_104c0171 = 0;
                    }
                    if (DAT_10db6ee8[DAT_104bf8b4].m_238c == 0)
                    {
                        DAT_10db6ee8[DAT_104bf8b4].m_237c = 0.4f;
                        DAT_10db6ee8[DAT_104bf8b4].velocity.Set(0, 0, 0, 0);
                        DAT_10db6ee8[DAT_104bf8b4].m_238c = 1;
                    }
                    if (DAT_10db6ee8[DAT_104bf8b4].m_238c != 0)
                        DAT_104687b0 = 1;
                    DAT_1021a360 = 0;
                    DAT_104c009c = 0;
                    DAT_10db6ee8[DAT_104bf8b4].PlayMotion(MOTION_KNOCKBACK, 1, 5, 1.0f);
                    DAT_104c01e8 = 1;
                    DAT_10468128 = KNOCKBACK_TIMER_START;
                }
            }
        }
    }

    if (DAT_10468128 != 0)
        DAT_10468128++;
    if (DAT_104c009e != 0)
        DAT_104c009e--;
    if (DAT_104bfe4c != 0 || DAT_104bfe50 != 0)
        DAT_104c010c = 0;
    if (DAT_104bfe3c == 0 && DAT_10468128 > KNOCKBACK_TIMER_END)
    {
        DAT_10468128 = 0;
        DAT_10db6ee8[DAT_104bf8b4].AddMotionSpeed(35.0f);
        DAT_104c0155 = 0;
        Vec4 position = DAT_10db6ee8[0].matrix.GetTranslation();  // unused
        DAT_10db6ee8[0].m_229c = 7;
        DAT_10db6ee8[0].m_23d8.Fill(0);
        DAT_104c0228 = 0;
        DAT_104c022c = 0;
        DAT_104c022d = 0;
        for (int i = 0; i < POSITION_HISTORY_SIZE; i++)
        {
            DAT_10802ad8[DAT_104c022c].Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            DAT_104c022c = (DAT_104c022c + 1) & POSITION_HISTORY_MASK;
        }
    }
    return 1;
}
