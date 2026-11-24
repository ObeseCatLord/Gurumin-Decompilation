// Command codes of the world's host callback (scene_callback.cpp, 0x10092af0) and the globals only it uses. The
// script engine (WorldRenderer::Bind) calls the callback with a code and a pointer to the command's arguments.
#ifndef GURUMIN_SCENE_CALLBACK_H
#define GURUMIN_SCENE_CALLBACK_H

#include "dshow_stream.h"

// Codes 1..99 name a character slot's model file ("Name.it3": the name without the 4-character extension is
// compared with the slots' file names, case ignored) and make the world use that slot's model.
#define SCENE_CB_LOAD_MAP 100                // args: map constant name; loads the map but keeps the player's start
#define SCENE_CB_SUBSTITUTE_TEXT 2000        // args: text; replaced by its word-substituted form
#define SCENE_CB_MOUSE_SKIP 0x29a            // sets the skip request when the skip button or key is pressed
#define SCENE_CB_RESET_SOUND_MUTE 999
#define SCENE_CB_PLACE_EFFECT_MODEL 0x28b    // args: file, x, y, z, turn
#define SCENE_CB_START_EFFECT 0x28c          // args: effect, x, y, z, rotated (0/1), turn in degrees
#define SCENE_CB_START_EFFECT_AT_NODE 0x28d  // args: effect, model, node name
#define SCENE_CB_SET_GAME_FLAG 0x28e         // args: flag id, value
#define SCENE_CB_RUN_VALUE_COMMAND 0x2a2     // args: script value id, value
#define SCENE_CB_DETACH_NODE 0x2ef           // args: node name; detaches the node from its parent
#define SCENE_CB_RESTART_COLOUR_PLUGS 0x320  // args: node name, motion name at byte 0xfa
#define SCENE_CB_PLAY_EFFECT_SOUND 0x352     // args: sound id; plays it unpositioned
#define SCENE_CB_PLAY_BGM 0x353
#define SCENE_CB_PLAY_SOUND_AT_MODEL 0x354   // args: sound id, model (negative = unpositioned), volume in percent / 2
#define SCENE_CB_PLAY_SOUND_AT_POINT 0x355   // args: sound id, -, volume, x, y, z
#define SCENE_CB_PLAY_WAV 0x35c
#define SCENE_CB_PLAY_WAV_AT_MODEL 0x35e
#define SCENE_CB_PLAY_WAV_AT_POINT 0x35f
#define SCENE_CB_COPY_NODE_TO_PICTURES 0x458 // args: node name; applies the parameter block at byte 0x40 to every node of that name
#define SCENE_CB_MMV_OPEN 0x5dc
#define SCENE_CB_MMV_NEXT_FRAME 0x5dd
#define SCENE_CB_MOVIE_OPEN 0x5de
#define SCENE_CB_MOVIE_NEXT_FRAME 0x5df
#define SCENE_CB_MOVIE_CLOSE 0x5e0
#define SCENE_CB_DSHOW_OPEN 0x5e1
#define SCENE_CB_DSHOW_NEXT_FRAME 0x5e2
#define SCENE_CB_DSHOW_CLOSE 0x5e3

#define SCENE_CB_VOLUME_DIVISOR 50.0f
#define SCENE_CB_SOUND_HEIGHT 60.0f          // sounds at a model come from this far above its position

int Scene_HostCallback(int code, int *args);   // 0x10092af0 (C++: the matrix temporaries give it an unwind frame)

extern "C" unsigned char DAT_10da7dd8;     // set by the flag command 0xa29
extern "C" void *DAT_11b1bb54;             // pixels of the movie frame decoded by the library at 0x101b27d0
extern "C" int DAT_11b1bb58;               // its width
extern "C" int DAT_11b1bb64;               // its height
extern "C" DShowStream DAT_10836e98;       // the DirectShow movie player

#endif
