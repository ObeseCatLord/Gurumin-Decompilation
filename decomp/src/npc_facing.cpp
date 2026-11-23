// NPCs that face a fixed direction and turn to the player while being talked to. The character records are viewed
// from their model part at 0x10db6ee8 (like char_move.cpp).

#include <math.h>

#include "vec4.h"

#include "matrix4.h"

#include "anim_model.h"

#include "char_state.h"

#include "char_vars.h"

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x1001b880 (Npc_TalkFacingSimple). Like Npc_TalkFacing with talk motion 3: idle in motion 2 facing the configured
// direction, talking in motion 3 turned to the player (unless the facing value is 100 or more). Returns 0.
extern "C" int FUN_1001b880(int index)
{
    if (DAT_1020b430[index].idleStarted == 0)
    {
        DAT_10db6ee8[index].PlayMotion(2, 1, 5, 1.0f);
        DAT_1020b430[index].idleStarted = 1;
    }
    int state = DAT_1020b430[index].state;
    DAT_10db6ee8[index].mode = 0;
    if (state == 0)
        DAT_10db6ee8[index].targetHeading = (float)(DAT_1020b430[index].facing % 100) * 94.245f / 180.0f;
    else if (state == 1)
    {
        DAT_10db6ee8[index].PlayMotion(3, 0, 5, 1.0f);
        if (DAT_1020b430[index].facing / 100 == 0)
        {
            Vec4 player;
            Vec4 self;
            player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
            DAT_10db6ee8[index].targetHeading = Atan2(player.x - self.x, player.y - self.y) + 3.1415f;
        }
    }
    if (DAT_10c6351c == index)
    {
        if (DAT_1020b430[index].state == 0)
        {
            DAT_1020b430[index].state = 1;
            DAT_10db6ee8[index].PlayMotion(3, 1, 5, 1.0f);
            return 0;
        }
    }
    else if (DAT_1020b430[index].state == 1)
    {
        DAT_1020b430[index].state = 0;
        DAT_10db6ee8[index].PlayMotion(2, 1, 5, 1.0f);
    }
    return 0;
}

// 0x1001c210 (Npc_TalkFacing). Keeps NPC `index` in idle motion 2. Idle, it faces its configured direction; while
// talking it keeps motion 2 going and, unless its facing value is 100 or more, turns to the player. The state
// follows DAT_10c6351c (talking while this NPC is the one being talked to), restarting the motion on each change.
// Returns 0.
extern "C" int FUN_1001c210(int index)
{
    if (DAT_1020b430[index].idleStarted == 0)
    {
        DAT_10db6ee8[index].PlayMotion(2, 1, 5, 1.0f);
        DAT_1020b430[index].idleStarted = 1;
    }
    int state = DAT_1020b430[index].state;
    DAT_10db6ee8[index].mode = 0;
    if (state == 0)
        DAT_10db6ee8[index].targetHeading = (float)(DAT_1020b430[index].facing % 100) * 94.245f / 180.0f;
    else if (state == 1)
    {
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        int current = DAT_10db6ee8[index].motion->layer;
        int next = DAT_10db6ee8[index].motion->nextLayer;
        if (current != 2 && current != 3 && next != 2 && next != 3)
        {
            DAT_10db6ee8[index].PlayMotion(2, 1, 5, 1.0f);
            if (DAT_10db6ee8[index].motion->nextLayer != 2)
                DAT_10db6ee8[index].PlayMotion(0, 1, 5, 1.0f);
        }
        if (DAT_1020b430[index].facing / 100 == 0)
        {
            Vec4 player;
            Vec4 self;
            player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
            self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
            DAT_10db6ee8[index].targetHeading = Atan2(player.x - self.x, player.y - self.y) + 3.1415f;
        }
    }
    if (DAT_10c6351c == index)
    {
        if (DAT_1020b430[index].state == 0)
        {
            DAT_1020b430[index].state = 1;
            DAT_10db6ee8[index].PlayMotion(2, 1, 5, 1.0f);
        }
    }
    else if (DAT_1020b430[index].state == 1)
    {
        DAT_1020b430[index].state = 0;
        DAT_10db6ee8[index].PlayMotion(2, 1, 5, 1.0f);
    }
    return 0;
}
