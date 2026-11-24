// Collision against the world's region nodes (EN build; JP: collide_regions.cpp). This is the one declaration of these
// globals and functions in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_COLLIDE_REGIONS_H
#define GURUMIN_EN_COLLIDE_REGIONS_H

#include "node.h"
#include "vec4.h"

// Node::FindByFlags mask that selects the collision (region) nodes.
const unsigned int NODE_MASK_REGION = 4;
// Distance above and below a marker point that the test ray reaches.
const float REGION_RAY_REACH = 10000.0f;

extern "C" Node *g_worldRoot;           // EN 0x00927104 (JP DAT_10255f08): root of the loaded world, 0 while none is loaded
extern "C" Node *g_markerNode;          // EN 0x0091da3c (JP DAT_104c03ac): node whose translation keys are the marker points
extern "C" char *g_markerFree;          // EN 0x0091d798 (JP DAT_104c0394): per marker point 1 = nothing is in the way

// Moves `position` by `velocity` against the region nodes of `root` whose x/y bounds contain the position; returns 1
// at the first node that reports a collision, else 0. EN 0x6353f0.
extern "C" int Regions_Collide(Node *root, const char *tag, unsigned int mask, Vec4 *position, Vec4 *velocity, Vec4 *hit);
// Tests every marker point against the world's "testatari" regions. Returns 0. EN 0x5c1340.
extern "C" int Markers_TestRegions(void);

#endif
