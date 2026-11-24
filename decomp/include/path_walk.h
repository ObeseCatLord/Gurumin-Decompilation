// Globals of the path walk (player_path_walk.cpp): how the player walks along the map's camera path.
#ifndef GURUMIN_PATH_WALK_H
#define GURUMIN_PATH_WALK_H

extern "C" int DAT_1020c308;     // length of the last step, rounded (0 = standing)
extern "C" int DAT_1020c30c;     // heading of the last step plus pi, times 1000
extern "C" int DAT_1020c310;     // 1 once a step was taken: the next input is turned towards that heading
extern "C" int DAT_1020c314;     // the camera path key (frame) the player is nearest to
extern "C" float DAT_10e3dec0;   // heading of the last step plus pi
extern "C" int DAT_10c64a58;     // copy of DAT_1020c314

int Player_WalkAlongPath(int stickX, int stickY);

#endif
