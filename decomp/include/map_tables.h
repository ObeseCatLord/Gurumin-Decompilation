// Tables of bin\map.itm (the map data file, Game::LoadMapData): the record layouts of the Game object's script tail.
// This is the one declaration of these types: include this header, never redeclare them.
#ifndef GURUMIN_MAP_TABLES_H
#define GURUMIN_MAP_TABLES_H

// Longest names the character table holds.
#define MAP_CHARACTER_NAME_LENGTH 0x40
#define MAP_CHARACTER_MODEL_NODE_LENGTH 0x20

// Strings of a table record are 16 characters including the terminator.
#define MAP_LABEL_SIZE 0x10

// One 0x174-byte record of the character table of map.txt (DAT_10918e60, count DAT_109276e0). Game_LoadData copies
// its fields into the slot (CharSlot) and the per-slot tables with the record's id as index.
struct MapCharacter
{
    int id;                                   // 0x00: slot the record fills
    char name[MAP_CHARACTER_NAME_LENGTH];     // 0x04: display name
    char label[MAP_CHARACTER_NAME_LENGTH];    // 0x44: copied to CharSlot::m_0165
    int kind;                                 // 0x84: CharSlot::kind
    int speedPercent;                         // 0x88: motion speed in percent (CharSlot::speedScale = this / 100)
    int m_008c;                               // 0x8c: CharSlot::m_00a0
    int m_0090;
    int faceSet;                              // 0x94: CharSlot::faceSet
    union
    {
        struct
        {
            char slotKind;                    // 0x98: DAT_1021f3c0
            char pad_0099[3];
            int params[4];                    // 0x9c: CharSlot::params
            int m_00ac;
            int m_00b0;                       // 0xb0: DAT_102192c0
            int m_00b4;                       // 0xb4: DAT_102108e8
        };
        int values[8];                        // 0x98: the eight numbers of the record's second half
    };
    union
    {
        struct
        {
            char pad_00b8[0x5c];
            char modelNode[MAP_CHARACTER_MODEL_NODE_LENGTH];  // 0x114: DAT_1021a3c0
            char pad_0134[0x40];
        };
        struct
        {
            char pad_00b8_labels[0xc];
            char labels[11][MAP_LABEL_SIZE];  // 0xc4: eleven strings, each read before its number
        };
    };
};

// Battle script step (0x38 bytes): fourteen numbers of a "#BattleSc2" line.
struct MapBattleEntry
{
    int values[14];
};

// Music table entry (0x30 bytes): track name, file name and four numbers.
struct MapBgmEntry
{
    char name[MAP_LABEL_SIZE];
    char file[MAP_LABEL_SIZE];
    int values[4];                            // 0x20
};

// Face frame range (4 bytes): "#FaceFrameTable" pairs. An unused entry holds MAP_FACE_FRAME_NONE.
#define MAP_FACE_FRAME_NONE (-3000)
struct MapFaceFrame
{
    short first;
    short second;
};

// Sound effect entry (0x40 bytes).
struct MapSeEntry
{
    char name[0x20];
    int values[4];                            // 0x20
    char pad_0030[0x10];
};

// Battle motion entry (0x124 bytes): "#BtMotTable".
struct MapBtMotEntry
{
    char name[0x40];
    int values[7];                            // 0x40
    char pad_005c[4];
    char labels[7][MAP_LABEL_SIZE];           // 0x60: the string read before each number
    char pad_00d0[0x54];
};

// Ability entry (0x240 bytes): "#AbTable".
struct MapAbilityEntry
{
    char name[0x20];
    char text[0x40];                          // 0x20
    int values[15];                           // 0x60
    char pad_009c[0x24];
    char labels[24][MAP_LABEL_SIZE];          // 0xc0: fifteen read with the numbers, checked for overflow up to 24
};

// Entry of the "#USDG" table (0x20 bytes): five numbers.
struct MapUsdgEntry
{
    int values[5];
    char pad_0014[0xc];
};

// Character status entry (0xc0 bytes): "#StatusTable".
struct MapStatusEntry
{
    char name[0x20];
    char text[0x40];                          // 0x20
    int values[17];                           // 0x60
    char pad_00a4[0x1c];
};

// Item entry (0xc0 bytes): "#ItemTable".
struct MapItemEntry
{
    char name[0x20];
    char text[0x40];                          // 0x20
    int values[22];                           // 0x60
    char pad_00b8[8];
};

// Entry of "#CharPos" (0xf4 bytes).
struct MapCharPos
{
    int id;
    int m_0004;
    int m_0008;
    int m_000c;
    int m_0010;
    int m_0014;
    int values[7];                            // 0x18
    char pad_0034[0x10];
    char labels[11][MAP_LABEL_SIZE];          // 0x44
};

// Entry of "#CharMonsPos" (0xf4 bytes).
struct MapMonsterPos
{
    int id;
    int m_0004;
    int m_0008;
    int m_000c;
    int m_0010;
    int m_0014;
    int values[11];                           // 0x18
    char labels[11][MAP_LABEL_SIZE];          // 0x44
};

// Entry of "#ObjAnimeTable" (0xd0 bytes).
struct MapObjAnime
{
    int id;
    char name[0x20];                          // 0x04
    int m_0024;
    int m_0028;
    int m_002c;
    int values[2];                            // 0x30
    char pad_0038[0x18];
    char labels[8][MAP_LABEL_SIZE];           // 0x50
};

// Number of key lines of a "#CharMotTable" section.
#define MAP_MOTION_KEY_COUNT 400

// One line of a character motion: six numbers (0x20 bytes with the two that are never read).
struct MapMotionKey
{
    int values[6];
    int pad_0018[2];
};

// "#CharMotTable:n" section (0x4b34 bytes): a name and its key lines.
struct MapCharMotion
{
    int count;                                // lines read
    char name[0x30];                          // 0x04
    char labels[MAP_MOTION_KEY_COUNT][MAP_LABEL_SIZE];    // 0x34
    MapMotionKey keys[MAP_MOTION_KEY_COUNT];              // 0x1934
};

#endif
