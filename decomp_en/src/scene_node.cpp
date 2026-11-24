// Scene node methods (EN build). Ported from the matched JP source decomp/src/scene_node.cpp.
#include "node.h"

// Node::Find: the index-th node called `name` in the subtree; a leading '@' turns it into a prefix search
// ("@Spot" finds Spot01, Spot02, ...). Called by the model, effect and map code to find attachment points.
Node *Node::Find(const char *name, int index)
{
    int skip = index;
    if (*name == '@')
        return FindPrefixRecursive(name + 1, &skip);
    return FindRecursive(name, &skip);
}
