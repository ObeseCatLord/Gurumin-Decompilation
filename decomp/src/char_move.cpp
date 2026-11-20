// Character movement vectors. The character records are 0x249c bytes; this file views them from their model part
// (0x10db6ee8, 0x10 bytes into each Character of world.h), where all the fields used here fall inside one record.

#include <math.h>
#include <string.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                  // 0x101ac990: zero
    void Copy(const Vec4 &source);             // 0x101acc20
    void RotateZ(float angle);                 // 0x101ac300
    float LengthSquared3();  // 0x101ac6a0: x*x + y*y + z*z
    float Length();          // 0x101ac4e0: length of x/y/z
    float Length2D();        // 0x101ac500: length of x/y
    void Fill(const float &value);
    Vec4 operator-(const Vec4 &other) const;   // 0x101aca40
};

struct CharMatrix
{
    float m[16];             // m[12], m[13], m[14]: the position
    Vec4 GetTranslation();   // 0x101ade80
};

struct CharRecord
{
    void PlayMotion(int motion, int a, int b, float speed);  // 0x1017b0e0 (world.h CharModel::M_1017b0e0)
    void AddMotionSpeed(float amount);                       // 0x101630b0 (AnimModel::AddMotionSpeed)
    char pad_0000[0x267];
    char m_0267;              // 0x267: set to 1 while an NPC idles (FUN_100b5ae0)
    char pad_0268[0x20c0];
    CharMatrix matrix;        // 0x2328: world matrix
    char pad_2368[0x24];
    int m_238c;               // 0x238c: Character::m_239c (1 = ?; app_0011.cpp plays the idle motion when non-zero)
    char pad_2390[0x18];
    Vec4 move;                // 0x23a8: movement vector set by Character_SetMove
    Vec4 m_23b8;              // 0x23b8: current speed vector (compared against 1.0 below)
    char pad_23c8[0x10];
    Vec4 m_23d8;              // 0x23d8
    char pad_23e8[0x94];
    float heading;            // 0x247c: facing angle in radians
    float targetHeading;      // 0x2480: angle the character turns towards
    char pad_2484[0x14];
    int m_2498;               // 0x2498: set to 1 by the scene start for the player
};

extern "C" CharRecord DAT_10db6ee8[0x80];

// 0x100241e0 (Character_SetMove). Sets the movement vector of character `index` from (x, y, z): half of it in the
// state where m_2498 is 1 and m_238c is not 1, otherwise 0.7 of it, doubled to 1.4 while the character is nearly
// standing still (speed vector shorter than 1). Called by the enemy and player movement code. Returns 0.
extern "C" int FUN_100241e0(int index, float x, float y, float z)
{
    if (DAT_10db6ee8[index].m_238c != 1 && DAT_10db6ee8[index].m_2498 == 1)
    {
        DAT_10db6ee8[index].move.x = x * 0.5f;
        DAT_10db6ee8[index].move.y = y * 0.5f;
        DAT_10db6ee8[index].move.z = z * 0.5f;
        return 0;
    }
    DAT_10db6ee8[index].move.x = x * 0.7f;
    DAT_10db6ee8[index].move.y = y * 0.7f;
    DAT_10db6ee8[index].move.z = z * 0.7f;
    if (DAT_10db6ee8[index].m_23b8.LengthSquared3() < 1.0f)
    {
        DAT_10db6ee8[index].move.x = x * 1.4f;
        DAT_10db6ee8[index].move.y = y * 1.4f;
        DAT_10db6ee8[index].move.z = z * 1.4f;
    }
    return 0;
}

// Per-character AI state (world.h CharState).
struct NpcState
{
    int state;                // 0x00: 0 = start, 1 = idling
    int wait;                 // 0x04: frames until the next voice line may play
    int talk;                 // 0x08: talk id: script number * 100 + line (line 0 = never speaks)
    int range;                // 0x0c: distance at which a waiting character reacts
    char pad_0010[0x28];
    int m_0038;               // 0x38
    int m_003c;
};

extern "C" NpcState DAT_1020b430[0x80];
extern "C" int DAT_10210b30[0x80];
extern "C" char DAT_10255a88[0x80];
extern "C" int FUN_1013f880(void);                                          // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_1000d620(int index, int key, int frames);                // KeyPrompt_Show
extern "C" int FUN_1001b670(int index, float range, int a);                 // Enemies_WakeNearby
extern "C" int FUN_10043cc0(int index);                                     // Char_ResetState

// atan2 on floats (an inline helper: both arguments are rounded to float first, the second one first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

// 0x100b58a0 (Npc_TalkWhenNear). Idles NPC `index` like Npc_Idle (voice sounds now and then, with talk prompt line - 1,
// or line + 1979 for lines past 20) and starts talking once the player comes closer than the talk id's hundreds (in
// units, or at once when DAT_10210b30 is set; `wakeEnemies` also wakes the enemies within 800): it turns to the
// player, speeds up its motion, plays voice 0x20c, shows prompt 9 (line 10) or 15 and its state is reset.
// Returns 0.
extern "C" int FUN_100b58a0(int index, int wakeEnemies)
{
    float distance = (DAT_10db6ee8[0].matrix.GetTranslation() - DAT_10db6ee8[index].matrix.GetTranslation()).Length2D();
    if (DAT_10210b30[index] != 0 || distance < (float)(DAT_1020b430[index].talk / 100 * 100))
    {
        DAT_10210b30[index] = 0;
        if (wakeEnemies != 0)
            FUN_1001b670(index, 800.0f, 0);
        DAT_1020b430[index].m_0038 = 1;
    }
    if (DAT_1020b430[index].state == 0)
    {
        DAT_1020b430[index].state = 1;
        return 0;
    }
    if (DAT_1020b430[index].state == 1)
    {
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        if (DAT_1020b430[index].wait > 0)
            DAT_1020b430[index].wait--;
        else if (DAT_1020b430[index].talk % 100 != 0 && FUN_1013f880() % 10000 < 150)
        {
            FUN_10065b10(index, FUN_1013f880() % 3 + 0x209, 0, 0);
            int line = DAT_1020b430[index].talk % 100;
            if (line > 20)
                FUN_1000d620(index, line + 1979, 60);
            else
                FUN_1000d620(index, line - 1, 60);
            DAT_1020b430[index].wait = 75;
        }
        if (DAT_1020b430[index].m_0038 != 0)
        {
            DAT_10db6ee8[index].targetHeading =
                Atan2(DAT_10db6ee8[0].matrix.m[12] - DAT_10db6ee8[index].matrix.m[12],
                      DAT_10db6ee8[0].matrix.m[13] - DAT_10db6ee8[index].matrix.m[13]) + 3.1415f;
            DAT_10db6ee8[index].AddMotionSpeed(20.0f);
            FUN_10065b10(index, 0x20c, 0, 0);
            if (DAT_1020b430[index].talk % 100 == 10)
                FUN_1000d620(index, 9, 60);
            else
                FUN_1000d620(index, 15, 60);
            FUN_10043cc0(index);
        }
    }
    return 0;
}

// 0x100b5ae0 (Npc_Idle). Keeps NPC `index` standing still in idle motion 2. Now and then (a 3 % chance per frame,
// then at least 75 frames apart) an NPC with a talk line plays one of the voice sounds 0x209..0x20b and shows its
// talk prompt (key script * 1000 + line - 1) for 60 frames. Returns 0.
extern "C" int FUN_100b5ae0(int index)
{
    if (DAT_10210b30[index] != 0 || DAT_1020b430[index].m_0038 != 0)
    {
        DAT_10210b30[index] = 0;
        DAT_1020b430[index].m_0038 = 0;
    }
    if (DAT_1020b430[index].state == 0)
    {
        DAT_1020b430[index].state = 1;
        return 0;
    }
    if (DAT_1020b430[index].state == 1)
    {
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        DAT_10db6ee8[index].m_0267 = 1;
        DAT_10db6ee8[index].m_23d8.Fill(0);
        DAT_10db6ee8[index].move.Fill(0);
        DAT_10db6ee8[index].m_23b8.Fill(0);
        DAT_10255a88[index] = 0;
        if (DAT_1020b430[index].wait > 0)
        {
            DAT_1020b430[index].wait--;
            return 0;
        }
        if (DAT_1020b430[index].talk % 100 != 0 && FUN_1013f880() % 10000 < 300)
        {
            FUN_10065b10(index, FUN_1013f880() % 3 + 0x209, 0, 0);
            FUN_1000d620(index, DAT_1020b430[index].talk / 100 * 1000 - 1 + DAT_1020b430[index].talk % 100, 60);
            DAT_1020b430[index].wait = 75;
        }
    }
    return 0;
}

// 0x10045c30 (Npc_WaitFacingAway). A character that waits with its back to the player: on the first frame it starts
// idle motion 2 and takes its reaction distance from the talk id (line * 10). Then it keeps turning so that it
// faces away from the player (idle motion 2 once it is within about one degree, else motion 0). When the player
// comes closer than that distance (or the character was told to react, m_0038, and has a script number) its state
// is reset to behaviour 0x1d with talk id 999 and it starts walking away (16 units along its facing). Returns 0.
extern "C" int FUN_10045c30(int index)
{
    float distance = (DAT_10db6ee8[0].matrix.GetTranslation() - DAT_10db6ee8[index].matrix.GetTranslation()).Length2D();
    NpcState *state = &DAT_1020b430[index];
    if (DAT_1020b430[index].state == 0)
    {
        DAT_1020b430[index].state = 1;
        DAT_10db6ee8[index].m_2498 = 0;
        DAT_1020b430[index].range = DAT_1020b430[index].talk % 100 * 10;
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        return 0;
    }
    {
        float turn = DAT_10db6ee8[index].heading - DAT_10db6ee8[index].targetHeading;
        while (turn > 3.1415f)
            turn -= 6.283f;
        while (turn < -3.1415f)
            turn += 6.283f;
        float squared = turn * turn;
        if (squared < 0.000304503f)
            DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        else
            DAT_10db6ee8[index].PlayMotion(0, 0, 5, 1.0f);
    }
    DAT_10db6ee8[index].targetHeading =
        Atan2(DAT_10db6ee8[0].matrix.m[12] - DAT_10db6ee8[index].matrix.m[12],
              DAT_10db6ee8[0].matrix.m[13] - DAT_10db6ee8[index].matrix.m[13]) + 3.1415f;
    if (distance < (float)DAT_1020b430[index].range ||
        (DAT_1020b430[index].m_0038 != 0 && DAT_1020b430[index].talk / 100 != 0))
    {
        memset(state, 0, sizeof(NpcState));
        DAT_1020b430[index].state = 2;
        DAT_1020b430[index].wait = 0x1d;
        DAT_1020b430[index].talk = 999;
        DAT_1020b430[index].m_003c = 0xf;
        DAT_10210b30[index] = 0;
        DAT_10db6ee8[index].m_2498 = 1;
        Vec4 step;
        step.y = -16.0f;
        step.RotateZ(-DAT_10db6ee8[index].heading);
        FUN_100241e0(index, step.x, step.y, 0);
        DAT_10db6ee8[index].m_23b8.Copy(step);
        DAT_10db6ee8[index].AddMotionSpeed(20.0f);
    }
    return 0;
}

extern "C" int FUN_10043e20(const Vec4 *target, const Vec4 *eye, float heading, float range, float angleRange,
                            float nearRange, int checkWalls);           // Char_CanSee

// 0x100b1520 (Npc_WatchForPlayer). A guard-like NPC that idles (motion 2) until it sees the player: from its head
// (75 above its origin) towards the player's head (50 above), within its talk id's line * 10 units, 63 degrees either
// side of its facing or 250 units. When it has seen the player (remembered in m_0038) or was told to react, it
// shows prompt 15 for 60 frames, plays voice 0x20c, wakes the enemies within 1500 and starts turning: from then on
// it faces the player (idle motion 2 once within one degree, else motion 0) and after 30 frames its state is reset.
// Returns 0.
extern "C" int FUN_100b1520(int index)
{
    Vec4 player;
    Vec4 self;
    player.Copy(DAT_10db6ee8[0].matrix.GetTranslation());
    self.Copy(DAT_10db6ee8[index].matrix.GetTranslation());
    (player - self).Length();      // computed and not used
    (player - self).Length2D();    // computed and not used
    if (DAT_1020b430[index].state == 0)
    {
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        DAT_1020b430[index].state = 1;
    }
    else if (DAT_1020b430[index].state == 1)
    {
        DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        player.z += 50.0f;
        self.z += 75.0f;
        if ((DAT_1020b430[index].m_0038 |= FUN_10043e20(&self, &player, DAT_10db6ee8[index].heading,
                                                         (float)(DAT_1020b430[index].talk * 10), 1.099525f, 250.0f, 1)) != 0 ||
            DAT_10210b30[index] != 0)
        {
            FUN_1000d620(index, 15, 60);
            FUN_10065b10(index, 0x20c, 0, 0);
            DAT_1020b430[index].state = 2;
            DAT_1020b430[index].wait = 0;
            FUN_1001b670(index, 1500.0f, 0);
        }
    }
    else if (DAT_1020b430[index].state == 2)
    {
        float target = Atan2(player.x - self.x, player.y - self.y) + 3.1415f;
        DAT_1020b430[index].wait++;
        DAT_10db6ee8[index].targetHeading = target;
        float turn = DAT_10db6ee8[index].heading - target;
        while (turn > 3.1415f)
            turn -= 6.283f;
        while (turn < -3.1415f)
            turn += 6.283f;
        if (turn < 0.01745f)
            DAT_10db6ee8[index].PlayMotion(2, 0, 5, 1.0f);
        else
            DAT_10db6ee8[index].PlayMotion(0, 0, 5, 1.0f);
        if (DAT_1020b430[index].wait == 0x1e)
            FUN_10043cc0(index);
    }
    DAT_10210b30[index] = 0;
    return 0;
}
