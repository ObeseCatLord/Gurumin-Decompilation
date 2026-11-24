// Globals and engine calls of the map 0x18 football game (football_game.cpp) that no other header declares. The ball is
// the model of character slot 0x3b (DAT_10db6ee8[FOOTBALL_BALL_SLOT]); the five players are the roster entries
// 3..7 of DAT_1020c2b0 (soccer_ai.h), entry 2 stands for the human player (slot 0). Many of these words are shared
// with the other mini games of maps 0x15..0x17 (dodge_game.h, mole_game.h, target_race.h declare them with their
// own meanings): only include this header where those are not included.
#ifndef GURUMIN_FOOTBALL_STATE_H
#define GURUMIN_FOOTBALL_STATE_H

#include "vec4.h"
#include "matrix4.h"
#include "minigame_area.h"
#include "menu_scene.h"
#include "ui_vars.h"
#include "field_update.h"
#include "player_frame.h"
#include "field_item_menu.h"
#include "path_walk.h"

struct Node;   // node.h

// The ball's character slot, and the roster entries of DAT_1020c2b0.
#define FOOTBALL_BALL_SLOT 0x3b
#define FOOTBALL_ROSTER_STATE 0          // 0 = not set up yet, 1 = running
#define FOOTBALL_ROSTER_FRAMES 1         // frames the game has run
#define FOOTBALL_ROSTER_HUMAN 2
#define FOOTBALL_TEAM0_FIRST 3           // the human's team mates (roster 3 and 4)
#define FOOTBALL_TEAM1_FIRST 5           // the opponents (roster 5, 6 and 7)

// One 0x40 byte record of DAT_1020c368: the first word counts the frames a player celebrates a goal.
struct FootballCelebration
{
    int frames;
    int pad_0004[15];
};

// The football meanings of words other headers declare (each declared once there): DAT_1020c2d0 goal timeline (frames
// the ball has been inside a net, negative while a goal is celebrated); DAT_1020c2f0[0].footballKick how the human's
// kick turned out (0 none, 1 pass, 2 long pass, 3..10 shots); DAT_1020c2f4 cleared with the kick; DAT_1020c2f8 frames the
// ball keeps its direction after a kick; DAT_1020c2fc the ball's vertical speed (bounce); DAT_1020c304 1 while the ball
// rolls on the ground; DAT_1020c308 frames the ball keeps rising (counts down); DAT_1020c30c team that last kicked
// (1 = human's, 2 = opponents); DAT_1020c310 frames before that is forgotten; DAT_10803ab8[0] the ball's velocity
// ([1] cleared with it); DAT_10255510[0].footballSpeedFactor the ball's speed factor; DAT_1086e0b8[0].endA set to identity.
extern "C" int DAT_1020c2e0;      // 1 once the scenery nodes of slots 0x5c..0x7b are prepared (first frame only)
extern "C" int DAT_1020c270;      // frames the human's team mates keep playing after the player moved (300 on movement)
extern "C" int DAT_1020c300;      // cleared with the other words of the kick
extern "C" FootballCelebration DAT_1020c368[5];
extern "C" float DAT_10255514;    // roll angle of the ball model, 0..2 pi
extern "C" float DAT_10255500;    // height of the ball's "Bip01" node (its matrix z)
extern "C" Vec4 DAT_10837378;     // the point the player's kick is measured from (the ball's resting offset)
extern "C" char DAT_1025511b;     // set by the player's attack: the ball was struck this frame
extern "C" char DAT_10255ac3;     // cleared with the ball's state
extern "C" char DAT_1031538b;     // non-zero while the game is frozen (only the effect on the ball is kept)
extern "C" int DAT_1030879c;      // cleared with the ball's state
extern "C" char DAT_1020d473;     // set to 1 when the game starts
extern "C" short DAT_10315426;    // cleared while the ball is unreachable
extern "C" int DAT_1021985c;      // cleared while the ball is unreachable
extern "C" Node *DAT_1021c644[9]; // the nodes flagged 0x24 in the map; entry 0 is then replaced by the ball model "@EN_006"

// 0x10034210: sweeps a sphere of `radius` from `from` along `direction` against the map. Returns 0 for no hit, 1 for a
// wall, 2 for a floor or slope; `hitPoint` receives the contact, `distance` the distance, `normal` the surface normal.
extern "C" int FUN_10034210(const Vec4 *from, const Vec4 *direction, float radius, Vec4 *hitPoint, float *distance,
                            Vec4 *normal, Vec4 *extra);
extern "C" int FUN_1015c9e0(int mode);                        // 0x1015c9e0 (Screen_StartTransition)
extern "C" int FUN_1016afa0(int mode, int r, int g, int b);   // 0x1016afa0 (colour_fades.cpp)

extern "C" int DAT_10467d10[5];
extern "C" int DAT_104482c8[5];
extern "C" int DAT_104bfbf8[5];
#endif
