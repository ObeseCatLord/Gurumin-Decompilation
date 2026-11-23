// The character slot records (0x188 bytes; CHAR_SLOT_COUNT of them at DAT_10daaaec): health, what kind of character
// each slot holds, its model definition, face set and display name. The behaviour state of a slot is CharState
// (char_state.h) and its model is the AnimModel DAT_10db6ee8[slot] (anim_model.h).
// Older code saw these records 0x98 bytes later, starting at `kind` (as CharSlot views of DAT_10daab84), or as
// ActorSlot / ActorFace views of DAT_10daaaec.
// This is the one declaration of this type: include this header, never redeclare it.
#ifndef GURUMIN_CHAR_SLOT_H
#define GURUMIN_CHAR_SLOT_H

// Number of character slots (also defined by char_state.h).
#ifndef CHAR_SLOT_COUNT
#define CHAR_SLOT_COUNT 0x80
#endif
// CharSlot::kind of an enemy.
#define CHAR_KIND_ENEMY 0x62

struct CharSlot
{
    int health;               // 0x00: hit points left
    int maxHealth;            // 0x04: initial value of health (restored when the slot is reused)
    int pad_0008;
    int m_000c;               // 0x0c: passed to the effects the character starts (EffectOptions::m_0004)
    char pad_0010[0x70];
    int m_0080;               // 0x80: 1 = released by its group
    char pad_0084[0xc];
    int faceSet;              // 0x90: face set of the character, -1 = no face animation
    int m_0094;               // 0x94: 0x1d while held (npc_frozen.cpp)
    int kind;                 // 0x98: behaviour kind (CHAR_KIND_ENEMY; variant * 10000 + kind while being set)
    int m_009c;               // 0x9c: non-zero = shares the previous slot's model (char_motion.cpp); actor_scan.cpp
                              // only counts slots where it is 0
    int pad_00a0;
    int m_00a4;               // 0xa4: read for the previous slot when sharing models (char_motion.cpp)
    char pad_00a8[0x28];
    int twoHanded;            // 0xd0: set for the two-handed weapons 0x36 and 0x37 (weapon_attach.cpp)
    int m_00d4;               // 0xd4: score value (char_score.cpp)
    char pad_00d8[0x20];
    int model;                // 0xf8: model definition of the character (index into DAT_10b314c8)
    char pad_00fc[0x40];
    char switchesHeld;        // 0x13c: number of switches the character is holding
    char m_013d;              // 0x13d: set every frame while a watcher behaviour runs, cleared on reset and while an
                              // enemy pendulum swings
    char held;                // 0x13e: 1 while held (npc_frozen.cpp)
    char m_013f;              // 0x13f: copied into DAT_104679a0 by the score code (char_score.cpp)
    char m_0140;              // 0x140: kept in CharState while held (npc_frozen.cpp)
    char name[0x47];          // 0x141: display name (message_open.cpp shows slot 0's)
};

// Every character slot.
extern "C" CharSlot DAT_10daaaec[CHAR_SLOT_COUNT];

typedef char check_char_slot[sizeof(CharSlot) == 0x188 ? 1 : -1];

#endif
