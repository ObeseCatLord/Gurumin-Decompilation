// Counters of the sphere-versus-mesh collision test (Node::SphereHitsMesh, src/node_sphere_hits.cpp).
#ifndef GURUMIN_NODE_COLLISION_VARS_H
#define GURUMIN_NODE_COLLISION_VARS_H

extern "C" int DAT_10f65548;  // triangles whose bounding sphere was reached by a SphereHitsMesh test
extern "C" int DAT_10f6554c;  // triangles whose plane was within the sphere's radius
extern "C" int DAT_11b002b4;  // 1 when the node last touched by a probe test is a camera node (tag "CA")

#endif
