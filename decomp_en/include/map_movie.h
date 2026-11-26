// Background movie of the map (EN build; JP twin: decomp/src/map_movie_select.cpp). Declares the globals and the
// function of the movie choice. This is the one declaration of them in decomp_en.
#ifndef GURUMIN_EN_MAP_MOVIE_H
#define GURUMIN_EN_MAP_MOVIE_H

// Movies that can be named in a camera point (size of the name table).
const int MAP_MOVIE_COUNT = 0x40;
// Bytes of a movie file name.
const int MAP_MOVIE_NAME_SIZE = 0x20;
// Map record chosen when no camera point lies near the player.
const int MAP_MOVIE_DEFAULT_ENTRY = 0x5c;
// Squared distance from the player within which a camera point is chosen (100 units).
const float MAP_MOVIE_NEAR_DISTANCE_SQUARED = 10000.0f;
// A movie shows its first MAP_MOVIE_FRAMES_FORWARD frames forwards, then plays them back mirrored.
const int MAP_MOVIE_FRAMES_FORWARD = 0x30;
// Frame count of the forwards-and-back cycle plus one (the mirrored frame is MAP_MOVIE_MIRROR_FRAME - frame).
const int MAP_MOVIE_MIRROR_FRAME = 0x61;

// Names of the movie files (index CameraPoint::movie). EN 0x00f45bf0 (JP DAT_10875e48).
extern "C" char g_movieNames[MAP_MOVIE_COUNT][MAP_MOVIE_NAME_SIZE];
// The movie that is playing (a CameraPoint::movie value). EN 0x00916f80 (JP DAT_1020c2d0).
extern "C" int g_playingMovie;
// Texture slot of the current map's icon (Map_IconTexture's result). EN 0x0091da44 (JP DAT_1020b228).
extern "C" int g_mapIconTextureSlot;
// Copy of the chosen map entry, for the map screen. EN 0x01334804 (JP DAT_10c64a5c).
extern "C" int g_mapScreenEntry;

// Picks the camera point nearest to the player, switches to its movie and returns 0. EN 0x5b3860 (JP 0x1001f270).
int Map_SelectMovie();

#endif
