// NPCs that face a fixed direction and turn to the player while being talked to. The character records are viewed
// from their model part at 0x10db6ee8 (like char_move.cpp).

#include <math.h>

#include "vec4.h"

struct CharMatrix
{
    float m[16];
    Vec4 GetTranslation();                       // 0x101ade80
};

// Motion playback state of a model (world.h MotionState).
struct MotionState
{
    char pad_0000[0x1c];
    int current;              // 0x1c: motion number playing
    char pad_0020[0x7c5c];
    int next;                 // 0x7c7c: motion being blended into
};

struct CharRecord
{
    void PlayMotion(int motion, int a, int b, float speed);  // 0x1017b0e0 (world.h CharModel::M_1017b0e0)
    char pad_0000[0x298];
    MotionState *motion;      // 0x298: the model's motion playback state
    char pad_029c[0x208c];
    CharMatrix matrix;        // 0x2328: world matrix
    char pad_2368[0x118];
    float targetHeading;      // 0x2480: angle the character turns towards
    char pad_2484[0x14];
    int m_2498;               // 0x2498
};

// Per-character script state (world.h CharState).
struct NpcState
{
    int state;                // 0x00: 0 = idle, 1 = talking
    int pad_0004;
    int facing;               // 0x08: below 100: the idle facing in steps of pi/6; 100 and up: do not turn to the
                              // player while talking
    int started;              // 0x0c: 1 once the idle motion was started
    char pad_0010[0x30];
};

extern "C" CharRecord DAT_10db6ee8[0x80];
extern "C" NpcState DAT_1020b430[0x80];
extern "C" int DAT_10c6351c;             // the character being talked to

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x1001b880 (Npc_TalkFacingSimple). Like Npc_TalkFacing with talk motion 3: idle in motion 2 facing the configured
// direction, talking in motion 3 turned to the player (unless the facing value is 100 or more). Returns 0.
extern "C" int FUN_1001b880(int index)
{
    if (DAT_1020b430[index].started == 0)
    {
        DAT_10db6ee8[index].PlayMotion(2, 1, 5, 1.0f);
        DAT_1020b430[index].started = 1;
    }
    int state = DAT_1020b430[index].state;
    DAT_10db6ee8[index].m_2498 = 0;
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
    if (DAT_1020b430[index].started == 0)
    {
        DAT_10db6ee8[index].PlayMotion(2, 1, 5, 1.0f);
        DAT_1020b430[index].started = 1;
    }
    int state = DAT_1020b430[index].state;
    DAT_10db6ee8[index].m_2498 = 0;
    if (state == 0)
        DAT_10db6ee8[index].targetHeading = (float)(DAT_1020b430[index].facing % 100) * 94.245f / 180.0f;
    else if (state == 1)
    {
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        int current = DAT_10db6ee8[index].motion->current;
        int next = DAT_10db6ee8[index].motion->next;
        if (current != 2 && current != 3 && next != 2 && next != 3)
        {
            DAT_10db6ee8[index].PlayMotion(2, 1, 5, 1.0f);
            if (DAT_10db6ee8[index].motion->next != 2)
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
