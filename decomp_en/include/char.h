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
// CharState::values indices an idle NPC uses: frames to wait before it may speak again, and its talk line (key script * 100 + line).
const int CHARSTATE_NPC_WAIT = 0;
const int CHARSTATE_NPC_TALK = 1;
const int CHARSTATE_NPC_RANGE = 2;       // distance at which a waiting NPC reacts to the player
// CharState::values index of the flag a route walker keeps: non-zero = walking the route backwards.
const int CHARSTATE_ROUTE_REVERSED = 4;
// Characters in the party table (g_partyCharacters).
const int PARTY_SIZE = 5;
// Number of entries of the group table.
const int CHAR_GROUP_COUNT = 0x5c;
// Bytes of a CharRecord.
const int CHAR_RECORD_SIZE = 0x188;

// Characters in a character's display name, including the terminator.
const int CHAR_NAME_SIZE = 0x20;
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
    STAT_MAX_HEALTH = 1,      // 0x08: initial value of the health (restored when the slot is reused)
    STAT_EFFECT_COLOUR = 3,   // 0x10: colour of the effects the character starts
    STAT_VALUE_4 = 4,         // 0x14: saved with the colour when Char_UnpackParameter switches the kind
    STAT_PICKUPS = 6,         // 0x1c: guarded by the anti-tamper copies
    STAT_GUARDED_B = 7        // 0x20: guarded by the anti-tamper copies
};

// Entries of CharRecord::params.
enum CharParam
{
    PARAM_MODEL = 3           // 0xfc: model definition of the character (index into g_modelPaths)
};

// Four floats with a copy assignment of its own (EN 0x76f1d0, the same code as Vec4::Copy).
struct ParamVector
{
    float x;    // x component
    float y;    // y component
    float z;    // z component
    float w;    // w component
    ParamVector &operator=(const ParamVector &other);   // EN 0x76f1d0
};

// A character slot record (0x188 bytes). Layout and names follow the JP CharSlot (decomp/include/char_slot.h).
struct CharRecord
{
    int m_0000;                              // 0x00: not used by EN code yet
    int stats[CHAR_STAT_COUNT];              // 0x04: health and other values by CharStat index
    int m_0044[CHAR_STAT_COUNT];             // 0x44: not used by EN code yet
    int m_0084;                              // 0x84: 1 = released by its group
    int m_0088;                              // 0x88: not used by EN code yet
    int m_008c;                              // 0x8c: not used by EN code yet
    int m_0090;                              // 0x90: not used by EN code yet
    int faceSet;                             // 0x94: face set of the character, -1 = no face animation
    int m_0098;                              // 0x98: 0x1d while held
    int kind;                                // 0x9c: behaviour kind
    int sharesModel;                         // 0xa0: non-zero = shares the previous slot's model; the actor scans only count slots where it is 0
    int m_00a4;                              // 0xa4: not used by EN code yet
    int m_00a8;                              // 0xa8: tested on the previous slot by Chars_ReloadModels (0 = reload this slot)
    int m_00ac;                              // 0xac: not used by EN code yet
    int routeRecord;                         // 0xb0: the map record whose route the character walks
    int m_00b4;                              // 0xb4: not used by EN code yet
    int money;                               // 0xb8: money carried (guarded by the anti-tamper copies)
    int m_00bc;                              // 0xbc: not used by EN code yet
    int m_00c0;                              // 0xc0: not used by EN code yet
    int m_00c4;                              // 0xc4: not used by EN code yet
    int m_00c8;                              // 0xc8: not used by EN code yet
    int helmet;                              // 0xcc: the helmet item worn (slot 0)
    int costume;                             // 0xd0: the player's costume (slot 0)
    int twoHanded;                           // 0xd4: set for the two-handed weapons
    int packedParameter;                     // 0xd8: packed script value (formation digits, mode digits; see Char_UnpackParameter)
    int m_00dc;                              // 0xdc: not used by EN code yet
    int m_00e0;                              // 0xe0: not used by EN code yet
    int m_00e4;                              // 0xe4: not used by EN code yet
    int m_00e8;                              // 0xe8: not used by EN code yet
    float speedScale;                        // 0xec: motion speed scale given to the model when it is loaded
    int params[CHAR_PARAM_COUNT];            // 0xf0: by CharParam index
    ParamVector position;                    // 0x130: position
    char switchesHeld;                       // 0x140: number of switches the character is holding
    char defeated;                           // 0x141: 1 once the character is marked defeated
    char m_0142;                             // 0x142: not used by EN code yet
    char colourByte;                         // 0x143: colour value Char_UnpackParameter saves before it switches the kind
    char heldFlag;                           // 0x144: kept in the CharState while the character is held
    char name[CHAR_NAME_SIZE];               // 0x145: display name (message boxes show it as the speaker)
    char m_0165[CHAR_NAME_SIZE];             // 0x165: not used by EN code yet
};

// A shove: a timed step applied to a character or a look target (the shoves view of a CharState, 0x10 bytes).
struct Shove
{
    int target;     // 0x00: character to move (the digits above the lowest four are a flag); negative = a head look target
    int stepX;      // 0x04: x added to the character each frame (for a look target: the character to look at)
    int stepY;      // 0x08: y added to the character each frame
    int frames;     // 0x0c: frames left (the digits above the lowest four are a flag); 0 = idle
};

// Shoves a slot runs at once.
const int SHOVE_COUNT = 4;

// The behaviour state of a character slot (0x40 bytes); every behaviour keeps its own variables in `values`. The slot of a
// scripted scene uses the same memory as SHOVE_COUNT shoves instead.
struct CharState
{
    union
    {
        struct
        {
            int state;                // 0x00: behaviour step (0 = not started yet)
            int values[13];           // 0x04: behaviour variables (the AI pickers use values[0] and values[1])
            int sawPlayer;            // 0x38: 1 once the character saw the player (or was told to react)
            int behaviour;            // 0x3c: behaviour number the character switches to (set with state 2)
        };
        Shove shoves[SHOVE_COUNT];    // 0x00: the shoves of the slot
        int words[CHAR_STATE_SIZE / sizeof(int)];   // 0x00: the whole state as raw words (clear loops)
    };
};

// The model file of a model number.
struct ModelPath
{
    char path[0x4b34];        // 0x0000: path of the model file
};

extern "C" CharRecord g_charRecords[CHAR_SLOT_COUNT];      // EN 0x00f381e0 (JP DAT_10daaae8)
extern "C" CharState g_charState[CHAR_SLOT_COUNT];         // EN 0x009160e0 (JP DAT_1020b430)
// Runs the shoves of slot `slot`: look targets turn the player's head, running shoves move their character and play its run or
// idle motion. Returns 0. EN 0x63aeb0 (JP 0x1002ee70).
extern "C" int Char_UpdateShoves(int slot);
extern "C" char g_charRespawned[CHAR_SLOT_COUNT];         // EN 0x009e1810 (JP DAT_1021f9c0): 1 once an enemy dropped in or was defeated and refilled
extern "C" unsigned char g_charHitLevel[CHAR_SLOT_COUNT];   // EN 0x009e1a10 (JP DAT_102550e0): non-zero = wake at once (1 = hit, attack at once)
extern "C" short g_charReleaseNow[CHAR_SLOT_COUNT];         // EN 0x009e2278 (JP DAT_103153b0): non-zero = release now (set to 150 for drop-ins)
extern "C" int g_charGroupTimer[CHAR_SLOT_COUNT];       // EN 0x0091f618 (JP DAT_102199b8): set to 2 when an NPC notices the player
extern "C" int g_charToldToReact[CHAR_SLOT_COUNT];         // EN 0x00920260 (JP DAT_10210b30): non-zero = told to react
extern "C" char g_charTalkPending[CHAR_SLOT_COUNT];        // EN 0x009e1890 (JP DAT_10255a88): cleared while an NPC idles
extern "C" int g_charPhase[CHAR_SLOT_COUNT];               // EN 0x00915cb8 (JP DAT_10210d30): non-zero = busy; 2 = finished
// Squared distance of each character slot to the camera target, filled before the draw order is sorted. EN 0x01664218.
extern "C" float g_charCameraDistance[CHAR_SLOT_COUNT];
// qsort comparator over slot numbers: orders them by g_charCameraDistance, nearest first (1 when `a` is at least as far as
// `b`, else -1). EN 0x71bf70.
extern "C" int CharSlot_CompareByDistance(const int *a, const int *b);
// qsort comparator over slot numbers: orders them by depth, smallest sortBias + z position first (-1 when `a` has the
// smaller depth, else 1). EN 0x5c94c0 (see object_lookup.cpp).
extern "C" int CharSlot_CompareByDepth(const int *a, const int *b);
// g_charKind letters of the object slots the scripts can look up by model id (the meaning of the letters is not recovered).
const char OBJECT_KIND_LETTER_F = 'F';
const char OBJECT_KIND_LETTER_M = 'M';
const char OBJECT_KIND_LETTER_N = 'N';
// Returned by Object_FindByModelId when no object has the id.
const int OBJECT_NOT_FOUND = -1;
// Slot of the first object (slots OBJECT_SLOT_FIRST up, kind letter F, M or N) whose model has id `objectId`, or
// OBJECT_NOT_FOUND. Called by the script commands that name an object. EN 0x602dc0 (see object_lookup.cpp).
extern "C" int Object_FindByModelId(int objectId);
// Marks character `index` defeated and switches it to behaviour 1 unless it is busy; returns 1 when busy. EN 0x6de920.
extern "C" int Char_MarkDefeatedAndSwitch(int index);
extern "C" char g_charKind[CHAR_SLOT_COUNT];               // EN 0x00a35bc8 (JP DAT_1021f3c0): kind of each slot (3 = enemy)
extern "C" int g_charGroup[CHAR_GROUP_COUNT];              // EN 0x0091f818 (JP DAT_10255f10): group of each character, -1 = none
extern "C" unsigned char g_charSpawnFlag[CHAR_SLOT_COUNT];   // EN 0x009e1bd0 (JP DAT_10315350): a per-character flag, cleared when a slot is reused
// Saved looks of characters that Char_UnpackParameter switched to kind CHAR_KIND_PACKED (restored later).
extern "C" char g_charKindSwitched[CHAR_SLOT_COUNT];        // EN 0x0091ddb8 (JP DAT_1030ce38): 1 once the character's kind was switched
extern "C" int g_charBaseKind[CHAR_SLOT_COUNT];             // EN 0x0091deb8 (JP DAT_1030b668): kind before the switch (-1 = kind 0)
extern "C" int g_charSavedColourByte[CHAR_SLOT_COUNT];      // EN 0x0091e2b8 (JP DAT_104679a0): CharRecord::colourByte before the switch
extern "C" int g_charSavedEffectColour[CHAR_SLOT_COUNT];    // EN 0x0091e970 (JP DAT_10314ec0): stats[STAT_EFFECT_COLOUR] before the switch
extern "C" int g_charSavedStat4[CHAR_SLOT_COUNT];           // EN 0x0091ed70 (JP DAT_1021f440): stats[STAT_VALUE_4] before the switch
// g_charRecords kind a character takes while its packed parameter holds a non-zero mode (0x62).
const int CHAR_KIND_PACKED = 0x62;
// Unpacks CharRecord::packedParameter of character `index` into its state (formation, behaviour). EN 0x6d1fb0 (JP 0x1001b5b0).
extern "C" int Char_UnpackParameter(int index);
// Advances character `index`'s pulse step and sets its pose node to the matching scale. EN 0x665a80 (JP 0x1001a390).
extern "C" int Char_PulseScale(int index);
// Plays the idle motion of character `index` and resets its state when the player comes close: see char_proximity.h.
extern "C" const char g_partyCharacters[PARTY_SIZE];        // EN 0x00848850 (JP DAT_101d67f0): slots 0, 13, 15, 58, 20
extern "C" Matrix4 g_characterNodeMatrices[CHAR_SLOT_COUNT][CHAR_NODE_MATRIX_COUNT];   // EN 0x015ad660 (JP DAT_10868838): saved node matrices of each character; [0][0] of character 0 is the matrix the item stand sparkle follows
extern "C" int g_charEffectHandle[CHAR_SLOT_COUNT];        // EN 0x009e2078 (JP DAT_10219770): effect handle of each slot
// Nodes attached to each character (index 0 is the pose / stand parent / wheel node). g_charNodes[0][0] is g_itemStandParent.
const int CHAR_NODE_COUNT = 9;
extern "C" Node *g_charNodes[CHAR_SLOT_COUNT][CHAR_NODE_COUNT];   // EN 0x009147e0 (JP DAT_1021bdf8): attached nodes of each character
extern "C" AnimModel g_characterModels[CHAR_SLOT_COUNT];   // EN 0x01487ff0 (JP DAT_10db6ee8): the model of each slot
extern "C" int g_alwaysPlaceOnGround;   // EN 0x00908388 (JP DAT_102022b8): non-zero = Char_PlaceOnGround always runs

// Casts down from 1000 units back along the light direction of character `index` and moves its shadow matrix to the
// ground point (or to the bottom of the world when nothing is hit). Runs when g_alwaysPlaceOnGround or `force` is set. Returns 0. EN 0x645770.
extern "C" int Char_PlaceOnGround(int index, int force);
extern "C" ModelPath g_modelPaths[];                       // EN 0x01201270 (JP DAT_10b314c8): the model file of each model number
// Bytes of one entry of g_leftModelPaths.
const int LEFT_MODEL_PATH_SIZE = 0x124;
extern "C" char g_leftModelPaths[][LEFT_MODEL_PATH_SIZE];   // EN 0x00f64440 (JP DAT_10894698): model files an enemy leaves behind when destroyed (AnimModel::leftModel)
extern "C" int g_sceneTimer;                              // EN 0x009e33a0 (JP DAT_104c00d0): scene timer
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
// Respawns an enemy in a reusable slot and places it at (x, y, z); returns the slot or -1. EN 0x642cd0 (JP 0x10011d40).
extern "C" int Enemy_Respawn(float x, float y, float z);
// Actor scans (actor_scan.cpp).
extern "C" int Enemy_WatchPlayer(int index);                                      // EN 0x5be510 (JP 0x10027400): faces the player until a random end
extern "C" int Enemy_ClearIdleHealth(void);                                       // EN 0x6e1690
extern "C" int Object_IsNearPoint(int unused, const Vec4 *point);                 // EN 0x5c1b20
extern "C" int Char_IsPartyNear(int index, const Vec4 *offset, int range);        // EN 0x5c0bd0
extern "C" int Object_SlowNearBlocker(const Vec4 *position, Vec4 *velocity);      // EN 0x649c00
extern "C" int Char_IsStandingOn(int lower, int upper, int maxHeight, int maxDistanceSquared);   // EN 0x5c1510
// Behaviour pickers (ai_pick.cpp): set the state and the first two values of the slot's CharState.
extern "C" int EnemyAI_PickSingle(int index);        // EN 0x5b9ae0: always behaviour 0
extern "C" int Enemy_PickAction_65_35(int index);    // EN 0x5b9a80: behaviour 0 65 %, behaviour 1 35 %

// Wave enemies (char_retire.cpp).
const int WAVE_GRID_SIZE = 10;                      // the wave grid is 10 rows by 10 columns (formation number = row * 10 + column)
const float CHAR_RETIRE_OFFSET = -100000.0f;        // a retired character's x becomes this minus its x (moves it far out of the world)
const float CHAR_MARKER_HIDDEN = -200000.0f;        // marker y of a character that is not placed
// The marker record of a placed character (0x10 bytes; only y is used so far).
struct CharMarker
{
    float y;           // 0x00: height of the marker (CHAR_MARKER_HIDDEN = hidden)
    char m_0004[0xc];  // 0x04: not used by EN code yet
};
extern "C" char g_charInPlay[CHAR_SLOT_COUNT];                       // EN 0x0091d608 (JP DAT_1020d540): 1 = the character is in play
extern "C" int g_waveEnemiesAlive;                                    // EN 0x01333350 (JP DAT_10c635a8): wave enemies still alive
extern "C" char g_waveGrid[WAVE_GRID_SIZE][WAVE_GRID_SIZE];           // EN 0x0091e740 (JP DAT_1049efb0): character in each wave slot (0 = empty)
extern "C" CharMarker g_charMarkers[CHAR_SLOT_COUNT];                 // EN 0x0147b258 (JP DAT_10867e18): where a character in play was placed
extern "C" int g_wakeRadiusShort;                                     // EN 0x013364b8 (JP DAT_10c66710): non-zero = Enemies_WakeNearby uses a radius of 300
// Retires a wave enemy: moves it out of the world and records it in the wave grid. EN 0x6d64e0 (JP 0x1001b7b0).
extern "C" int Char_RetireWaveEnemy(int index);

#endif
