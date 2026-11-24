// The player's attack start (0x1005ce80, Player_StartAttack) and the tables it works with.
#ifndef GURUMIN_ATTACK_START_H
#define GURUMIN_ATTACK_START_H

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
    char pad_0008[0xcac];
};

extern "C" AttackRecord DAT_108a6a58[ATTACK_COUNT];
extern "C" int DAT_104c01f8;                           // cleared on every attack request
extern "C" int DAT_10c64018;                           // attacks started by the player
extern "C" int DAT_10c6401c;                           // attacks started while DAT_10447dd0 is set (the other mode)

// 0x1005ce80 (Player_StartAttack), attack_start.cpp.
extern "C" int FUN_1005ce80(int attack, int target, char flag);

#endif
