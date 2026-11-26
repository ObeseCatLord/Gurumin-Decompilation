// Freeing of a node's animation tracks block (EN build, EN-only: no JP twin found).
#include "engine_memory.h"
#include "node.h"
#include "node_data.h"

// Node::FreeExtra: frees the animation tracks block of the node (its arrays first) and forgets it. Does nothing when the
// node has none. No direct caller.
void Node::FreeExtra()
{
    if (extra != 0)
    {
        extra->Clear();
        g_engineFree(extra);
    }
    extra = 0;
}
