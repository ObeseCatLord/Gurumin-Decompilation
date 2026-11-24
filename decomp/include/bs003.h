// The BS_003 boss (a winged lizard fought on the Tokatyu maps 0x133/0x134; bs003_update.cpp, bs003_variant.cpp): its
// per-frame behaviour, the texture variants of its model and the globals only it uses.
#ifndef GURUMIN_BS003_H
#define GURUMIN_BS003_H

#include "vec4.h"
#include "matrix4.h"
#include "node.h"

// CharSlot::kind of the boss.
#define BS003_KIND 0x37

// Where the boss's bones are kept: DAT_1042d918[BS003_BONE_*] (Node pointers, found by name in state 0).
enum Bs003Bone
{
    BS003_BONE_HEAD = 0,
    BS003_BONE_JAW = 1,
    BS003_BONE_SPINE = 2,
    BS003_BONE_LEFT_TOE = 3,
    BS003_BONE_RIGHT_TOE = 4,
    BS003_BONE_RIGHT_WING = 5,
    BS003_BONE_LEFT_WING = 6,
    BS003_BONE_EYE_ROOT = 7,
    BS003_BONE_COUNT = 8
};

// DAT_1021bdf8[slot][BS003_NODE_*]: the animation nodes of the boss's scripted fight (found in tokatyu.it3). Each
// carries key frames that start an event (a motion, an attack, a sound) when the boss's frame reaches them.
enum Bs003Node
{
    BS003_NODE_MAP_MOVE = 0,      // "mapmove": where the whole map model sits at each frame
    BS003_NODE_BODY_MOVE = 1,     // "tokamove": the boss's body path
    BS003_NODE_SIGHT = 2,         // "tokasiten": the point the boss looks at
    BS003_NODE_BREATH_A = 3,      // "bress1": key = a claw swipe starts
    BS003_NODE_LASER = 4,         // "laser1": key = the breath attack starts
    BS003_NODE_CHARGE = 5,        // "totugeki": key = the flame is switched on or off
    BS003_NODE_SHAKE = 6,         // "yure": key = the roar and the shaking
    BS003_NODE_THUNDER = 7,       // "rakurai": key (30 frames ahead) = a shard of rock is thrown
    BS003_NODE_BREATH_B = 8       // "tokahika": key = the head effect (state 6) or the beam (state 8)
};

extern "C" Vec4 DAT_10802bb8[0x80][4];   // per slot: [0] = the camera's look point while the boss is shown
extern "C" Vec4 DAT_10802c08[0x80][4];   // per slot: [0] zeroed when the boss is set up
extern "C" Vec4 DAT_104c4bd8[6];         // drift of each of the six rock shards (the shard k uses entry 5 - k)
extern "C" int DAT_10c6476c;             // value given to the cut-scene texts (houdai*.txt)

#define BS003_PEOPLE_FIRST 0x3a   // the people's slots 0x3a..0x5b (cleared of health in the last form)
#define BS003_PEOPLE_END 0x5c

extern "C" int FUN_1000da40(int id, int value, int frames);       // 0x1000da40 (volume_fade.cpp): fades the volume
extern "C" void FUN_100cc1d0(const char *characterFile, int value);   // 0x100cc1d0 (scene_start.cpp): starts a cut-scene text
extern "C" int FUN_10034f70(const Matrix4 *matrix, float x, float y, float z, int owner);   // group_tables.cpp: attachment slot

// 0x10034920 (Bs003_SetVariant, bs003_variant.cpp): puts texture variant `variant` (0-4) on the model of slot `index`.
extern "C" int FUN_10034920(int index, int variant);

// 0x100e25f0 (Bs003_Update; C++ linkage: the original has an EH frame for its matrices). One frame of the boss in
// slot `index`. Returns 0.
int Bs003_Update(int index);

#endif
