// The character slot records (0x188 bytes; CHAR_SLOT_COUNT of them at DAT_10daaae8, the save file stores slot 0's
// whole record) and their template DAT_10db4408: health, what kind of character each slot holds, its model
// definition, face set and display name. The behaviour state of a slot is CharState (char_state.h) and its model is
// the AnimModel DAT_10db6ee8[slot] (anim_model.h).
// The implicit copy assignment of this struct is 0x100357a0: the three int arrays are copied as blocks, the fields
// from 0x84 to 0xec one by one and `position` through ParamVector::operator=, so those groups are arrays and single
// fields in the original too. It is emitted only in a file that copies a record (game_reset.cpp), so it matches once
// one of its callers (FUN_1005def0, FUN_1003e710, Game_LoadData 0x100cb0c0) is decompiled with this header.
// Older code saw these records 0x9c bytes later, starting at `kind` (CharSlot views of DAT_10daab84), 4 bytes later
// (ActorSlot / ActorFace views of DAT_10daaaec), or as CharParams (char_params.h).
// This is the one declaration of this type: include this header, never redeclare it.
#ifndef GURUMIN_CHAR_SLOT_H
#define GURUMIN_CHAR_SLOT_H

// Number of character slots (also defined by char_state.h).
#ifndef CHAR_SLOT_COUNT
#define CHAR_SLOT_COUNT 0x80
#endif
// CharSlot::kind of an enemy.
#define CHAR_KIND_ENEMY 0x62

// Entries of CharSlot::stats.
enum CharStat
{
    STAT_HEALTH = 0,          // 0x04: hit points left
    STAT_MAX_HEALTH = 1,      // 0x08: initial value of the health (restored when the slot is reused)
    STAT_EFFECT_PARAM = 3,    // 0x10: passed to the effects the character starts (EffectOptions::m_0004); saved
                              // when the character turns into an enemy (char_score.cpp)
    STAT_VALUE_4 = 4,         // 0x14: meaning unknown; 999 while a companion waits outside the play area, else its
                              // owner's value (companion_update.cpp); saved with STAT_EFFECT_PARAM (char_score.cpp)
    STAT_PICKUPS = 6,         // 0x1c: the player's pickup counter, 0..99 (pickups.cpp; guarded by anti_tamper.cpp)
    STAT_GUARDED_B = 7,       // 0x20: guarded by the anti-tamper check
    STAT_COIN_BONUS = 11,     // 0x30: coins a defeated enemy drops (multiplied by the helmet's coin bonus)
    STAT_BUSY = 13            // 0x38: set to 1 while an enemy is dragged into a pit (char_pit.cpp)
};

// Entries of CharSlot::params.
enum CharParam
{
    PARAM_MODEL = 3           // 0xfc: model definition of the character (index into DAT_10b314c8)
};

// Four floats with a copy assignment of its own (0x101acc20, the same code as Vec4::Copy).
struct ParamVector
{
    float x;
    float y;
    float z;
    float w;
    ParamVector &operator=(const ParamVector &other);  // 0x101acc20 (Vec4::Copy)
};

struct CharSlot
{
    int m_0000;               // 0x00: guarded by the anti-tamper check (anti_tamper.cpp)
    int stats[16];            // 0x04: health and other values by CharStat index
    int m_0044[16];           // 0x44
    int m_0084;               // 0x84: 1 = released by its group
    int m_0088;
    int m_008c;
    int m_0090;
    int faceSet;              // 0x94: face set of the character, -1 = no face animation
    int m_0098;               // 0x98: 0x1d while held (npc_frozen.cpp)
    int kind;                 // 0x9c: behaviour kind (CHAR_KIND_ENEMY; variant * 10000 + kind while being set)
    int m_00a0;               // 0xa0: non-zero = shares the previous slot's model (char_motion.cpp); actor_scan.cpp
                              // only counts slots where it is 0
    int m_00a4;
    int m_00a8;               // 0xa8: read for the previous slot when sharing models (char_motion.cpp)
    int m_00ac;
    int routeRecord;          // 0xb0: the map record (DAT_10210fc0) whose route the character walks (map_cursor.cpp)
    int m_00b4;
    int coins;                // 0xb8: the player's coin count (slot 0; debris_update.cpp adds the coins picked up)
    int money;                // 0xbc: the player's money (slot 0, guarded by the anti-tamper check); -1 for the
                              // slots map_enter_reset.cpp clears
    int m_00c0;
    int m_00c4;
    int m_00c8;
    int helmet;               // 0xcc: the helmet item worn (slot 0; passed to Char_WearHelmet 0x100154c0 on load)
    int costume;              // 0xd0: the player's costume (slot 0; picks greeting balloons in npc_greet.cpp)
    int twoHanded;            // 0xd4: set for the two-handed weapons 0x36 and 0x37 (weapon_attach.cpp)
    int m_00d8;               // 0xd8: score value (char_score.cpp)
    int m_00dc;
    int m_00e0;
    int m_00e4;
    int m_00e8;
    float speedScale;         // 0xec: motion speed scale given to the model (AnimModel::speedScale) when it is loaded
    int params[16];           // 0xf0: by CharParam index (params[PARAM_MODEL] = model definition)
    ParamVector position;     // 0x130
    char switchesHeld;        // 0x140: number of switches the character is holding
    char m_0141;              // 0x141: set every frame while a watcher behaviour runs, cleared on reset and while an
                              // enemy pendulum swings; also 1 while a companion is down or once a life has run out
                              // (companion_update.cpp, char_status.cpp) and while dragged into a pit (char_pit.cpp)
    char held;                // 0x142: 1 while held (npc_frozen.cpp); for a companion, 1 while it waits outside the
                              // play area (companion_update.cpp)
    char m_0143;              // 0x143: copied into DAT_104679a0 by the score code (char_score.cpp)
    char m_0144;              // 0x144: kept in CharState while held (npc_frozen.cpp)
    char name[0x20];          // 0x145: display name (message_open.cpp shows slot 0's)
    char m_0165[0x20];        // 0x165
};

// Every character slot, and the template a new game copies into slot 0.
extern "C" CharSlot DAT_10daaae8[CHAR_SLOT_COUNT];
extern "C" CharSlot DAT_10db4408;

typedef char check_char_slot[sizeof(CharSlot) == 0x188 ? 1 : -1];

#endif
