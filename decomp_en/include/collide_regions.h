// Collision against the world's region nodes (EN build; JP: collide_regions.cpp). This is the one declaration of these
// globals and functions in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_COLLIDE_REGIONS_H
#define GURUMIN_EN_COLLIDE_REGIONS_H

#include "visibility.h"
#include "node.h"
#include "vec4.h"

// Node::FindByFlags mask that selects the collision (region) nodes.
const unsigned int NODE_MASK_REGION = 4;
// Distance above and below a marker point that the test ray reaches.
const float REGION_RAY_REACH = 10000.0f;

// Probe ray of Regions_CollideRay: length in y and z, the lowering of its footprint, the stretch of the long ray and the
// margin a node box is widened by.
const float RAY_LENGTH = 1700.0f;
const float RAY_DROP = 350.0f;
const float RAY_STRETCH = 2.0f;
const float REGION_MARGIN = 150.0f;

extern "C" Node *g_markerNode;          // EN 0x0091da3c (JP DAT_104c03ac): node whose translation keys are the marker points
extern "C" char *g_markerFree;          // EN 0x0091d798 (JP DAT_104c0394): per marker point 1 = nothing is in the way

// Moves `position` by `velocity` against the region nodes of `root` whose x/y bounds contain the position; returns 1
// at the first node that reports a collision, else 0. EN 0x6353f0.
extern "C" int Regions_Collide(Node *root, const char *tag, unsigned int mask, Vec4 *position, Vec4 *velocity, Vec4 *hit);
// Casts a ray from `position` along the probe direction against the region nodes of `root` whose widened x/y box overlaps
// the ray footprint; returns 1 at the first node that reports a collision, else 0. EN 0x635010.
extern "C" int Regions_CollideRay(Node *root, const char *tag, unsigned int mask, Vec4 *position, Vec4 *velocity, Vec4 *hit);
// Tests every marker point against the world's "testatari" regions. Returns 0. EN 0x5c1340.
extern "C" int Markers_TestRegions(void);

// Returns the first "@CDL" region whose x/y rectangle contains `point` with a REGION_POINT_MARGIN margin on every side,
// leaving out the regions named for eggs, crowds and fire; 0 when there is none or no world is loaded. EN 0x631480.
extern "C" Node *Map_FindRegionAt(const Vec4 *point);

// Node flag of the collision probe nodes (the same bit as the debug sphere shape) and the part of a probe's radius that
// counts as its sphere.
const unsigned int NODE_FLAG_PROBE = 0x400;
const float PROBE_RADIUS_SCALE = 0.578f;

// Tests every probe node below `node` (at its position plus its bounds centre plus `offset`) as a sphere against the world;
// each hit's normal (left normalised in `normal`) is folded into the running average `average`, weighted by the number of
// hits so far (`count`). Returns 0. EN 0x5c2800 (JP 0x10024ac0).
extern "C" int Collision_AverageNormals(Node *node, const Vec4 *offset, Matrix4 *matrix, Vec4 *normal, Vec4 *average, int *count, float *depth);

#endif
