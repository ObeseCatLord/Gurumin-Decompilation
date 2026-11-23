// Globals of maps, regions, routes and collision.
// Generated from the per-file extern declarations by tools/globals_driver.py; this is the one
// declaration of each of them: include this header, never redeclare one. Comments say what is known.
#ifndef GURUMIN_MAP_VARS_H
#define GURUMIN_MAP_VARS_H

#include "matrix4.h"
#include "vec4.h"

struct Node;   // node.h

extern "C" int DAT_102022c4;                              // non-zero = draw the overhead pass
extern "C" int DAT_102028c8;                              // texture slots held by the map (-1 = none); texture of the count-down digits
extern "C" int DAT_102028cc;
extern "C" int DAT_102028d0;
extern "C" int DAT_102028d4;
extern "C" int DAT_102028dc;
extern "C" char DAT_10202b40[][0x100];
extern "C" int DAT_1020dcd0[10];
extern "C" int DAT_102108d0[6];                           // owner of each attached matrix (0 = free)
extern "C" int DAT_10219bb8;
extern "C" int DAT_1021a370[10];
extern "C" int DAT_1021b728[0x5c];
extern "C" int DAT_1021bbf8[0x5c];
extern "C" int DAT_1021d018[0x28];
extern "C" char DAT_1021f3fa[0x42];
extern "C" int DAT_102549a8[0x42];
extern "C" int DAT_10255960[3][12];
extern "C" int DAT_102564a0;                              // number of map object records
extern "C" int DAT_103088d0[0xb64];
extern "C" int DAT_1032d868;                              // number of listed parts
extern "C" int DAT_1042dea8[0x5c];                        // per-character group numbers (characters 0..0x5b)
extern "C" int DAT_1049efd8[0x3c];
extern "C" float DAT_1049f3dc[][16][16];                  // height samples per object, 25 units apart (-200 = none)
extern "C" int DAT_104bfdb4;                              // the ground object nearest to the player
extern "C" Node*DAT_104c0080;                             // second water node (found next to the surface)
extern "C" Node*DAT_104c0084;                             // "ICERIPA": the ice surface added below the water node
extern "C" int DAT_104c00b8;                              // non-zero = invulnerable
extern "C" char DAT_104c02dc;                             // 1 while the water is frozen
extern "C" int DAT_104c0350;
extern "C" int DAT_104c0354;                              // non-zero = the camera is not smoothed here
extern "C" int DAT_104c0364;                              // the character whose voice played (0 = none)
extern "C" int DAT_104c0368;                              // that voice (-5000..5000, 0x240 = first voice sound)
extern "C" int DAT_104c036c;                              // 1 once a scripted character was registered
extern "C" int DAT_104c0370;
extern "C" int DAT_104c038c;
extern "C" Node*DAT_104c0390;                             // the model of the map's held object
extern "C" short DAT_104c0398;
extern "C" void*DAT_104c03a0;
extern "C" int DAT_104c03a4;
extern "C" Node*DAT_104c03a8;
extern "C" int DAT_104c03b4;
extern "C" int DAT_104c03b8;
extern "C" int DAT_104c03bc;
extern "C" Matrix4 DAT_104c03f8;                          // a second projection
extern "C" Vec4 DAT_104d29e8;                             // the pit area: minimum corner
extern "C" Vec4 DAT_104d29f8;                             // the pit area: maximum corner
extern "C" Matrix4 DAT_10836cd8;                          // the camera under that projection
extern "C" Matrix4 DAT_10836ec8;                          // parent matrix of the overhead pass (identity, set in game_reset.cpp)
extern "C" Matrix4 DAT_10866da8;                          // the projection of the main view
extern "C" Matrix4 DAT_108687f8[0x80 * 6];
extern "C" int DAT_10c62250[0x100];
extern "C" int DAT_10c62650[0x200];
extern "C" int DAT_10c63370;
extern "C" int DAT_10c6505c;                              // map script timer: > 0 running, 0 = raise the end event, < 0 = ended
extern "C" Vec4 DAT_10da90d0;                             // current colour
extern "C" int DAT_10daaad0;                              // 1 while the effects draw into it
extern "C" float DAT_10db9248;                            // the player's height (character 0, position z)
extern "C" Matrix4 DAT_10e40204;                          // world matrix of the object the map 0x18 effect is placed on
extern "C" Vec4 DAT_10edbd58[8];
extern "C" Matrix4 DAT_10f08880[6];                       // attached matrices
extern "C" char DAT_10f19860[16][0x100];
extern "C" int DAT_10f1fd24;
extern "C" float DAT_10f65594;                            // height of the view
extern "C" int DAT_10f655bc;                              // 1 while the map nodes are queued for it
extern "C" Vec4 DAT_10f668b8;                             // the visible area: maximum corner
extern "C" Vec4 DAT_10f66918;                             // the overhead pass's area: minimum corner
extern "C" Vec4 DAT_10f66928;                             // maximum corner
extern "C" int DAT_10faae88[0x400];                       // reference count of each texture name
extern "C" char DAT_10ff65f8[0x400][0x20];                // texture names (see mesh_cache.cpp)
extern "C" char DAT_11affdd8[100];                        // per collision region "CDLnn": 1 = enabled

#endif
