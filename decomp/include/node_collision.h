// Globals of the model collision test (Node::CollideModel).
#ifndef GURUMIN_NODE_COLLISION_H
#define GURUMIN_NODE_COLLISION_H

// Called with (event, code, 1) when two models whose collision classes match touch: event 0x1869b = the pair passes
// the class test, 0x1869f = it fails it; `code` is the colour value (Node::m_189c) of the node that has one.
typedef void (__cdecl *NodeContactCallback)(int event, int code, int flag);

extern "C" NodeContactCallback DAT_11b044fc;   // 0 = no contact reporting

#endif
