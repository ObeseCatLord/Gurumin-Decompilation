// The player's per-frame scene update (player_frame.cpp, 0x1004d960): globals and types it owns.
#ifndef GURUMIN_PLAYER_FRAME_H
#define GURUMIN_PLAYER_FRAME_H

#include "matrix4.h"
#include "vec4.h"

// A point of the guide marker table DAT_10216cc0: an integer position and the 2D range within which the player
// counts as near it. Records are numbered from 1; record 0 is not a point, its first field holds how many there are.
struct MarkerPoint
{
    union
    {
        int count;        // record 0 only: the number of points
        int x;
    };
    int y;
    int z;
    int range;
};

extern "C" MarkerPoint DAT_10216cc0[];
extern "C" Matrix4 DAT_1086e0f8;    // the viewer's world matrix (its translation is the viewer's position)
extern "C" int DAT_10c63388;        // non-zero = the player stands still while the field input is locked
extern "C" int DAT_10c63510;        // 1 = the quiet state that the script flag DAT_104bf8bc = 1 ends
extern "C" int DAT_10c63590;        // 0..1000: how far the guide beam has travelled to its goal
extern "C" int DAT_1020b830;        // the guide marker the pointer arrow shows (0 = none yet)
extern "C" int DAT_1020b834;        // frames left that the arrow flies to a newly chosen marker
extern "C" int DAT_1020c2c8;        // frames the player update has run
extern "C" int DAT_1020c2fc;        // frame of the camera path DAT_104c03ac the camera is held at (negative = none)
extern "C" int DAT_1020c348;        // the helmet item shown on the player now
extern "C" int DAT_1020c34c;        // guide beam: 0 = off, 1 = start at the scene registers, 2 = running
extern "C" float DAT_10e3debc;      // the heading the player model is drawn with (eases towards DAT_10e3dec0)
extern "C" Vec4 DAT_10803b08;       // the guide beam's head
extern "C" Vec4 DAT_10803b38;       // the guide beam's velocity

// 0x1004d960 (player_frame.cpp): called by the scene frame once per frame.
int Player_UpdateFrame();

#endif
