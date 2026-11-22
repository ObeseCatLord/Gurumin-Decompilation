// The player taking damage (0x1005dbf0, called by enemy attacks, traps and map hazards): health and the anti-tamper
// copies, the hurt voice, rumble, the game over movie and the hit effect.

#include "vec4.h"

struct Matrix4
{
    float m[16];
    Vec4 GetTranslation();                       // 0x101ade80
};

struct Matrix4;
struct EffectOptions;

struct EffectManager
{
    int Start(int effect, Vec4 position, int parent, int unused, int matrixOwner, int value, int frame,
              const Matrix4 *matrix, const EffectOptions *options, int *pause);  // 0x10172350
};

// A character record seen from its model part (char_move.cpp CharRecord), only the fields used here.
struct CharRecord
{
    void PlayMotion(int motion, int a, int b, float speed);  // 0x1017b0e0 (world.h CharModel::M_1017b0e0)
    char pad_0000[0x2328];
    Matrix4 matrix;           // 0x2328: world matrix
    char pad_2368[0x134];
};

// Per-slot state table at 0x10daaaec (actor_scan.cpp ActorSlot).
struct ActorSlot
{
    int health;               // 0x00
    int maxHealth;            // 0x04
    char pad_0008[0x180];
};

struct TextureObject
{
    char data[0x38];
};

struct MoviePlayer
{
    int pad_0000[4];
    int m_0010;               // 0x10: frame counter (cleared to restart)
    char pad_0014[0x38];
    char m_004c;              // 0x4c: 1 = playing
    int M_10160a90(const char *path);                // 0x10160a90 (MmvPlayer::Open)
    int M_10160960(const TextureObject *target);     // 0x10160960: decodes the current frame into `target`
};

struct Input
{
    int Rumble(float divisor, int milliseconds);     // 0x10142700
};

struct Game
{
    int RaiseGameOverEvents();                       // 0x10054a60
};

extern "C" CharRecord DAT_10db6ee8[0x80];
extern "C" ActorSlot DAT_10daaaec[0x80];
extern "C" EffectManager DAT_10daaa80;
extern "C" MoviePlayer DAT_10f08ab0;
extern "C" TextureObject DAT_118d0ba8[1600];
extern "C" int DAT_10202478;                  // texture slot the movies are decoded into
extern "C" Input DAT_10f08008;
extern "C" Game DAT_10874848;
extern "C" int DAT_104bfe78;                  // 1 once the game over has started
extern "C" int DAT_104bfe1c;
extern "C" int DAT_104bfd78;
extern "C" int DAT_1032d69c;                  // 1 once the game built-ins are registered
extern "C" int DAT_104bfe6c;
extern "C" int DAT_104c00c8;
extern "C" char DAT_104c00cc;                 // 2 = the extra life was used up this hit
extern "C" int DAT_104c00d0;
extern "C" char DAT_104c0135;
extern "C" int DAT_104c00b8;                  // non-zero = invulnerable
extern "C" int DAT_10c655f0;                  // current costume
extern "C" unsigned char DAT_104c033a;                 // face variant (char_attach.cpp)
extern "C" char DAT_104c0332;                 // non-zero = voices off
extern "C" char DAT_104c0093;                 // non-zero = damage reduced to two thirds
extern "C" int DAT_10c66d00;
extern "C" int DAT_10c64ad4;
extern "C" int DAT_10c64ad8;
extern "C" char DAT_104686b0;
extern "C" char DAT_1020d4c0;
extern "C" int FUN_1003ad20(int sound, const Vec4 &position, float volume);  // Sound_PlayEffect
extern "C" int FUN_1013f880(void);                                          // the game's random number
extern "C" int FUN_100119d0(int index, int damage, int a);                  // EventSlots_Add: shows the damage
extern "C" void FUN_1000e750(void);                                         // AntiTamper_Store
extern "C" const char *FUN_1013f8a0(const char *format, ...);               // builds a data path

// 0x1005dbf0 (Player_TakeDamage). Takes `damage` from the player's health unless a scene, menu or other pause
// state blocks it (none while invulnerable; at most 1 in costume 12, two thirds with DAT_104c0093). A hit plays a
// hurt voice (0x76..0x82, or 0x8d.. per face variant) and rumbles, shows the damage and lowers DAT_10c64ad8 by the
// damage's share of health (at least 2). At 0 health an extra life (DAT_104c00cc) refills it, else the game over
// movie and sound 0x335 start. Always starts the hit effect 0xc 50 units above the player. Returns 0.
extern "C" int FUN_1005dbf0(int damage)
{
    if (DAT_104bfe78 != 0)
        return 0;
    if (DAT_104bfe1c != 0)
        return 0;
    if (DAT_104bfd78 != 0)
        return 0;
    if (DAT_1032d69c == 0)
        return 0;
    if (DAT_104bfe6c != 0)
        return 0;
    if (DAT_104c00c8 != 0)
        return 0;
    if (DAT_104c00cc == 2)
        return 0;
    if (DAT_104c00d0 != 0)
        return 0;
    if (DAT_104c0135 != 0)
        return 0;
    if (DAT_104c00b8 != 0)
        damage = 0;
    else if (damage > 0)
    {
        if (DAT_10c655f0 == 12)
            damage = 1;
        if (DAT_104c033a == 0)
        {
            const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
            int voice = FUN_1013f880() % 13 + 0x76;
            if (DAT_104c0332 == 0)
                FUN_1003ad20(voice, position, 1.0f);
        }
        else
        {
            const Vec4 &position = DAT_10db6ee8[0].matrix.GetTranslation();
            int voice = FUN_1013f880() % 5 + 0x8d + DAT_104c033a * 15;
            if (DAT_104c0332 == 0)
                FUN_1003ad20(voice, position, 1.0f);
        }
    }
    if (DAT_104c0093 != 0)
    {
        damage = damage * 2 / 3;
        if (damage == 0)
            damage = 1;
    }
    if (damage != 0)
        DAT_10f08008.Rumble(1.0f, 100);
    FUN_100119d0(0, damage, 2);
    DAT_10daaaec[0].health -= damage;
    int maxHealth = DAT_10daaaec[0].maxHealth;
    int gauge = DAT_10c64ad8;
    if (DAT_10c66d00 == 0 && damage != 0)
    {
        int loss = damage * 100 / (maxHealth / (DAT_10c64ad4 + 1));
        if (loss <= 2)
            loss = 2;
        gauge -= loss;
    }
    DAT_10c64ad8 = gauge;
    FUN_1000e750();
    if (DAT_10daaaec[0].health <= 0)
    {
        if (DAT_104c00cc != 0)
        {
            DAT_104c00cc = 2;
            DAT_10daaaec[0].health = maxHealth;
            DAT_10874848.RaiseGameOverEvents();
            FUN_1000e750();
        }
        else
        {
            DAT_10f08ab0.M_10160a90(FUN_1013f8a0("gameover.mmv", 0));
            DAT_10f08ab0.M_10160960(&DAT_118d0ba8[DAT_10202478]);
            FUN_1003ad20(0x335, DAT_10db6ee8[0].matrix.GetTranslation(), 1.0f);
            DAT_10f08ab0.m_004c = 1;
            DAT_10f08ab0.m_0010 = 0;
            DAT_104bfe78 = 1;
            DAT_10db6ee8[0].PlayMotion(0x84, 1, 5, 1.0f);
        }
    }
    DAT_104686b0 = 60;
    DAT_1020d4c0 = 4;
    Vec4 position = DAT_10db6ee8[0].matrix.GetTranslation();
    position.z += 50.0f;
    DAT_10daaa80.Start(0xc, position, -1, 0, 0, 0, 0, 0, 0, 0);
    return 0;
}
