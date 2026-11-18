// Enemy behaviours used by the AI updaters (enemy_ai.cpp). Each one runs for several frames and returns 1 on the
// frame it finishes. Per-enemy state lives in CharState (world.h): m_0004[0] = behaviour running,
// m_0004[1] = phase, m_0004[2] = frames left in the phase.
#include <math.h>
#include "world.h"

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();
    void Copy(const Vec4 &source);
    void Fill(const float &value);
};

extern "C" int FUN_1013f880(void);  // the game's random number

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x10027560 (Enemy_IdleMotion). Plays one of the two idle motions, chosen at random (motion 5 for 146 frames or
// motion 6 for 93 frames), and waits for it to end. Returns 1 when it has ended, else 0.
extern "C" int FUN_10027560(int index)
{
    DAT_1020b430[index].m_0004[0] = 1;
    if (DAT_1020b430[index].m_0004[1] == 0)
    {
        DAT_1020b430[index].m_0004[2] = 0;
        switch (FUN_1013f880() % 2)
        {
        case 0:
            DAT_10db6ed8[index].model.M_1017b0e0(5, 1, 5, 1.0f);
            DAT_1020b430[index].m_0004[2] = 146;
            DAT_1020b430[index].m_0004[1] = 1;
            return 0;
        case 1:
            DAT_10db6ed8[index].model.M_1017b0e0(6, 1, 5, 1.0f);
            DAT_1020b430[index].m_0004[2] = 93;
            DAT_1020b430[index].m_0004[1] = 1;
            break;
        }
    }
    else if (DAT_1020b430[index].m_0004[1] == 1)
    {
        DAT_1020b430[index].m_0004[2]--;
        if (DAT_1020b430[index].m_0004[2] == 0)
        {
            DAT_1020b430[index].m_0004[1] = 0;
            return 1;
        }
    }
    return 0;
}

// 0x10027400 (Enemy_WatchPlayer). Starts motion 7 (unless it is already playing or being blended into) and then
// keeps the enemy turned towards the player until a 15-in-3000 chance per frame ends the behaviour. Returns 1 on
// the frame it ends, else 0.
extern "C" int FUN_10027400(int index)
{
    Vec4 move;
    Vec4 player;
    Vec4 self;
    player.Copy(DAT_10db6ed8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ed8[index].matrix.GetTranslation());
    DAT_1020b430[index].m_0004[0] = 1;
    move.Fill(0);
    if (DAT_1020b430[index].m_0004[1] == 0)
    {
        MotionState *motion = DAT_10db6ed8[index].motion;
        DAT_1020b430[index].m_0004[2] = 150;
        if (motion->current != 7 && (motion->blending == 0 || motion->next != 7))
            DAT_10db6ed8[index].model.M_1017b0e0(7, 1, 5, 1.0f);
        DAT_1020b430[index].m_0004[1] = 1;
    }
    else if (DAT_1020b430[index].m_0004[1] == 1)
    {
        DAT_10db6ed8[index].targetHeading = Atan2(player.x - self.x, player.y - self.y) + 3.1415f;
        if (FUN_1013f880() % 3000 < 15)
        {
            DAT_1020b430[index].m_0004[1] = 0;
            return 1;
        }
    }
    return 0;
}
