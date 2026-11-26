// The in-game frame (game_frame.cpp, 0x1011d810, game mode 1): globals and routines only it uses, and its declaration.
#ifndef GURUMIN_GAME_FRAME_H
#define GURUMIN_GAME_FRAME_H

#include "vec4.h"
#include "matrix4.h"

// 0x1011d810 (the in-game frame, called by the frame step in game mode 1). Runs one pass of the game logic for each
// engine tick that has elapsed since the last frame (the first frame also resets the mode's globals), then prepares
// and draws the scene. Returns 0 when the pass was cut short (the main menu opened or a queued event is due while
// the debug flag is set) and 1 when the frame was completed.
int FUN_1011d810(void);

// 0x10057ee0 (drill_level.cpp): keeps the drill gauge in range and moves the drill level to match it. Returns 0.
int UpdateDrillLevel(void);

extern "C" unsigned char DAT_104c0233;           // counts down once per tick, next to DAT_104c0232
extern "C" Vec4 DAT_10868618[0x20];              // per fish: the direction it faces (unit vector)
extern "C" Vec4 DAT_10daa880[0x20];              // per fish: the point it swims to
extern "C" int FUN_1000c5c0(const Vec4 &position);   // 0x1000c5c0 (slot_alloc.cpp): stores a falling drop's position in a free slot
extern "C" int FUN_1000c390(void);               // 0x1000c390 (falling_drops.cpp, Drops_Update): moves and draws the falling drops
extern "C" Vec4 DAT_10864ce8;                    // world position of the sun: the lens flare point the frame adds
extern "C" int FUN_1000e870(void);               // 0x1000e870 (anti_tamper.cpp, AntiTamper_Check)
extern "C" void FUN_10189080(void);              // 0x10189080: runs once per tick in the mode where DAT_104bfe6c is 1
extern "C" void FUN_1013fa10(int index, int frames, float z, float y, float x, float w);   // 0x1013fa10 (colour_fades.cpp)
extern "C" int __cdecl FUN_1000c2f0(const void *first, const void *second);   // 0x1000c2f0: compares two model indices (qsort order of the model update)

extern "C" int DAT_104c024c;                 // swing power: level * 10 + sub-step (0 = at rest)
extern "C" int DAT_104c0250;                 // swing power saved at the end of a beat
extern "C" int DAT_104c0254;                 // frame the swing's current rhythm started at
extern "C" int DAT_104c0258;                 // frame the swing was last started at

extern "C" float DAT_10314eb8;               // strength of the scene shove (grows while the player is pushed)
extern "C" Vec4 DAT_10868828;                // direction of the last scene shove
extern "C" int DAT_1030c274;                 // index of the respawn point in DAT_10daa400
extern "C" int FUN_1000f360(int index, int frame);       // 0x1000f360 (face_anim.cpp): face change of a character
// One entry of the player's face cue table: at motion frame `frame` the face `face` is shown (frame -3000 ends the table).
struct FaceCue
{
    short frame;
    short face;
};
extern "C" FaceCue DAT_108dbf58[0x80];
extern "C" int DAT_104c01d4;                         // counts down while the player is on the ground (next to DAT_104c01d0)
extern "C" int FUN_100157b0(int index);                  // 0x100157b0 (visibility.cpp): marks a character as in view
extern "C" int FUN_10015710(int index);                  // 0x10015710 (visibility.cpp): 0 = the character is out of view
extern "C" float FUN_10016700(int x, int y);             // 0x10016700 (ground_height.cpp, Ground_HeightAt)
extern "C" int FUN_10016500(int index, const char *path, int start, int end, int once);   // 0x10016500 (char_motion.cpp): plays a motion file
extern "C" int FUN_10011aa0(int id, int value);          // 0x10011aa0 (event_slots.cpp)

extern "C" void FUN_1000ed20(void);              // 0x1000ed20 (app_0009.cpp)
extern "C" void FUN_1003a6c0(void);              // 0x1003a6c0 (scene_start.cpp)
extern "C" int FUN_1000f320(void);               // 0x1000f320 (app_0009.cpp)
extern "C" void FUN_100f5a80(void);              // 0x100f5a80: the scene frame (camera modes, characters)
extern "C" int FUN_10058500(void);               // 0x10058500 (switch_effects.cpp)
int FUN_100564d0(void);                          // 0x100564d0 (debris_update.cpp)
extern "C" int FUN_1000d670(void);               // 0x1000d670 (app_0008.cpp)
extern "C" int FUN_1000f490(void);               // 0x1000f490 (app_0009.cpp)
extern "C" int FUN_1000ec10(void);               // 0x1000ec10 (app_0009.cpp)
extern "C" int FUN_1003a940(void);               // 0x1003a940
extern "C" int FUN_1000dab0(int id, int value, int frames, int mode);   // 0x1000dab0 (volume_fade.cpp)
extern "C" int FUN_10011fb0(int frozen);         // 0x10011fb0 (Water_SetFrozen)

extern "C" int FUN_101b25a0(int a);              // 0x101b25a0: decodes the next frame of the movie library

extern "C" char DAT_1065cb40;                // a key of the current input frame (DemoRecording::buttons_now[0x14]): camera reset request
extern "C" char DAT_1065cb41;                // a key of the current input frame (buttons_now[0x15]): lock-on
extern "C" char DAT_1065cb42;                // a key of the current input frame (buttons_now[0x16]): turns the camera right
extern "C" char DAT_1065cb44;                // a key of the current input frame (buttons_now[0x18]): turns the camera left
extern "C" int FUN_1016c300(void);           // 0x1016c300: non-zero while the camera is locked (a cutscene or a mode that fixes it)
extern "C" int FUN_1016b340(float *outX, float *outY, float x, float y);   // 0x1016b340: turns (x, y) by the camera's heading
extern "C" int FUN_1016bdf0(float angle);    // 0x1016bdf0: puts the camera behind the player looking along `angle`

extern "C" Vec4 DAT_10836dd8[10];            // the pad stick's last 10 directions (unit vectors), the newest last

extern "C" unsigned int DAT_1042dda8[30];    // the last 30 input codes (greetings, walking, action), newest last
extern "C" void FUN_100114a0(void);          // 0x100114a0
extern "C" int FUN_1003a880(const char *path, float x, float y, float z, int start, float ox, float oy, float oz,
                            const Vec4 &colour);   // 0x1003a880 (effect_model.cpp, EffectModel_PlaceColoured)
extern "C" Matrix4 DAT_10836c98;           // copy of the player's matrix, raised for the held item's effects
extern "C" Matrix4 DAT_10daa600[4];            // the matrices the extra sword strikes are placed with
extern "C" float DAT_10467508;               // scale of the player's effect parameter (EffectOptions::m_0004 of attack effects)
extern "C" Vec4 DAT_108392c8;                 // where the swing last ended up
extern "C" int FUN_1016b260(float x, float y);   // 0x1016b260
extern "C" Vec4 DAT_10867e08;                // the last position of the helmet pointer light
extern "C" int FUN_10011520(int unused, int *lockedFlag, int *resetRequest, int input1, int input2, int input3, int restart);   // 0x10011520 (action_select.cpp): the attack the selector picks
extern "C" float FUN_101ac810(float a, float b);   // 0x101ac810: difference of two angles (each wrapped to +-3.1415, the shorter way round)
extern "C" void FUN_1005ecb0(int arg);   // 0x1005ecb0 (gimmick_use.cpp)
extern "C" float DAT_10202360[0x20];         // shove curve of a pushed object: speed factor per frame of the carry action (the second shove reads it two frames earlier)
extern "C" float DAT_102023e0[0x1f];         // the same for the other push side
extern "C" float DAT_102027f4;               // 1.0 normally, 2.0 while the leap runs at double speed
extern "C" Vec4 DAT_1065cb18;                // last non-zero value of the model's m_0090
extern "C" int FUN_1016b400(float *outX, float *outY, float x, float y);   // 0x1016b400 (camera_orbit.cpp, Camera_TurnInput)

// Per character slot (0..0x7f), used by the enemy code of the frame.
extern "C" Vec4 DAT_108377c8[0x80];          // where the model rests while it fades out
extern "C" int DAT_10467f28[0x80];           // fade strength of the model
extern "C" char DAT_1021b940[0x80];          // frames of the hit flash left
extern "C" float DAT_1021b9c0[0x80];         // the tint value the hit flash started from
extern "C" unsigned int DAT_1030c998[0x80];  // DAT_104bf890 at the moment of the last hit
extern "C" Vec4 DAT_10836470[0x80];          // direction the defeated enemy is thrown in
extern "C" Vec4 DAT_108387c8[0x80];          // offset of a carried enemy from its carrier
extern "C" Vec4 *FUN_101ae1c0(Vec4 *out, const Vec4 *direction, float angle);   // 0x101ae1c0: `direction` turned by -angle round the vertical axis
extern "C" Vec4 *FUN_101ae250(Vec4 *out, const Vec4 *direction, float angle);   // 0x101ae250: `direction` turned by +angle round the vertical axis
extern "C" int FUN_100f9670(int index);      // 0x100f9670 (Char_UpdateByKind)
extern "C" int FUN_101148b0(int index);      // 0x101148b0 (char_dispatch.cpp): per-kind update of kinds from 100
#endif
