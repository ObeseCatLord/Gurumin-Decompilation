// EN-only tiny node method without a JP twin (batch 11, section 4).
#include "node.h"

// Finds the first node of this subtree whose name starts with `prefix` (case-insensitive). Called by the loaders that
// look nodes up by name prefix.
Node *Node::FindPrefixNode(const char *prefix)
{
    return FindPrefixNodeRecursive(prefix);
}
