// Globals of the map setup from a map table entry (src/map_setup_entry.cpp, 0x100fda40) and its sibling the field map
// setup (0x100fc4a0). This is the one declaration of these globals: include this header, never redeclare them.
#ifndef GURUMIN_MAP_SETUP_H
#define GURUMIN_MAP_SETUP_H

#include "vec4.h"

struct Node;   // node.h

extern "C" Node DAT_104c4c38[];                       // the loaded map models (0x18a0 bytes each): the map and its second model
extern "C" Node *DAT_1020af20[0x40];                  // the "@kskNN_" nodes (0 = none)
extern "C" Node *DAT_1021fa40[0x40];                  // the body of each "kskNNa" node (its first child when it has children)
extern "C" char DAT_10253e58[0x40];                   // 1 when the "@kskNN_" node name has a 'K' after its prefix
extern "C" short DAT_1021d0b8[0x40];                  // the number in the "@kskNN_" node name
extern "C" short DAT_1042da88[0x40];                  // random 10..15 per "@kskNN_" node
extern "C" char DAT_10255a48[0x40];                   // 4 for each found "kskNNa" body
extern "C" char DAT_10219970[0x40];                   // 0 for each found "kskNNa" body
extern "C" Node *DAT_10210f38[0x20];                  // the "sakana01" fish nodes
extern "C" char DAT_1021a1c0[0x20];                   // 0 for each fish
extern "C" Node *DAT_1049f0f8[0x40];                  // the "@firelight" and "@CandleNull" nodes
extern "C" short DAT_1021b3c0;                        // number of nodes in DAT_1049f0f8
extern "C" Node *DAT_104c0240;                        // the "@BurankoRoot" node (0 unless it has both "@b1" and "@b2")
extern "C" Node *DAT_104c0244;                        // its "@b1" node
extern "C" Node *DAT_104c0248;                        // its "@b2" node
extern "C" Node *DAT_104c0268;                        // the "swan" node (0 unless it has a "swanat" node)
extern "C" Node *DAT_104c0264;                        // its "swanat" node
extern "C" int DAT_1020aeec[4];                       // cleared with the four "fannel" nodes
extern "C" int DAT_1020d64c[4];
extern "C" int DAT_1020b1e0[0x10];
extern "C" int DAT_10219f78[0x10];
extern "C" int DAT_104507e8[6];
extern "C" int DAT_10467718[0x20];
extern "C" int DAT_104678a0[0x20];
extern "C" int DAT_10c62ee0[0x1c1];
extern "C" float DAT_102027ec;                        // 1.0 after a setup
extern "C" int DAT_102027f8;                          // -1 after a setup
extern "C" int DAT_1021a368;
extern "C" int DAT_10256210;
extern "C" int DAT_1042d958;
extern "C" int DAT_104bf87c;
extern "C" int DAT_1032d678;
extern "C" int DAT_104c0144;
extern "C" int DAT_104c01d0;
extern "C" char DAT_104c01f4;
extern "C" char DAT_104c023d;
extern "C" char DAT_104c025c;
extern "C" char DAT_104c025d;
extern "C" int DAT_104c0260;
extern "C" int DAT_104c02b8;
extern "C" short DAT_104c02e4;
extern "C" char DAT_104c02ed;
extern "C" char DAT_10f65554;                         // 2 after a setup
extern "C" int DAT_1101d870;                          // 1 when voices are unlimited, else the voice setting DAT_104c0332
extern "C" int DAT_1101de30;
extern "C" int DAT_1065cb9c;
extern "C" int DAT_1065cba4;
extern "C" Vec4 DAT_10866de8;
extern "C" Vec4 DAT_10f65848[2];                     // two random offsets: x and y in -1499..-500, z = w = 0

#endif
