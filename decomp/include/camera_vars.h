// Globals of camera, projection and visibility state.
// Generated from the per-file extern declarations by tools/globals_driver.py; this is the one
// declaration of each of them: include this header, never redeclare one. Comments say what is known.
#ifndef GURUMIN_CAMERA_VARS_H
#define GURUMIN_CAMERA_VARS_H

#include "camera_path_select.h"
#include "key_track.h"
#include "node.h"
#include "vec4.h"


extern "C" char DAT_101e8d50[];                            // node name the camera rays are limited to
extern "C" int DAT_10203440;
extern "C" int DAT_10203444;                              // ground height cache, -100 = unknown
extern "C" float DAT_10203448;
extern "C" float DAT_1020344c;
extern "C" float DAT_10203450;
extern "C" float DAT_10203454;
extern "C" float DAT_10203458;
extern "C" float DAT_1020345c;
extern "C" float DAT_10203460;
extern "C" Node *DAT_1021c620;                            // the "Bip01" node the scene looks at
extern "C" Node *DAT_1021c628;                            // "@kumo2"
extern "C" Node *DAT_1021c62c;                            // "@kumo3"
extern "C" Node *DAT_1021c630;                            // "scop-pt"
extern "C" Node *DAT_1021c624;                            // a node the target goal drifts towards (0 = none)
extern "C" int DAT_104c0358;                              // 1 once the camera vectors were set up
extern "C" int DAT_104c035c;                              // frames left in the camera shift (0 = none)
extern "C" Node *DAT_104c039c;                            // the node the camera follows (0 = none)
extern "C" Vec4 DAT_104d29d8;                             // the position of the previous frame
extern "C" Vec4 DAT_10802be8;                             // the position goal of a camera shift
extern "C" Vec4 DAT_108393d8;                             // the point the camera orbits (w: its heading)
extern "C" Vec4 DAT_10866d98;                             // goal of the position
extern "C" int DAT_10c6356c;                              // the shot's point as integers (z + 115)
extern "C" int DAT_10c63570;
extern "C" int DAT_10c63574;
extern "C" Vec4 DAT_10c67ff0;                             // the target goal of a camera shift
// 0x10da93e0: in Camera_Between x = the character the camera stands at (rounded to an index), y = the character it
// looks at (rounded; below 0 = in front of the first), z = how much the target rises with the distance (per cent);
// in Camera_ShiftOrbit the point the camera position orbits.
extern "C" Vec4 DAT_10da93e0;
extern "C" Vec4 DAT_10874838;                             // orbit centre of the camera target (Camera_ShiftOrbit)
extern "C" Vec4 DAT_10da93f0;                             // the orbit centre (the player's position)
extern "C" Vec4 DAT_10edbd48;                             // goal of the target (the followed node's position when there is one)
extern "C" Vec4 DAT_10f08aa0;                             // the target of the previous frame
extern "C" Vec4 DAT_10f65688;                            // camera position (eye)
extern "C" Vec4 DAT_10f668e8;                            // camera target
extern "C" Vec4 DAT_10f668f8;                            // fog colour (x/y/z set by FUN_10199dc0, all four by a "fogset" node)
extern "C" Vec4 DAT_10f66908;
extern "C" Vec4 DAT_10f66938;
extern "C" float DAT_10f65590;                             // 50 above the camera target height
extern "C" float DAT_10f66968;
extern "C" float DAT_10f6696c;
extern "C" int DAT_10f66970;
extern "C" float DAT_10f66974;                           // fog density
extern "C" int DAT_10f8d938[];                            // id of each path
extern "C" char DAT_10faa678;
extern "C" char DAT_10faaa80;
extern "C" int DAT_10fabe88;
extern "C" char DAT_10fabe8c;
extern "C" float DAT_10fdea90[];                         // per path: closest distance of the camera target seen
extern "C" int DAT_10fdeb10[];                            // 1 = has a "...b" node
extern "C" int DAT_10fe01d0;
extern "C" int DAT_10feb3d8;
extern "C" PathSettings DAT_10ff17e0[];
extern "C" int DAT_10ff19e0;
extern "C" int DAT_10ff1df0;                              // 1 = the effect is on
extern "C" float DAT_10ff2ff0;                           // distance of the camera target to the path frame DAT_1101c800
extern "C" int DAT_10ff2ff4;
extern "C" int DAT_11018624;                              // effect timer
extern "C" int DAT_11018628[];                            // 1 = has a second target
extern "C" int DAT_11018728;
extern "C" int DAT_1101872c;
extern "C" int DAT_11018730;                              // effect mode
extern "C" int DAT_11018734;
extern "C" int DAT_11018738;
extern "C" int DAT_11018740[];                            // 1 = active
extern "C" float DAT_1101c800;                            // frame of the path
extern "C" CameraTarget *DAT_1101d884;                  // the model the camera follows
extern "C" int DAT_1101d888;
extern "C" int DAT_1101d88c;                              // frames left of a camera cut blend (0 = none)
extern "C" int DAT_1101d890;                              // 1 while a path camera runs
extern "C" int DAT_1101d894;
extern "C" int DAT_1101d898;
extern "C" int DAT_1101d89c;                              // number of camera paths
extern "C" int DAT_1101d8a0;
extern "C" int DAT_1101d8a4;                              // the current path
extern "C" int DAT_1101d8a8;
extern "C" int DAT_1101d8ac;
extern "C" int DAT_1101d8b0;
extern "C" int DAT_1101d8b4;
extern "C" char DAT_1101d8b8[0x100];                     // title caption text
extern "C" int DAT_1101d9b8;
extern "C" char DAT_1101d9c0[4][0x100];
extern "C" int DAT_1101ddc0[4];
extern "C" int DAT_1101ddd0;
extern "C" int DAT_1101ddd4;
extern "C" int DAT_1101ddd8;
extern "C" float DAT_1101dddc;
extern "C" float DAT_1101dde4;                            // how far the camera target is lifted (0 or up to 120)
extern "C" int DAT_1101dde8;
extern "C" int DAT_1101ddec;
extern "C" int DAT_1101ddf0;
extern "C" int DAT_1101ddf4;
extern "C" int DAT_1101de08;
extern "C" int DAT_1101de0c;
extern "C" int DAT_1101de10;
extern "C" int DAT_1101de14;
extern "C" int DAT_1101de18;
extern "C" int DAT_1101de1c;
extern "C" int DAT_1101de20;
extern "C" int DAT_1101de24;
extern "C" int DAT_1101de28;
extern "C" int DAT_1101de2c;
extern "C" char DAT_1101de38;
extern "C" Node DAT_111be9c8[0x47e];
extern "C" KeyTrack DAT_118e69f8[];                       // the position track of each path
extern "C" Vec4 DAT_118e6c78;                             // goal of the camera target; copies of the target
extern "C" Vec4 DAT_118e6c88;
extern "C" Vec4 DAT_118e6c98;
extern "C" Vec4 DAT_118e6ca8;
extern "C" Vec4 DAT_118e6cd8;
extern "C" KeyTrack DAT_118e96f8[];                       // the "...b" node's track
extern "C" KeyTrack DAT_11af3928[];                       // per path: the camera position track
extern "C" KeyTrack DAT_11af3ba8[];                       // the second target track
extern "C" Vec4 DAT_11af3e28;                             // camera position (camera_orbit.cpp); goal of the camera position; copies of the position
extern "C" Vec4 DAT_11af3e38;
extern "C" Vec4 DAT_11af3e48;
extern "C" Vec4 DAT_11af3e58;
extern "C" Vec4 DAT_11af3f08;                             // camera position before the cut
extern "C" Vec4 DAT_11af3f18;                             // camera target before the cut
extern "C" Vec4 DAT_11af3f48;                             // effect colour (0..255 per channel, stored as floats)
extern "C" Vec4 DAT_11af3f58;                             // the camera-space push of the player (x, y; z is 0)
// Visibility cells seen this frame (visibility.cpp): 1 = cell i is visible. Collision regions "CDLnn" and node kinds
// below 0x40 are indexed by their cell number, so the same table switches them on and off.
extern "C" char DAT_11afffe8[0x40];

#endif
