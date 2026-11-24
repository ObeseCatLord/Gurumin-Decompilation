// A scene node: one part of a loaded model (.it3 file) with its mesh, matrices, animation plugs and children.
// EN declaration of Node (the JP one is decomp/include/node.h, 0x18a0 bytes). Only the members EN code has used so
// far are declared; add fields here, with their EN offsets, when a matched EN function needs them, and move the JP
// field names over once the EN layout is proven to be the same (NOTES.md "EN headers").
// This is the one declaration of Node in decomp_en: include this header, never redeclare it.
#ifndef GURUMIN_EN_NODE_H
#define GURUMIN_EN_NODE_H

struct Node
{
    // Name lookups in the subtree (scene_node.cpp). `skip` counts down the matches still to pass over.
    Node *Find(const char *name, int index);                   // EN 0x7532c0 (JP 0x1019ece0)
    Node *FindRecursive(const char *name, int *skip);          // EN 0x7531d0 (JP 0x10191a30)
    Node *FindPrefixRecursive(const char *prefix, int *skip);  // EN 0x753240 (JP 0x10191c30)
};

#endif
