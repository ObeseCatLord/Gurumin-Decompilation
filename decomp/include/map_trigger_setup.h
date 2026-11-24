// Map_SetUpTrigger (0x10058790) and the globals only the map triggers fill. This is the one declaration of these
// names: include this header, never redeclare them.
#ifndef GURUMIN_MAP_TRIGGER_SETUP_H
#define GURUMIN_MAP_TRIGGER_SETUP_H

#include "vec4.h"
#include "matrix4.h"

struct GameTrigger;   // game.h

// The "OTI" triggers: key of each (OTI_COUNT entries are never exceeded by the maps) and its position.
extern "C" int DAT_1030c1f8[0x100];
extern "C" Vec4 DAT_10daa400[0x100];

// The matrices of the "IWA" nodes (DAT_104c0218 of them).
extern "C" Matrix4 *DAT_10467208[0x20];

// The positions the "SETJUMP" triggers give the camera to keep in view (DAT_1032d5b8 points at them).
extern "C" Vec4 DAT_10838fc8[0x30];

// 0x10058790. Runs one map trigger: reads its name and numbers and sets up what it names (script flags, snow, camera
// jump points, objects and their gimmick records). Returns 0.
int Map_SetUpTrigger(GameTrigger *trigger);

#endif
