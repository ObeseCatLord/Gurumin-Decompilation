// Node child list and subtree searches (EN build, EN-only: no JP twin found).
#include "node.h"

// Collects the nodes of this subtree that are in the collision-sphere state and links them to the chasing nodes (the
// same two steps Rebuild runs). Returns 1. No direct caller.
int Node::LinkTrackingNodes()
{
    Node *tracked[NODE_TRACKED_MAX];
    int trackedCount = 0;
    CollectTrackingNodes(tracked, &trackedCount);
    if (trackedCount != 0)
        LinkToChasing(tracked, trackedCount);
    return 1;
}

// Appends the existing node `child` to this node's child list, makes this node its parent and returns the node just
// appended. The caller keeps the list below NODE_MAX_CHILDREN. No direct caller.
Node *Node::AttachChild(Node *child)
{
    children[childCount] = child;
    children[childCount]->parent = this;
    childCount++;
    return children[childCount - 1];
}

// Returns this node when it has skinning data, otherwise the first descendant (depth first, in child order) that has
// some; 0 when the subtree has none. Calls itself on every child. No direct caller outside the recursion.
Node *Node::FindSkinned()
{
    if (skin != 0)
        return this;
    for (int i = 0; i < childCount; i++)
    {
        Node *found = children[i]->FindSkinned();
        if (found != 0)
            return found;
    }
    return 0;
}
