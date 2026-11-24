// The map 0x18 football game's CPU players (soccer_ai.cpp): each frame one CPU player picks a role, runs towards the
// ball or holds its post, steps round the goal nets and the other players, jumps for high balls and moves.
#ifndef GURUMIN_SOCCER_AI_H
#define GURUMIN_SOCCER_AI_H

#include "vec4.h"
#include "route_point.h"

// SoccerState::role: 1 chases the ball, 2 and 3 hold the back and forward posts until the ball comes close.
#define SOCCER_ROLE_NONE 0
#define SOCCER_ROLE_CHASER 1
#define SOCCER_ROLE_BACK 2
#define SOCCER_ROLE_FORWARD 3

// Motions the player model is asked to play (AnimModel::PlayMotion).
#define SOCCER_MOTION_WALK 0
#define SOCCER_MOTION_RUN 1
#define SOCCER_MOTION_IDLE 2

// DAT_1020c2b0 in the football map: the character slot of each member of the roster. Entry 2 stands for the ball
// (it has no character slot of its own and is skipped), entries 3..7 are the players. (The dodge game of map 0x16 keeps
// its state in the same words, see dodge_game.h.)
#define SOCCER_ROSTER_SIZE 8
#define SOCCER_ROSTER_BALL 2
#define SOCCER_ROSTER_FIRST_PLAYER 3
extern "C" int DAT_1020c2b0[SOCCER_ROSTER_SIZE];

// DAT_10211000: rows 4..15 of the map record DAT_10210fc0[0], which hold the football pitch. Rows 0 and 1 are the two
// corners of the goal nets' x range (x), with the y ranges of the two nets in the y of rows 0/1 and 2/3; rows 2*team
// and 2*team + 1 hold the corners (x and code, doubled) of the zone the team's players keep to. Row 8 is the
// anchor of the back (x) and forward (y) posts.
extern "C" RoutePoint DAT_10211000[12];
// DAT_10211040: radius of each roster member (entry 0 = the ball), the integers of rows 4 and up read in a row.
extern "C" int DAT_10211040[][4];

// DAT_10803ac0: the CPU's skill value; below 4 a CPU player only jumps for balls within 50 units.
extern "C" float DAT_10803ac0;

// The football AI reads the length of each slot's last step, LastPosition::step (last_position.h).

// 0x10032240 (SoccerGame_UpdateCpuPlayer; C++ linkage: the original has an EH frame). One frame of the CPU player in
// character slot `index` of team `team` (0 or 1, the side it shoots at); `mateA` and `mateB` are the slots of its
// two team mates (0 = none) and `clearance` the extra distance kept from other players and the nets. Returns 0.
int SoccerGame_UpdateCpuPlayer(int index, int team, int mateA, int mateB, int clearance);

#endif
