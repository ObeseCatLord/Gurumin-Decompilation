// Globals of per-character tables, indexed by character slot (0..CHAR_SLOT_COUNT-1).
// Generated from the per-file extern declarations by tools/globals_driver.py; this is the one
// declaration of each of them: include this header, never redeclare one. Comments say what is known.
#ifndef GURUMIN_CHAR_TABLES_H
#define GURUMIN_CHAR_TABLES_H

// Number of character slots (also defined by char_state.h and char_slot.h).
#ifndef CHAR_SLOT_COUNT
#define CHAR_SLOT_COUNT 0x80
#endif

#include "matrix4.h"
#include "vec4.h"

struct Node;   // node.h

extern "C" short DAT_1020b128[CHAR_SLOT_COUNT];           // switch animation frame
extern "C" int DAT_1020b230[CHAR_SLOT_COUNT];             // kind variant (the kind's ten-thousands)
extern "C" char DAT_1020d540[CHAR_SLOT_COUNT];            // 1 = the character is in play (char_retire.cpp clears it); per zone character: also hurt enemies that are away
extern "C" int DAT_1020e138[CHAR_SLOT_COUNT];
extern "C" int DAT_10210b30[CHAR_SLOT_COUNT];             // non-zero = told to react
extern "C" int DAT_10218fc0[CHAR_SLOT_COUNT];
extern "C" short DAT_102191c0[CHAR_SLOT_COUNT];
extern "C" int DAT_10219770[CHAR_SLOT_COUNT];             // effect handle of each character slot
extern "C" char DAT_10219e40[CHAR_SLOT_COUNT];
extern "C" short DAT_10219ec0[CHAR_SLOT_COUNT];           // gimmick id controlled by the switch
extern "C" short DAT_1021b3c8[CHAR_SLOT_COUNT];
extern "C" char DAT_1021b4c8[CHAR_SLOT_COUNT];
extern "C" int DAT_1021d138[CHAR_SLOT_COUNT];             // the prompt: cell (0..15) + 1000 or 2000 for the other sheets; the prompt shown; key prompt id
extern "C" char DAT_1021d338[CHAR_SLOT_COUNT][0x40];      // per character: 1 for each cell box within 75 units
extern "C" char DAT_1021f340[CHAR_SLOT_COUNT];
extern "C" char DAT_1021f3c0[CHAR_SLOT_COUNT];            // slot kind of each character (',' = free marker, 'Q' = taken marker); slot kind (0x13 / 0x14 = the two switch kinds); kind of each character slot ('B' = balloon); slot kind (3 = enemy)
extern "C" int DAT_1021f440[CHAR_SLOT_COUNT];
extern "C" char DAT_1021f6c0[CHAR_SLOT_COUNT];
extern "C" int DAT_1021f740[CHAR_SLOT_COUNT];             // frames the prompt stays; key prompt frames left
extern "C" char DAT_1021f940[CHAR_SLOT_COUNT];
extern "C" char DAT_1021f9c0[CHAR_SLOT_COUNT];
extern "C" char DAT_102239d0[CHAR_SLOT_COUNT];
extern "C" int DAT_10253a58[CHAR_SLOT_COUNT];
extern "C" int DAT_10253c58[CHAR_SLOT_COUNT];
extern "C" char DAT_10253e98[CHAR_SLOT_COUNT];
extern "C" char DAT_10254090[CHAR_SLOT_COUNT];
extern "C" int DAT_10254110[CHAR_SLOT_COUNT];
extern "C" int DAT_10254390[CHAR_SLOT_COUNT];             // greeting countdown per character (10000 = greet now)
extern "C" int DAT_10254598[CHAR_SLOT_COUNT];
extern "C" short DAT_10254838[CHAR_SLOT_COUNT];           // per-object switch state; 1000 is added while it is held
extern "C" int DAT_102548c0[CHAR_SLOT_COUNT];             // per-character packed value, four two-digit fields AABBCCDD; script code of each character (pairs of decimal digits)
extern "C" unsigned char DAT_102550e0[CHAR_SLOT_COUNT];   // non-zero = wake at once (the character's level elsewhere); 1 = the character was hit (attack at once); level of each character
extern "C" char DAT_10255a88[CHAR_SLOT_COUNT];
extern "C" int DAT_10256218[CHAR_SLOT_COUNT];
extern "C" char DAT_10256420[CHAR_SLOT_COUNT];
extern "C" int DAT_103084a8[CHAR_SLOT_COUNT];             // number of words in each group
extern "C" int DAT_103086b0[CHAR_SLOT_COUNT];
extern "C" int DAT_1030b668[CHAR_SLOT_COUNT];             // base kind of each character (-1 or 0 = none)
extern "C" int DAT_1030b8f8[CHAR_SLOT_COUNT];
extern "C" int DAT_1030c278[CHAR_SLOT_COUNT];             // frames until the next face change (0 = none)
extern "C" char DAT_1030ce38[CHAR_SLOT_COUNT];
extern "C" unsigned char DAT_1030ceb8[CHAR_SLOT_COUNT][0x100];  // replacement chosen for each word (0xff = not yet)
extern "C" int DAT_10314ec0[CHAR_SLOT_COUNT];
extern "C" int DAT_10315140[CHAR_SLOT_COUNT];
extern "C" short DAT_103153b0[CHAR_SLOT_COUNT];           // non-zero = release now
extern "C" short DAT_1032d4b8[CHAR_SLOT_COUNT];           // script flag set by the switch (0 = none)
extern "C" char DAT_1032d870[CHAR_SLOT_COUNT][0x100][0x20];  // the words
extern "C" char DAT_1042d9e8[CHAR_SLOT_COUNT];            // set when the character was hit this frame
extern "C" int DAT_1042dba8[CHAR_SLOT_COUNT];
extern "C" int DAT_10447dd8[CHAR_SLOT_COUNT];             // face to show next
extern "C" int DAT_104483e8[CHAR_SLOT_COUNT];
extern "C" int DAT_104485e8[CHAR_SLOT_COUNT];
extern "C" char DAT_104487e8[CHAR_SLOT_COUNT][0x100];     // length of each word
extern "C" int DAT_10453008[CHAR_SLOT_COUNT];
extern "C" int DAT_10467288[CHAR_SLOT_COUNT];             // each character's debris colour
extern "C" char DAT_10467920[CHAR_SLOT_COUNT];            // 1 = no ground found below the slot
extern "C" int DAT_104679a0[CHAR_SLOT_COUNT];
extern "C" char DAT_10468730[CHAR_SLOT_COUNT];
extern "C" int DAT_104bf8f8[CHAR_SLOT_COUNT];             // 1 while a key prompt is shown over the character; key prompt shown per character (app_0008.cpp)
extern "C" Vec4 DAT_104c4438[CHAR_SLOT_COUNT];            // where a wave enemy came in
extern "C" Vec4 DAT_10836fc8[CHAR_SLOT_COUNT];
extern "C" Vec4 DAT_10837fc8[CHAR_SLOT_COUNT];
extern "C" Vec4 DAT_10864cf8[CHAR_SLOT_COUNT];            // ground point found below each slot
extern "C" Matrix4 DAT_10868838[CHAR_SLOT_COUNT][6];      // character 0: [0] = the matrix the sparkle follows; saved node matrices of each character
extern "C" Vec4 DAT_10f08080[CHAR_SLOT_COUNT];            // each character's offset from the reference position
extern "C" Node *DAT_11b002d0[CHAR_SLOT_COUNT];           // the cached models
extern "C" char DAT_11b004d0[CHAR_SLOT_COUNT][0x80];      // their file names

#endif
