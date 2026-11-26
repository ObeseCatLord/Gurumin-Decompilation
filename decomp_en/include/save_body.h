// The layout of a save file (gdataNN.sav) of the EN build (JP: decomp/docs/SAVE_FORMAT.md). This is the one declaration
// of the save layout in decomp_en.
#ifndef GURUMIN_EN_SAVE_BODY_H
#define GURUMIN_EN_SAVE_BODY_H

#include "vec4.h"

// Bytes of the thumbnail at the start of a save body (256 x 256 pixels, three bytes each).
const int SAVE_THUMBNAIL_BYTES = 0x30000;
// Bytes of the character record a save keeps for the player, and of the script variables.
const int SAVE_PLAYER_BYTES = 0x188;
const int SAVE_SCRIPT_BYTES = 0x4000;
// Bytes of the player's name in the character record.
const int SAVE_PLAYER_NAME_SIZE = 0x20;
// Words of the character record before the position.
const int SAVE_PLAYER_WORDS_BEFORE_POSITION = 0x4c;
// Bytes of the save body between the player record and the script variables, and after the variables up to the play time.
const int SAVE_BYTES_BEFORE_PLAYER = 0x154;
const int SAVE_BYTES_AFTER_SCRIPT = 0x80;

// The player's character record as a save keeps it (the bytes of CharRecord, slot 0, copied as they are).
struct SavedPlayer
{
    int words[SAVE_PLAYER_WORDS_BEFORE_POSITION];    // 0x000: stats, parameters and the values in front of the position
    Vec4 position;                                   // 0x130: where the player stands
    char flags[5];                                   // 0x140: switches held, defeated, unused, colour byte, held flag
    char name[SAVE_PLAYER_NAME_SIZE];                // 0x145: the player's name
    char unused[SAVE_PLAYER_NAME_SIZE];              // 0x165: not used
};

// Indices of the script variables (ints) that the slot summary reads.
const int SAVE_VAR_CHAPTER = 0x1c7;
const int SAVE_VAR_COSTUME = 0x9c5;
const int SAVE_VAR_MODE = 0xf79;
// Costume number that renames the player.
const int SAVE_COSTUME_POPON = 6;

// A save body, the bytes after the 8-byte file header (descrambled).
struct SaveBody
{
    unsigned char thumbnail[SAVE_THUMBNAIL_BYTES];   // 0x00000: the picture of the save menu
    int map;                                         // 0x30000: the map the player is in
    int x;                                           // 0x30004: where the player stands (a float, copied as raw bits)
    int y;                                           // 0x30008
    int z;                                           // 0x3000c
    int angle;                                       // 0x30010: not read by the loader
    int m_0014;                                      // 0x30014: a value Save_Load copies to a global
    unsigned char m_0018[SAVE_BYTES_BEFORE_PLAYER];  // 0x30018: not mapped yet
    SavedPlayer player;                              // 0x3016c: the player's character record
    int scriptVariables[SAVE_SCRIPT_BYTES / sizeof(int)];   // 0x302f4: the script variables
    unsigned char m_342f4[SAVE_BYTES_AFTER_SCRIPT];  // 0x342f4: a block Save_Load copies to a global
    int playTime;                                    // 0x34374: the play time
};

// The trailer of a scrambled save file, after the body.
struct SaveTrailer
{
    int offset;                // the offset the loader adds (not used by the summary)
    int sum;                   // byte sum of the plain body
    unsigned int check;        // xor of the plain body
};

#endif
