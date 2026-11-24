// The player's attack start (0x1005ce80, Player_StartAttack) and the tables it works with.
#ifndef GURUMIN_ATTACK_START_H
#define GURUMIN_ATTACK_START_H

#include "key_track.h"

#define ATTACK_COUNT 0x32           // attack kinds 0..0x31
#define ACTION_EVENT_COUNT 0x20     // entries of the queue DAT_10254110, ended by key -1

// One entry of the queue of upcoming attack events (16 bytes; the queue ends at the first key of -1).
struct ActionEvent
{
    int key;                        // 0x00: a frame number, or the value the caller passed with the attack
    int length;                     // 0x04: frames the event lasts (0x1e) or the attack's motion length - 1
    int kind;                       // 0x08: 0, 1 = attack, 2 = long attack
    int reserved;                   // 0x0c
};

// One attack kind of the table DAT_108a6a58 (0xcb4 bytes per entry).
struct AttackRecord
{
    int motion;                     // 0x00: motion played on the player
    int length;                     // 0x04: number of frames the attack lasts
    int m_0008;
    int effect;                     // 0x0c: effect started at effectFrame (0 = none)
    int effectFrame;                // 0x10: frame of the attack's motion that starts it (negative: see the frame test)
    int m_0014;
    int strength;                   // 0x18: damage-like value that scales the effect
    unsigned char settingA;         // 0x1c: copied to EffectOptions::settings[0]
    char pad_001d[3];
    unsigned char settingB;         // 0x20: copied to EffectOptions::settings[1]
    char pad_0021[3];
    int sound;                      // 0x24: sound played at soundFrame (0 = none)
    int soundFrame;                 // 0x28
    int sound2;                     // 0x2c
    int sound2Frame;                // 0x30
    char pad_0034[0xc];
    int m_0040;                     // 0x40: 1 = the attack is a thrust (the item's helper checks it)
    int effect2;                    // 0x44: second effect, started while DAT_104c0170 is set
    int effect2Frame;               // 0x48
    char pad_004c[4];
    int effect3;                    // 0x50: third effect
    int effect3Frame;               // 0x54
    char pad_0058[0x68];
    char name[0xc];                 // 0xc0: node name the effect attaches to (begins with "scop" for the scope)
    char pad_00cc[0x164];
    KeyBank sample;                 // 0x230: camera kick curve of the swing (x = how far the swing has pushed)
};

typedef char AttackRecordSizeCheck[sizeof(AttackRecord) == 0xcb4 ? 1 : -1];

extern "C" AttackRecord DAT_108a6a58[ATTACK_COUNT];
extern "C" int DAT_104c01f8;                           // cleared on every attack request
extern "C" int DAT_10c64018;                           // attacks started by the player
extern "C" int DAT_10c6401c;                           // attacks started while DAT_10447dd0 is set (the other mode)

// 0x1005ce80 (Player_StartAttack), attack_start.cpp.
extern "C" int FUN_1005ce80(int attack, char charged, int target, char flag);

#endif
