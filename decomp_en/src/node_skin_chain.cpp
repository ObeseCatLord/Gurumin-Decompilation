// Following a node's chain of skinned parents (EN build, EN-only: no JP twin found).
#include "node.h"

// Returns the topmost node of the chain of skinned nodes this node belongs to, found through the parent links: the
// parent's answer when it has one, otherwise this node. An unskinned node, and a skinned node without a parent, answer 0.
// Calls itself up the tree.
Node *Node::GetSkinChainTop()
{
    if (!skinned)
        return 0;
    if (parent == 0)
        return 0;
    Node *top = parent->GetSkinChainTop();
    if (top == 0)
        top = this;
    return top;
}
