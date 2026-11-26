// World collision queries of the EN build (JP: decomp/src/collide_query.cpp).
#ifndef GURUMIN_EN_COLLIDE_QUERY_H
#define GURUMIN_EN_COLLIDE_QUERY_H

#include "vec4.h"
#include "char.h"

// Collision mask of the map's ground and walls.
const unsigned int COLLISION_MASK_WORLD = 0x4000000;
// Height above the probe point a ledge test starts from.
const float LEDGE_PROBE_HEIGHT = 90.0f;
// x value of a ground probe that hit nothing.
const float PROBE_NO_HIT = -200000.0f;

// 1 when there is world collision below the point `distance` units along `direction` from `position`; the ray runs
// from 90 units above that point to `depth` below it. EN 0x5c0fc0 (JP 0x10024c40).
extern "C" int Collision_GroundAhead(const Vec4 *position, Vec4 *direction, float depth, int distance);
// Casts a ray down from `height` above `position` to `position` and stores the end in `ground` (x = -200000 when
// nothing was hit). Returns 1 on a hit. EN 0x5c0b00 (JP 0x100248e0).
extern "C" int Collision_ProbeDown(Vec4 position, Vec4 *ground, float height);

// Collision mask of small objects, and the height above the feet a head-less probe starts from.
const unsigned int COLLISION_MASK_SMALL_OBJECTS = 4;
const float HEAD_PROBE_LIFT = 30.0f;
// Index of the w element of a matrix's translation row (m[15]).
const int MATRIX_TRANSLATION_W = 15;

// Casts a ray down `depth` units from character `index` (from its head node when `fromHead` is set and it has one,
// else from its position, 30 units up when `fromHead` is set), moved by `offset` when given. With `testOnly` only
// whether something is hit is returned; otherwise the hit point goes to `ground` (x = -200000 when there is none).
// Returns 1 on a hit. EN 0x5c0e40 (JP 0x10024750).
extern "C" int Char_CastRayDown(int index, Vec4 *ground, float depth, int fromHead, const Vec4 *offset, int testOnly,
                                int smallObjects);

// 1 when a sphere of `radius` at the point (x, y, z) touches the world collision. EN 0x5c3360 (JP 0x1000ff80).
int Collision_ProbePoint(float x, float y, float z, float radius);

// Slot ground test: runs every GROUND_TEST_PERIOD frames per slot; the ray starts GROUND_TEST_UP above the point
// GROUND_TEST_AHEAD units along the slot's vector and ends GROUND_TEST_DOWN below it.
const int GROUND_TEST_PERIOD = 10;
const float GROUND_TEST_AHEAD = 15.0f;   // (the source uses the literal: a named constant changes the code)
const float GROUND_TEST_UP = 50.0f;
const float GROUND_TEST_DOWN = 300.0f;

extern "C" unsigned short g_frameCounter;                 // EN 0x009160d0 (JP DAT_104c0380): frames since the scene started
extern "C" Vec4 g_slotGroundPoint[CHAR_SLOT_COUNT];       // EN 0x00daa880 (JP DAT_10864cf8): ground point found below each slot
extern "C" char g_slotNoGround[CHAR_SLOT_COUNT];          // EN 0x00914560 (JP DAT_10467920): 1 = no ground found below the slot

// Every tenth frame (spread over the slots by slot number) casts a ray at the point 15 units along slot `slot`'s
// vector from `point`, against the world collision filtered by the slot's collision filter, and records the hit
// point or that there is no ground. Returns 1 when the slot has no ground below it. EN 0x6abce0 (JP 0x1001a450).
extern "C" int Slot_HasNoGround(int slot, const Vec4 *point);

// Ray flags of the heading probes below.
const int PROBE_RAY_FLAGS = 4;

// Casts a ray from character `index` (from its head node when `fromHead` is set and it has one, else from its
// position), moved by `offset` when given, to the point `depth` units ahead of it (along -y turned by its heading)
// against the world collision. With `testOnly` only whether something is hit is returned; otherwise the hit point goes
// to `ground` (x = -200000 when there is none). Returns 1 on a hit. EN 0x5c1f10 (JP 0x100242c0).
extern "C" int Char_ProbeAhead(int index, Vec4 *ground, float depth, int fromHead, int testOnly, const Vec4 *offset);
// Char_ProbeAhead along `angle` instead of the heading, limited to the collision nodes called `name` (0 = all).
// EN 0x5c1cc0 (JP 0x100244b0).
extern "C" int Char_ProbeDirection(int index, float angle, Vec4 *ground, float depth, int fromHead, int testOnly,
                                   const Vec4 *offset, const char *name);

// CastRay flags and mask of Map_TraceRay: the tagged collision meshes of the map that take part in collision.
const int MAP_TRACE_RAY_FLAGS = 4;
const unsigned int MAP_TRACE_RAY_MASK = 0x4000000;
// Casts a ray between `second` and `first` against the map collision meshes with tag `tag`. Returns 1 on a hit; the hit point is
// copied to `hitOut` unless `testOnly` is non-zero. Returns 0 when no map is loaded or nothing is hit. EN 0x5c1e80 (see
// collide_query.cpp). Called by the character update (EN 0x66e780).
extern "C" int Map_TraceRay(Vec4 *first, Vec4 *second, Vec4 *hitOut, int testOnly, const char *tag);

#endif
