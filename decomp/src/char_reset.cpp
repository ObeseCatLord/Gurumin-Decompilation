// Resetting a character's state (called by many enemy and NPC behaviours when they start over). The character
// records are viewed from their model part at 0x10db6ee8 (like char_move.cpp), where all the fields used here fall
// inside one DAT_10db6ee8[index].

#include <string.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    void Fill(const float &value);  // 0x101acc40: sets all four components
};

struct CharRecord
{
    char pad_0000[0x12];
    char m_0012;              // 0x12
    char pad_0013[0x7a];
    char m_008d;              // 0x8d
    char pad_008e[0x1d9];
    char m_0267;              // 0x267: 1 while an NPC idles (char_move.cpp)
    char pad_0268[0x2230];
    int m_2498;               // 0x2498: 1 for characters that move at half speed (Character_SetMove)
};

// Per-character slot record, 0x188 bytes (world.h CharSlot).
struct SlotRecord
{
    int kind;                 // 0x00: character kind (0x62 = the one that fades the screen on reset)
    char pad_0004[0xa1];
    char m_00a5;              // 0xa5
    char pad_00a6[0xe2];
};

// Per-character AI state, 0x40 bytes (world.h CharState).
struct AiState
{
    int words[0xf];
    int mode;                 // 0x3c: current behaviour mode
};

// Per-character start vectors, 0x40 bytes (char_shots.cpp, char_leash.cpp).
struct StartVectors
{
    Vec4 position;            // start position
    Vec4 m_0010;
    char pad_0020[0x20];
};

// Per-character pose table entry, 0x24 bytes (pulse_scale.cpp).
struct PoseEntry
{
    int words[9];
};

extern "C" CharRecord DAT_10db6ee8[0x80];
extern "C" SlotRecord DAT_10daab84[0x80];
extern "C" AiState DAT_1020b430[0x80];
extern "C" StartVectors DAT_10802bf8[0x80];
extern "C" PoseEntry DAT_1021bdf8[0x80];
extern "C" char DAT_10255a88[0x80];
extern "C" int DAT_102199b8[0x80];    // per-character group timer
extern "C" char DAT_1020d540[0x80];   // 1 = the character is in play (char_retire.cpp clears it)
extern "C" int DAT_102548c0[0x80];    // per-character packed value, four two-digit fields AABBCCDD
extern "C" void FUN_1000ec70(int frames, float r, float g, float b, float a);   // ColourFade_Start
extern "C" int FUN_100404e0(int index, int a, int b, int c, int d, int e, int f);
extern "C" int FUN_1001b0a0(int index, int a, int b);

// 0x10043cc0 (Char_ResetState). Starts character `index` over: a kind 0x62 character in mode 11 or 12 first fades
// the screen to white over 15 frames. The AI state, the start vectors and the pose entry are cleared, a few flags
// are reset, the group timer is set to 16 and the character is marked in play. Then FUN_100404e0 is called with the
// four fields of its packed value (BB, CC, DD, AA) and FUN_1001b0a0 with zeros. Returns 0.
extern "C" int FUN_10043cc0(int index)
{
    if (DAT_10daab84[index].kind == 0x62 && (DAT_1020b430[index].mode == 11 || DAT_1020b430[index].mode == 12))
        FUN_1000ec70(15, 1.0f, 1.0f, 1.0f, 1.0f);
    DAT_10daab84[index].m_00a5 = 0;
    memset(&DAT_1020b430[index], 0, sizeof(AiState));
    DAT_10db6ee8[index].m_0267 = 0;
    DAT_10db6ee8[index].m_2498 = 1;
    DAT_10255a88[index] = 1;
    DAT_10db6ee8[index].m_008d = 0;
    DAT_10802bf8[index].position.Fill(0);
    DAT_10802bf8[index].m_0010.Fill(0);
    DAT_10db6ee8[index].m_0012 = 0;
    memset(&DAT_1021bdf8[index], 0, sizeof(PoseEntry));
    int value = DAT_102548c0[index];
    DAT_102199b8[index] = 16;
    DAT_1020d540[index] = 1;
    FUN_100404e0(index, value / 10000 % 100, value / 100 % 100, value % 100, value / 1000000 % 100, 1, 0);
    FUN_1001b0a0(index, 0, 0);
    return 0;
}
