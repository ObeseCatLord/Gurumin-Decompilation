// Whole-subtree passes of Node that restore, link and tag nodes (EN build, EN-only: no JP twin found for these).
#include <string.h>
#include "node.h"
#include "node_data.h"
#include "motion_list.h"
#include "visibility.h"
#include "engine_memory.h"

// Indices of the two links of a skinned node whose bones are tried in order.
const int SKIN_LINK_FIRST = 0;
const int SKIN_LINK_SECOND = 1;

// Node::LinkToChasing: stores the `count` nodes in `nodes` in the saved-state block of every chasing node of the
// subtree (they are the nodes the chasing node follows). Returns 1. Called by LinkTrackingNodes (EN 0x76ea50) and
// Node::Rebuild.
int Node::LinkToChasing(Node *const *nodes, int count)
{
    if ((flags & NODE_FLAG_CHASING) != 0)
    {
        for (int i = 0; i < count; i++)
            saved->chaseLinks[i] = nodes[i];
        saved->chaseLinkCount = count;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->LinkToChasing(nodes, count);
    return 1;
}

// Node::RestorePlugData: for every plug that has a private copy of its channel (made by CopyPlugData), frees the
// channel the plug points at, makes a new one from the copy and takes the copy's plug type back; then does the same
// for the children. Returns 0. No direct caller.
int Node::RestorePlugData()
{
    for (int i = 0; i < NODE_PLUG_COUNT; i++)
    {
        if (plugData[i] != 0)
        {
            if (plugs[i] != 0)
                g_engineFree(plugs[i]);
            const Channel *copy = (const Channel *)plugData[i];
            int size = copy->size;
            plugs[i] = (Channel *)g_engineAlloc(size, "->m_plugdata[g]");
            memcpy(plugs[i], plugData[i], size);
            plugType[i] = plugTypeCopy[i];
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->RestorePlugData();
    return 0;
}

// Node::StoreLinkedBoneInverses: for every skinned node of the subtree that has bones, asks the bone of its first link
// (or of its second link when the first has none) to store its inverse bone matrix as entry 0 of this node's skin.
// Returns 1. No direct caller.
int Node::StoreLinkedBoneInverses()
{
    if (skin != 0 && skin->boneCount != 0)
    {
        int boneIndex = 0;
        if (skin->links[SKIN_LINK_FIRST]->skin->bone != 0)
            skin->links[SKIN_LINK_FIRST]->skin->bone->StoreInverseBone(this, &boneIndex);
        else if (skin->links[SKIN_LINK_SECOND] != 0)
            skin->links[SKIN_LINK_SECOND]->skin->bone->StoreInverseBone(this, &boneIndex);
    }
    for (int i = 0; i < childCount; i++)
        children[i]->StoreLinkedBoneInverses();
    return 1;
}

// Node::TagRegionsInTree: runs TagRegion on every node of the subtree that has NODE_FLAG_COMMAND_TARGET set, with the
// region boxes and cells of `boxes` / `cells`. Called with both 0, the data comes from the visibility cells of this
// map root node (nothing is done when it has none). Returns 1, or 0 when called with 0 on a node without cells. No
// direct caller.
int Node::TagRegionsInTree(const Box2 *boxes, const VisCell *cells)
{
    if (boxes == 0)
    {
        VisData *data = visibilityCells;
        if (data == 0)
            return 0;
        cells = data->cells;
        boxes = data->boxes;
    }
    if ((flags & NODE_FLAG_COMMAND_TARGET) != 0)
        TagRegion(boxes, cells, 0);
    for (int i = 0; i < childCount; i++)
        children[i]->TagRegionsInTree(boxes, cells);
    return 1;
}
