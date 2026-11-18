// Character movement vectors. The character records are 0x249c bytes; this file views them from their model part
// (0x10db6ee8, 0x10 bytes into each Character of world.h), where all the fields used here fall inside one record.

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    float LengthSquared3();  // 0x101ac6a0: x*x + y*y + z*z
    void Fill(const float &value);
};

struct CharRecord
{
    void PlayMotion(int motion, int a, int b, float speed);  // 0x1017b0e0 (world.h CharModel::M_1017b0e0)
    char pad_0000[0x267];
    char m_0267;              // 0x267: set to 1 while an NPC idles (FUN_100b5ae0)
    char pad_0268[0x2124];
    int m_238c;               // 0x238c: Character::m_239c (1 = ?; app_0011.cpp plays the idle motion when non-zero)
    char pad_2390[0x18];
    Vec4 move;                // 0x23a8: movement vector set by Character_SetMove
    Vec4 m_23b8;              // 0x23b8: current speed vector (compared against 1.0 below)
    char pad_23c8[0x10];
    Vec4 m_23d8;              // 0x23d8
    char pad_23e8[0xb0];
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
    char pad_000c[0x2c];
    int m_0038;               // 0x38
    int pad_003c;
};

extern "C" NpcState DAT_1020b430[0x80];
extern "C" int DAT_10210b30[0x80];
extern "C" char DAT_10255a88[0x80];
extern "C" int FUN_1013f880(void);                                          // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound
extern "C" int FUN_1000d620(int index, int key, int frames);                // KeyPrompt_Show

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
