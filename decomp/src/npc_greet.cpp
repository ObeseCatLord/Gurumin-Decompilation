// NPC greetings: in some maps an NPC within 1500 units of the player starts a greeting countdown; when it reaches
// 10000 the NPC turns to face the player, shows a greeting balloon picked at random (some depend on the player's
// costume, the NPC or the map) and plays sound 0x356.

#include <math.h>

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
    Vec4();                                      // 0x101ac990: zero
    void Copy(const Vec4 &source);               // 0x101acc20
    float Length();                              // 0x101ac4e0: length of x/y/z
    Vec4 operator-(const Vec4 &other) const;     // 0x101aca40
};

struct CharMatrix
{
    float m[16];
    Vec4 GetTranslation();                       // 0x101ade80
};

// A character record (world.h Character), only the fields used here.
struct Character
{
    char pad_0000[0x2338];
    CharMatrix matrix;            // 0x2338: world matrix
    char pad_2378[0x118];
    float targetHeading;          // 0x2490: angle the character turns towards
    char pad_2494[8];
};

// The game's atan2 wrapper (float arguments, so each difference is rounded to a float first).
inline float Atan2(float y, float x)
{
    return (float)atan2(y, x);
}

extern "C" Character DAT_10db6ed8[0x80];
extern "C" int DAT_104c00e0;                     // non-zero = greetings are on
extern "C" int DAT_104c00e8;                     // the map area (greetings in areas 0x10, 0x14-0x19 and 0x28)
extern "C" int DAT_10c62ed0;                     // current map
extern "C" int DAT_10daabb8;                     // the player's costume
extern "C" int DAT_10254390[0x80];               // greeting countdown per character (10000 = greet now)
extern "C" int DAT_104bf8f8[0x7c];               // key prompt shown per character (app_0008.cpp)
extern "C" int DAT_1021d138[0x7c];               // the prompt shown
extern "C" int DAT_1021f740[0x7c];               // frames the prompt stays
extern "C" int FUN_1013f880(void);               // the game's random number
extern "C" int FUN_10065b10(int index, int sound, int always, int optional);  // Character_PlaySound

// 0x100b8220 (Npc_Greet). Greeting of character `index`: while greetings are on in one of the greeting areas and
// the player is within 1500 units, a finished countdown (<= 0) restarts at 10020-10039. At 10000 the character
// turns to face the player and shows greeting balloon 0x7d4 or 0x7d5 (one in three times 0x7d6-0x7d8 for the
// player's costumes 3, 15 and 14), replaced for characters 0x12 (0x7d0), 0x3a (0x7d1 before map 13 except map 6)
// and 0x3b (0x7dd in maps 6 and 0x1a), for 60 frames with sound 0x356; the countdown restarts at 30. Returns 0.
extern "C" int FUN_100b8220(int index)
{
    if (DAT_104c00e0 != 0 &&
        (DAT_104c00e8 == 0x17 || DAT_104c00e8 == 0x19 || DAT_104c00e8 == 0x16 || DAT_104c00e8 == 0x10 ||
         DAT_104c00e8 == 0x14 || DAT_104c00e8 == 0x18 || DAT_104c00e8 == 0x28 || DAT_104c00e8 == 0x15))
    {
        Vec4 player;
        Vec4 self;
        player.Copy(DAT_10db6ed8[0].matrix.GetTranslation());
        self.Copy(DAT_10db6ed8[index].matrix.GetTranslation());
        if ((player - self).Length() < 1500.0f && DAT_10254390[index] <= 0)
            DAT_10254390[index] = FUN_1013f880() % 20 + 10020;
    }
    if (DAT_10254390[index] != 10000)
        return 0;
    // face the player (its own scope: the vectors share the stack with the distance test's)
    {
        Vec4 player;
        Vec4 self;
        player.Copy(DAT_10db6ed8[0].matrix.GetTranslation());
        self.Copy(DAT_10db6ed8[index].matrix.GetTranslation());
        DAT_10db6ed8[index].targetHeading = Atan2(player.x - self.x, player.y - self.y) + 3.1415f;
    }
    int pick = FUN_1013f880() % 3;
    int balloon = 0;
    if (pick >= 2)
    {
        // the costume greetings, else one of the two plain ones
        switch (DAT_10daabb8)
        {
        case 3:
            balloon = 0x7d6;
            break;
        case 0xe:
            balloon = 0x7d8;
            break;
        case 0xf:
            balloon = 0x7d7;
            break;
        default:
            pick = FUN_1013f880() % 2;
            break;
        }
    }
    if (balloon == 0)
    {
        balloon = pick + 0x7d4;
        if (balloon < 0x7d6)
        {
            if (index == 0x12)
                balloon = 2000;
            else if (index == 0x3a)
            {
                if (DAT_10c62ed0 < 0xd && DAT_10c62ed0 != 6)
                    balloon = 0x7d1;
            }
            else if (index == 0x3b && (DAT_10c62ed0 == 0x1a || DAT_10c62ed0 == 6))
                balloon = 0x7dd;
        }
    }
    if (DAT_104bf8f8[index] == 0 || DAT_1021d138[index] != balloon)
    {
        DAT_104bf8f8[index] = 1;
        DAT_1021d138[index] = balloon;
    }
    DAT_1021f740[index] = 0x3c;
    FUN_10065b10(index, 0x356, 1, 0);
    DAT_10254390[index] = 0x1e;
    return 0;
}
