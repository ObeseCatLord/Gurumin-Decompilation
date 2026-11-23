// Globals of game progress, saves, settings and other game-wide state.
// Generated from the per-file extern declarations by tools/globals_driver.py; this is the one
// declaration of each of them: include this header, never redeclare one. Comments say what is known.
#ifndef GURUMIN_GAME_VARS_H
#define GURUMIN_GAME_VARS_H

#include "app_0008.h"
#include "game_reset.h"
#include "matrix4.h"
#include "vec4.h"

struct Node;   // node.h

extern "C" int DAT_10202348;
extern "C" float DAT_1020234c;
extern "C" int DAT_10202350;
extern "C" int DAT_10202354;
extern "C" int DAT_1020235c;
extern "C" int DAT_1020245c;
extern "C" float DAT_10202460;
extern "C" int DAT_10202468;
extern "C" int DAT_1020246c;
extern "C" int DAT_10202470;
extern "C" int DAT_10202474;
extern "C" int DAT_10202480;
extern "C" int DAT_10202484;
extern "C" int DAT_10202488;
extern "C" int DAT_1020248c;
extern "C" int DAT_10202490;
extern "C" int DAT_10202494;
extern "C" int DAT_10202498;
extern "C" int DAT_1020249c;
extern "C" float DAT_102024a4;
extern "C" int DAT_102024a8;
extern "C" int DAT_102024ac;
extern "C" int DAT_102024b0;
extern "C" char DAT_102024b4;
extern "C" short DAT_102024b8;
extern "C" short DAT_102024bc;
extern "C" int DAT_102024c0;
extern "C" char DAT_102024c4;
extern "C" int DAT_102024c8;
extern "C" float DAT_102027e4;
extern "C" float DAT_102027e8;
extern "C" char DAT_10202800;
extern "C" int DAT_10202808[0x10];                        // texture slots of the HUD (released here)
extern "C" int DAT_1020aee8;
extern "C" int DAT_1020b22c;
extern "C" int DAT_1020d430;
extern "C" char DAT_1020d438[0x80];                       // per character slot: non-zero = drawn without the area colour (DAT_10f655c4 stays 0)
extern "C" int DAT_1020d4b8;
extern "C" int DAT_1020d5c8[0x20];
extern "C" int DAT_1020d648;
extern "C" int DAT_1020dcf8[0x100];
extern "C" char DAT_1020e0f8[];                           // second banner text (empty = none)
extern "C" int DAT_10210b28;
extern "C" int DAT_10210b2c;
extern "C" int DAT_102199b0;
extern "C" char DAT_10219bc0[];                           // the message shown
extern "C" char DAT_1021a360;
extern "C" int DAT_1021b548[0x20];
extern "C" int DAT_1021b5c8[0x18];
extern "C" int DAT_1021bbf0;
extern "C" int DAT_1021bbf4;
extern "C" char DAT_1021f640[0x80];                      // event slots: in-use flags
extern "C" int DAT_1021fb40;
extern "C" int DAT_1021fb44;
extern "C" char DAT_10223a50;
extern "C" char DAT_10223a58[0x400];
extern "C" int DAT_10253f18[0x18];
extern "C" int DAT_10254088;
extern "C" unsigned char DAT_10254310[0x80];               // per character slot: Enemy_HopAndStrafe sets its own to 1
extern "C" int DAT_10254590;
extern "C" short DAT_102548b8;
extern "C" unsigned char DAT_10254c40[0x400];             // drop / position ring: 0 = unused, 1 = in use (falling), 2..30 = fading out
extern "C" int DAT_10255040[0x20];                        // effect kind of each slot (0 = free)
extern "C" int DAT_102559f0;
extern "C" int DAT_102559f4;
extern "C" int DAT_102559f8;
extern "C" int DAT_102559fc;
extern "C" int DAT_10255a00;
extern "C" int DAT_10255a08[0x10];
extern "C" Node *DAT_10255f0c;
extern "C" char DAT_10256110[];                           // name of system.sav
extern "C" int DAT_103088b4;
extern "C" int DAT_1030b660;                              // map to enter
extern "C" int DAT_1030b664;
extern "C" char DAT_1030b868;
extern "C" char DAT_1030c478;
extern "C" char DAT_1030c479;
extern "C" char DAT_1030c47a;
extern "C" char DAT_1030c47b;
extern "C" int DAT_1030c500[0x100];                       // the overlay mode's state (entry 0x13 is cleared first)
extern "C" int DAT_1030c568;
extern "C" int DAT_1030c990;
extern "C" char DAT_103150c0[0x80];                      // item stock per item id (saved with the game)
extern "C" int DAT_103154b0[0x6000];
extern "C" float DAT_1032d4b0;                            // fade step per frame
extern "C" int DAT_1032d67c[0x8];
extern "C" int DAT_1032d6a0;
extern "C" int DAT_1032d6a4;                              // set to 9 when the player is knocked back (0x10055ee0)
extern "C" int DAT_1032d6a8[0x40];
extern "C" int DAT_1032d7a8[0x30];
extern "C" int DAT_1042d918[0x10];
extern "C" int DAT_1042d9e4;
extern "C" int DAT_1042da68;                              // frames left
extern "C" int DAT_1042e3f0[0x6000];
extern "C" int DAT_104463f0;
extern "C" int DAT_104463f8[0x80];                       // event slots: state
extern "C" int DAT_10446ea8;
extern "C" int DAT_104472b0[0x18];
extern "C" short DAT_10447338[0x44c];                     // handle of each looping effect while it plays, -11 = not playing
extern "C" char DAT_10447dd0;
extern "C" int DAT_10447fd8[0x80];                       // event slots: third value
extern "C" int DAT_104481d8[0x80];                       // fade of each message box, 0 (gone) .. 14 (fully shown)
extern "C" int DAT_10450808[0x800];
extern "C" int DAT_10467488[0x20];
extern "C" int DAT_10467710;
extern "C" int DAT_10467c20[0x80];                       // frames each message box has been held
extern "C" int DAT_10467e20[0x20];
extern "C" char DAT_10467ea0[];                          // message subtext line (empty = none)
extern "C" int DAT_10467f20;
extern "C" int DAT_10468128;
extern "C" int DAT_1046812c;
extern "C" int DAT_10468230[0x80];                       // event slots: second value
extern "C" int DAT_10468430[0x18];
extern "C" int DAT_104684b0[115];
extern "C" int DAT_104687b0;
extern "C" char DAT_1049ef8c;
extern "C" char DAT_1049f1f8;
extern "C" char DAT_1049f1f9;
extern "C" char DAT_1049f1fa;
extern "C" char DAT_1049f1fb;
extern "C" int DAT_1049f200[0x6000];
extern "C" int DAT_104b7210;
extern "C" int DAT_104bf888;
extern "C" int DAT_104bf894;
extern "C" int DAT_104bf898;
extern "C" int DAT_104bf89c;
extern "C" int DAT_104bf8a0;
extern "C" unsigned int DAT_104bf8a4;                     // frame counter (restarts the colour plug animations)
extern "C" int DAT_104bf8a8;
extern "C" int DAT_104bf8ac;
extern "C" int DAT_104bf8b0;
extern "C" int DAT_104bf8b4;
extern "C" int DAT_104bf8b8;
extern "C" int DAT_104bf8bc;
extern "C" int DAT_104bf8c4;
extern "C" int DAT_104bf8c8;
extern "C" int DAT_104bf8cc;
extern "C" int DAT_104bf8d0;
extern "C" int DAT_104bf8d4;
extern "C" int DAT_104bf8d8;                              // fade-in counter, 15 = fully visible
extern "C" int DAT_104bf8dc;
extern "C" int DAT_104bf8e0;
extern "C" int DAT_104bf8e4;                              // frames left of the fade
extern "C" float DAT_104bf8e8;                            // fade target (set exactly on the last frame)
extern "C" float DAT_104bf8f0;                            // screen/volume fade amount, 0-100
extern "C" int DAT_104bfaf8;
extern "C" char DAT_104bfafc;
extern "C" int DAT_104bfb00;
extern "C" int DAT_104bfb08[0x80];                       // state of each message box
extern "C" int DAT_104bfd0c;
extern "C" int DAT_104bfd14;
extern "C" int DAT_104bfd18;
extern "C" int DAT_104bfd1c;
extern "C" int DAT_104bfd20;
extern "C" int DAT_104bfd24;
extern "C" int DAT_104bfd28;
extern "C" int DAT_104bfd2c;
extern "C" int DAT_104bfd30;
extern "C" int DAT_104bfd34;
extern "C" int DAT_104bfd38;
extern "C" int DAT_104bfd3c;
extern "C" int DAT_104bfd40;
extern "C" int DAT_104bfd44;
extern "C" int DAT_104bfd48;
extern "C" int DAT_104bfd4c;
extern "C" int DAT_104bfd50;
extern "C" unsigned int DAT_104bfd5c;                     // frames spent waiting for a key (message windows)
extern "C" int DAT_104bfd60;
extern "C" int DAT_104bfd64;
extern "C" int DAT_104bfd68;
extern "C" int DAT_104bfd6c;
extern "C" char DAT_104bfd70;
extern "C" char DAT_104bfd71;
extern "C" char DAT_104bfd72;
extern "C" char DAT_104bfd73;
extern "C" int DAT_104bfd74;
extern "C" int DAT_104bfd78;
extern "C" int DAT_104bfd7c;
extern "C" int DAT_104bfd80;
extern "C" int DAT_104bfd84;
extern "C" int DAT_104bfd88[0x8];
extern "C" int DAT_104bfda8;
extern "C" int DAT_104bfdac;
extern "C" int DAT_104bfdb0;
extern "C" int DAT_104bfdb8;
extern "C" int DAT_104bfdbc;
extern "C" int DAT_104bfdc0;
extern "C" int DAT_104bfdc4;
extern "C" int DAT_104bfdc8;
extern "C" int DAT_104bfdcc;
extern "C" int DAT_104bfdd0;
extern "C" int DAT_104bfdd4;
extern "C" int DAT_104bfddc;
extern "C" int DAT_104bfde0;
extern "C" int DAT_104bfde4;
extern "C" int DAT_104bfde8;
extern "C" int DAT_104bfdec;
extern "C" int DAT_104bfdf0;
extern "C" int DAT_104bfdf4;
extern "C" int DAT_104bfdf8;
extern "C" int DAT_104bfdfc;
extern "C" int DAT_104bfe00;
extern "C" int DAT_104bfe04;
extern "C" int DAT_104bfe08;
extern "C" int DAT_104bfe0c;
extern "C" int DAT_104bfe10;
extern "C" int DAT_104bfe14;
extern "C" int DAT_104bfe18;
extern "C" int DAT_104bfe1c;                              // 1 while the overlay mode is open
extern "C" char DAT_104bfe20;
extern "C" int DAT_104bfe2c;
extern "C" int DAT_104bfe30;
extern "C" int DAT_104bfe34;
extern "C" int DAT_104bfe38;
extern "C" int DAT_104bfe3c;
extern "C" int DAT_104bfe40;
extern "C" int DAT_104bfe44;
extern "C" char DAT_104bfe48;
extern "C" int DAT_104bfe4c;
extern "C" int DAT_104bfe50;
extern "C" int DAT_104bfe54;
extern "C" int DAT_104bfe58;
extern "C" int DAT_104bfe5c;
extern "C" int DAT_104bfe60;
extern "C" int DAT_104bfe64;
extern "C" int DAT_104bfe70;                              // frames of the movie decoded so far (0x30 = all)
extern "C" int DAT_104bfe74;
extern "C" char DAT_104bfe7c;
extern "C" int DAT_104bfe88;                              // ring write index
extern "C" char DAT_104bfe8c;
extern "C" int DAT_104bfe90;
extern "C" int DAT_104bfe94;
extern "C" int DAT_104bfe98;
extern "C" int DAT_104bfe9c;
extern "C" int DAT_104bfea0;
extern "C" int DAT_104bfea4;
extern "C" char DAT_104bfea8;
extern "C" char DAT_104bfea9;
extern "C" int DAT_104bfeac;
extern "C" char DAT_104bfeb8;
extern "C" char DAT_104bfeb9;
extern "C" char DAT_104bfeba;
extern "C" char DAT_104bfec0;
extern "C" int DAT_104bff04;
extern "C" int DAT_104bff08;
extern "C" int DAT_104bff10;
extern "C" int DAT_104bff14;
extern "C" int DAT_104bff18;
extern "C" int DAT_104c0020;
extern "C" int DAT_104c0024;
extern "C" int DAT_104c0028;
extern "C" int DAT_104c002c;
extern "C" int DAT_104c0030;
extern "C" int DAT_104c0034;
extern "C" int DAT_104c0038;
extern "C" int DAT_104c003c;
extern "C" int DAT_104c0040;
extern "C" int DAT_104c0044;
extern "C" int DAT_104c0048;
extern "C" int DAT_104c004c;
extern "C" int DAT_104c0050;
extern "C" int DAT_104c0054;
extern "C" int DAT_104c0058;
extern "C" int DAT_104c005c;
extern "C" int DAT_104c0060;
extern "C" int DAT_104c0064;
extern "C" int DAT_104c0068;
extern "C" int DAT_104c006c;
extern "C" int DAT_104c0070;
extern "C" int DAT_104c0074;
extern "C" int DAT_104c0078;
extern "C" char DAT_104c0088;
extern "C" int DAT_104c008c;
extern "C" char DAT_104c0090;
extern "C" char DAT_104c0091;
extern "C" char DAT_104c0092;
extern "C" char DAT_104c0095;
extern "C" char DAT_104c0096;
extern "C" char DAT_104c0097;
extern "C" char DAT_104c0098;
extern "C" char DAT_104c0099;
extern "C" char DAT_104c009a;
extern "C" char DAT_104c009b;
extern "C" char DAT_104c009c;
extern "C" char DAT_104c009d;
extern "C" char DAT_104c009e;
extern "C" int DAT_104c00a0;
extern "C" int DAT_104c00a4;
extern "C" char DAT_104c00a8;
extern "C" short DAT_104c00ac;
extern "C" short DAT_104c00b0;
extern "C" char DAT_104c00b2;
extern "C" char DAT_104c00b3;
extern "C" char DAT_104c00b4;
extern "C" char DAT_104c00b5;
extern "C" char DAT_104c00b6;
extern "C" char DAT_104c00b7;
extern "C" char DAT_104c00bc;
extern "C" char DAT_104c00bd;
extern "C" char DAT_104c00be;
extern "C" char DAT_104c00bf;
extern "C" short DAT_104c00c0;
extern "C" int DAT_104c00c4;
extern "C" char DAT_104c00d4;
extern "C" char DAT_104c00d5;
extern "C" int DAT_104c00d8;
extern "C" int DAT_104c00dc;                              // play time
extern "C" int DAT_104c00ec;
extern "C" int DAT_104c00f0;
extern "C" int DAT_104c00f4;
extern "C" int DAT_104c0108;
extern "C" char DAT_104c010c;
extern "C" int DAT_104c0110;
extern "C" int DAT_104c0114;
extern "C" char DAT_104c0118;
extern "C" char DAT_104c011a;
extern "C" char DAT_104c011b;
extern "C" char DAT_104c011c;
extern "C" char DAT_104c011d;
extern "C" char DAT_104c0120;
extern "C" char DAT_104c0121;
extern "C" int DAT_104c0124;
extern "C" short DAT_104c0128;
extern "C" int DAT_104c012c;
extern "C" char DAT_104c0130;
extern "C" char DAT_104c0131;
extern "C" char DAT_104c0132;
extern "C" char DAT_104c0133;
extern "C" char DAT_104c0134;
extern "C" int DAT_104c0138;
extern "C" int DAT_104c0140;
extern "C" char DAT_104c0148;
extern "C" char DAT_104c0154;
extern "C" char DAT_104c0155;
extern "C" char DAT_104c0156;
extern "C" char DAT_104c0158[4];
extern "C" int DAT_104c015c;
extern "C" int DAT_104c0160;
extern "C" int DAT_104c0164;
extern "C" int DAT_104c0168;
extern "C" int DAT_104c016c;
extern "C" char DAT_104c0170;
extern "C" char DAT_104c0171;
extern "C" char DAT_104c0172;
extern "C" char DAT_104c0173;
extern "C" int DAT_104c0180;
extern "C" int DAT_104c018c;
extern "C" char DAT_104c0190;
extern "C" int DAT_104c0194;
extern "C" int DAT_104c019c;
extern "C" char DAT_104c01a0;
extern "C" int DAT_104c01a4;
extern "C" int DAT_104c01a8;
extern "C" char DAT_104c01ac;
extern "C" int DAT_104c01c8;
extern "C" int DAT_104c01cc;
extern "C" int DAT_104c01d8;
extern "C" Node *DAT_104c01dc;
extern "C" int DAT_104c01e0;
extern "C" int DAT_104c01e4;
extern "C" int DAT_104c01e8;
extern "C" int DAT_104c01ec;
extern "C" int DAT_104c01f0;
extern "C" char DAT_104c01fc;
extern "C" int DAT_104c0204;
extern "C" int DAT_104c020c;
extern "C" int DAT_104c0210;
extern "C" int DAT_104c0214;
extern "C" int DAT_104c0218;
extern "C" int DAT_104c021c;
extern "C" char DAT_104c0224;
extern "C" char DAT_104c022e;
extern "C" char DAT_104c0232;
extern "C" int DAT_104c0234;
extern "C" char DAT_104c023c;
extern "C" char DAT_104c023e;
extern "C" int DAT_104c0270[0x10];                        // anti-tamper noise table
extern "C" int DAT_104c02b0;                              // number of groups read
extern "C" char DAT_104c02e6;
extern "C" char DAT_104c02e7;
extern "C" char DAT_104c02ec;
extern "C" char DAT_104c02ee;
extern "C" char DAT_104c02f4;
extern "C" int DAT_104c02f8;
extern "C" int DAT_104c02fc;
extern "C" char DAT_104c0300;
extern "C" char DAT_104c0301;
extern "C" int DAT_104c0304;
extern "C" char DAT_104c0308;
extern "C" char DAT_104c030a;
extern "C" int DAT_104c030c;
extern "C" int DAT_104c0310;
extern "C" char DAT_104c0318;
extern "C" float DAT_104c031c;                            // spinner angle
extern "C" float DAT_104c0320;                            // loading progress in percent
extern "C" char DAT_104c0330;
extern "C" char DAT_104c0331;
extern "C" int DAT_104c0334;
extern "C" int DAT_104c0344;
extern "C" short DAT_104c034c;
extern "C" char DAT_104c0360;
extern "C" int DAT_104c037c;
extern "C" int DAT_104c03dc;
extern "C" Matrix4 DAT_1065ca08[4];
extern "C" Vec4 DAT_1065cb08;
extern "C" EmptyObject DAT_1065cb28;
extern "C" Vec4 DAT_108029a8;
extern "C" Vec4 DAT_108029b8;                             // where the player starts on entering the map (x, y, z;
                                                          // set from a save by save_slot.cpp, cleared by game_reset.cpp)
extern "C" char DAT_1042de28[0x80];                       // 1 for each item ever obtained (inventory.cpp; the first
                                                          // 0x60 are saved as bits by save_write.cpp)
extern "C" int DAT_10202464[4];                           // 16 bytes of game state stored in the save after the play time
extern "C" Vec4 DAT_10802bd8;
extern "C" int DAT_108360f8;                              // texture slot of the save thumbnail
extern "C" Vec4 DAT_1083631c;
extern "C" Vec4 DAT_10836c84;
extern "C" SmallRecord DAT_10860de8[0x40];
extern "C" int DAT_10c62edc[0x1000];                      // script flags
extern "C" int DAT_10c66144[15];
extern "C" int DAT_10c66590;
extern "C" int DAT_10c66594;
extern "C" int DAT_10c6670c;
extern "C" Vec4 DAT_10c67ef0[4];
extern "C" Vec4 DAT_10c67f30;
extern "C" Vec4 DAT_10c67f50;
extern "C" Vec4 DAT_10c67f60;                             // target colour
extern "C" Vec4 DAT_10daa800;                             // step per frame
extern "C" Vec4 DAT_10daaad4;
extern "C" char DAT_10db6f5e;
extern "C" float DAT_10db7c34;
extern "C" int DAT_10db9254;
extern "C" float DAT_10db9264;
extern "C" int DAT_10db9368;
extern "C" Vec4 DAT_10edbd38;
extern "C" Vec4 DAT_10edbdd8;
extern "C" Vec4 DAT_10edbde8;
extern "C" MeshHolder DAT_10edbe0c[0x40];
extern "C" Vec4 DAT_10f08d1c;
extern "C" char DAT_10f60590[];                           // save directory prefix, e.g. "SaveData\\"
extern "C" int DAT_10f654b4;
extern "C" int DAT_10f654b8;
extern "C" int DAT_10f654f8;
extern "C" int DAT_10f654fc;
extern "C" char DAT_1101d87c;
extern "C" int DAT_11b044f0;

#endif
