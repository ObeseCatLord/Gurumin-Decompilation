// The character slots of the EN build: per-slot records, behaviour state and the per-slot tables. Names follow the JP
// headers (decomp/include/char_slot.h, char_state.h, char_tables.h); offsets are proven by matched EN functions.
// This is the one declaration of CharRecord, CharState and these tables in decomp_en: include this header, never
// redeclare them.
#ifndef GURUMIN_EN_CHAR_H
#define GURUMIN_EN_CHAR_H

#include "anim_model.h"

// Number of character slots.
const int CHAR_SLOT_COUNT = 0x80;
// First slot of the object slots (0x5c..0x7b share the enemy slots' end).
const int OBJECT_SLOT_FIRST = 0x5c;
// g_charKind values of objects the actor scans look for.
const int OBJECT_KIND_NEAR_POINT = 0x5c;
const int OBJECT_KIND_BLOCKER = 0x14;
// Object_IsNearPoint: half size of the object's square and of the point's square.
const float OBJECT_NEAR_HALF_SIZE = 125.0f;
const float POINT_NEAR_HALF_SIZE = 25.0f;
// Object_SlowNearBlocker: distance below which a blocker slows a moving object, and the speed factor it gets.
const float BLOCKER_RANGE = 400.0f;
const float BLOCKER_SLOWDOWN = 0.05f;
// Char_IsStandingOn: height of a character's feet above its position.
const float STAND_MIN_HEIGHT = 20.0f;
// Saved node matrices kept per character.
const int CHAR_NODE_MATRIX_COUNT = 6;
// CharState::values index of the frame counter the item stand camera shot keeps in character 0's state.
const int CHARSTATE_CAMERA_FRAMES = 2;
// Characters in the party table (g_partyCharacters).
const int PARTY_SIZE = 5;
// Number of entries of the group table.
const int CHAR_GROUP_COUNT = 0x5c;
// Bytes of a CharRecord.
const int CHAR_RECORD_SIZE = 0x188;
// Bytes of a CharState.
const int CHAR_STATE_SIZE = 0x40;
// First slot and one past the last slot that hold enemies.
const int ENEMY_SLOT_FIRST = 0x3a;
const int ENEMY_SLOT_END = 0x7c;
// g_charKind value of a live enemy.
const int CHAR_KIND_ENEMY_SLOT = 3;
// g_charPhase value of a character that has finished its defeat.
const int CHAR_PHASE_FINISHED = 2;
// x position below which a character is out of the world.
const float CHAR_OFFWORLD_X = -50000.0f;
// x position above which a character still counts as in the world (the clear-health scan compares with this).
const float CHAR_INWORLD_X = -49999.0f;
// CharState::values[1] value meaning "no formation" (the group becomes -1).
const int CHAR_FORMATION_NONE = 100;
// Group number of a character without a group.
const int CHAR_GROUP_NONE = -1;
// Model parameter slot count.
const int CHAR_PARAM_COUNT = 16;
// Stat slot count.
const int CHAR_STAT_COUNT = 16;

// Entries of CharRecord::stats.
enum CharStat
{
    STAT_HEALTH = 0,          // 0x04: hit points left
    STAT_MAX_HEALTH = 1       // 0x08: initial value of the health (restored when the slot is reused)
};

// Entries of CharRecord::params.
enum CharParam
{
    PARAM_MODEL = 3           // 0xfc: model definition of the character (index into g_modelPaths)
};

// A character slot record (0x188 bytes). Layout and names follow the JP CharSlot (decomp/include/char_slot.h).
struct CharRecord
{
    int m_0000;                              // 0x00: not used by EN code yet
    int stats[CHAR_STAT_COUNT];              // 0x04: health and other values by CharStat index
    char m_0044[0x9c - 0x44];                // 0x44: not used by EN code yet
    int kind;                                // 0x9c: behaviour kind
    int sharesModel;                         // 0xa0: non-zero = shares the previous slot's model; the actor scans only count slots where it is 0
    char m_00a4[0xf0 - 0xa4];                // 0xa4: not used by EN code yet
    int params[CHAR_PARAM_COUNT];            // 0xf0: by CharParam index
    char m_0130[0x141 - 0x130];              // 0x130: position and switches held
    char defeated;                           // 0x141: 1 once the character is marked defeated
    char m_0142[CHAR_RECORD_SIZE - 0x142];   // 0x142: held flag and name (not used by EN code yet)
};

// The behaviour state of a character slot (0x40 bytes); every behaviour keeps its own variables in `values`.
struct CharState
{
    int state;                // 0x00: behaviour step (0 = not started yet)
    int values[13];           // 0x04: behaviour variables (the AI pickers use values[0] and values[1])
    int m_0038;               // 0x38: not used by EN code yet
    int m_003c;               // 0x3c: not used by EN code yet
};

// The model file of a model number.
struct ModelPath
{
    char path[0x4b34];        // 0x0000: path of the model file
};

extern "C" CharRecord g_charRecords[CHAR_SLOT_COUNT];      // EN 0x00f381e0 (JP DAT_10daaae8)
extern "C" CharState g_charState[CHAR_SLOT_COUNT];         // EN 0x009160e0 (JP DAT_1020b430)
extern "C" int g_charPhase[CHAR_SLOT_COUNT];               // EN 0x00915cb8 (JP DAT_10210d30): non-zero = busy; 2 = finished
extern "C" char g_charKind[CHAR_SLOT_COUNT];               // EN 0x00a35bc8 (JP DAT_1021f3c0): kind of each slot (3 = enemy)
extern "C" int g_charGroup[CHAR_GROUP_COUNT];              // EN 0x0091f818 (JP DAT_10255f10): group of each character, -1 = none
extern "C" unsigned char g_charSpawnFlag[CHAR_SLOT_COUNT];   // EN 0x009e1bd0 (JP DAT_10315350): a per-character flag, cleared when a slot is reused
extern "C" const char g_partyCharacters[PARTY_SIZE];        // EN 0x00848850 (JP DAT_101d67f0): slots 0, 13, 15, 58, 20
extern "C" Matrix4 g_characterNodeMatrices[CHAR_SLOT_COUNT][CHAR_NODE_MATRIX_COUNT];   // EN 0x015ad660 (JP DAT_10868838): saved node matrices of each character; [0][0] of character 0 is the matrix the item stand sparkle follows
extern "C" int g_charEffectHandle[CHAR_SLOT_COUNT];        // EN 0x009e2078 (JP DAT_10219770): effect handle of each slot
extern "C" AnimModel g_characterModels[CHAR_SLOT_COUNT];   // EN 0x01487ff0 (JP DAT_10db6ee8): the model of each slot
extern "C" ModelPath g_modelPaths[];                       // EN 0x01201270 (JP DAT_10b314c8): the model file of each model number
extern "C" int g_sceneTimer;                               // EN 0x009e33a0 (JP DAT_104c00d0): scene timer
extern "C" int g_gameOverValue;                            // EN 0x00a150b8 (JP DAT_104c0198): value stored by the game-over start

struct EffectOptions;

// Marks character `index` as defeated unless it is busy; returns 1 when busy. EN 0x680210.
extern "C" int Char_MarkDefeated(int index);
// Reloads character `index`'s model from the model path table. EN 0x606fe0.
extern "C" int Char_ReloadModel(int index);
// Stores the character's formation number as its group and refreshes the group links. EN 0x6abe20.
extern "C" int Char_UpdateGroup(int index);
// Random idle animation: a 1 % chance per call to play the idle motion. EN 0x6ac220.
extern "C" int Char_RandomIdle(int index);
// Moves character `index` to the position of character `index - 2`. EN 0x6bd030.
extern "C" int Char_FollowLeader(int index);
// Starts effect `effect` for character slot `index` and records its handle. EN 0x6a2c20.
extern "C" int Char_StartEffect(int index, int effect, Vec4 position, int parent, int unused, const Matrix4 *owner,
                                int value, int frame, const Matrix4 *matrix, const EffectOptions *options);
// Resets the behaviour state of character `index` (JP 0x10043cc0). EN 0x6d2070.
extern "C" int Char_ResetState(int index);
// Stores `value`, sets the scene timer and plays the player's defeat motion. EN 0x5ec510.
extern "C" int GameOver_StartQuiet(int value);
// Returns the first enemy slot that has a model and is out of the world or finished, or -1. EN 0x5eb4b0.
extern "C" int Enemy_FindReusableSlot(void);
// Actor scans (actor_scan.cpp).
extern "C" int Enemy_ClearIdleHealth(void);                                       // EN 0x6e1690
extern "C" int Object_IsNearPoint(int unused, const Vec4 *point);                 // EN 0x5c1b20
extern "C" int Char_IsPartyNear(int index, const Vec4 *offset, int range);        // EN 0x5c0bd0
extern "C" int Object_SlowNearBlocker(const Vec4 *position, Vec4 *velocity);      // EN 0x649c00
extern "C" int Char_IsStandingOn(int lower, int upper, int maxHeight, int maxDistanceSquared);   // EN 0x5c1510
// Behaviour pickers (ai_pick.cpp): set the state and the first two values of the slot's CharState.
extern "C" int EnemyAI_PickSingle(int index);        // EN 0x5b9ae0: always behaviour 0
extern "C" int Enemy_PickAction_65_35(int index);    // EN 0x5b9a80: behaviour 0 65 %, behaviour 1 35 %

#endif
