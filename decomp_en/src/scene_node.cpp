// Scene node methods (EN build). Ported from the matched JP source decomp/src/scene_node.cpp.
#include "node.h"
#include "node_data.h"
#include "index_list.h"
#include "motion_list.h"
#include <string.h>
#include "engine_memory.h"
#include "render_queue.h"

// Node::Find: the index-th node called `name` in the subtree; a leading '@' turns it into a prefix search
// ("@Spot" finds Spot01, Spot02, ...). Called by the model, effect and map code to find attachment points.
Node *Node::Find(const char *name, int index)
{
    int skip = index;
    if (*name == '@')
        return FindPrefixRecursive(name + 1, &skip);
    return FindRecursive(name, &skip);
}

// Node::Find2: like Find with the second pair of finders (FindPrefixRecursiveStripped / FindRecursiveStripped),
// which ignore the name prefixes of special nodes. Called by the model code.
Node *Node::Find2(const char *name, int index)
{
    int skip = index;
    if (*name == '@')
        return FindPrefixRecursiveStripped(name + 1, &skip);
    return FindRecursiveStripped(name, &skip);
}

// Node::RunSpecialPass: marks the special pass (MarkSpecialPass) and runs the follow-up pass
// (ClearSpecialPassOnLinks). Called by the model code before drawing.
int Node::RunSpecialPass()
{
    MarkSpecialPass();
    ClearSpecialPassOnLinks();
    return 0;
}

// Node::ClearSpecialPassUp: clears the special pass marker on this node and all its ancestors. Called by
// ClearSpecialPassOnLinks for the bone links of skinned nodes.
int Node::ClearSpecialPassUp()
{
    flags &= ~NODE_FLAG_SKIP_SUBTREE;
    if (parent != 0)
        parent->ClearSpecialPassUp();
    return 0;
}

// Node::Root: the top node of the tree this node belongs to (following the parent links). Called by the model code.
Node *Node::Root()
{
    if (parent == 0)
        return this;
    return parent->Root();
}

// Node::GetLinkedRoot: the highest ancestor reachable through parents that are chasing nodes. Called by the
// model code to find the node a chasing chain hangs from (written recursively; the compiler makes it a loop).
Node *Node::GetLinkedRoot()
{
    if (parent != 0 && (parent->flags & NODE_FLAG_CHASING) != 0)
        return parent->GetLinkedRoot();
    return this;
}

// RenderQueue_Clear: empties the render queue and its extra-data and matrix pools. Called at the start of each
// frame's drawing.
extern "C" int RenderQueue_Clear()
{
    g_renderQueueMatrixCount = 0;
    g_renderQueueExtraCount = 0;
    g_renderQueueCount = 0;
    return 0;
}

// Node::AllocSaved: allocates the node's saved-state block on first use. Called while loading and by the tracking code.
int Node::AllocSaved()
{
    if (saved == 0)
    {
        saved = (NodeSaved *)g_engineAlloc(NODE_SAVED_SIZE, "");
        saved->Reset();
    }
    return 1;
}

// Node::AllocExtra: allocates the node's animation block on first use. Called while loading the model.
int Node::AllocExtra()
{
    if (extra == 0)
    {
        extra = (NodeExtra *)g_engineAlloc(NODE_EXTRA_SIZE, "");
        extra->Init();
    }
    return 1;
}

// Node::SaveFlags: keeps the flags of the whole subtree in savedFlags. Called before a temporary state change.
void Node::SaveFlags()
{
    savedFlags = flags;
    for (int i = 0; i < childCount; i++)
        children[i]->SaveFlags();
}

// Node::SaveRenderMode: keeps the render mode of the whole subtree in savedDrawMode.
void Node::SaveRenderMode()
{
    savedDrawMode = drawMode;
    for (int i = 0; i < childCount; i++)
        children[i]->SaveRenderMode();
}

// Node::RestoreFlags: puts back the flags kept by SaveFlags on the whole subtree.
void Node::RestoreFlags()
{
    flags = savedFlags;
    for (int i = 0; i < childCount; i++)
        children[i]->RestoreFlags();
}

// Node::RestoreRenderMode: puts back the render mode kept by SaveRenderMode on the whole subtree.
void Node::RestoreRenderMode()
{
    drawMode = savedDrawMode;
    for (int i = 0; i < childCount; i++)
        children[i]->RestoreRenderMode();
}

// Node::ClearFlags: clears flag bits on the whole subtree.
int Node::ClearFlags(unsigned int bits)
{
    flags &= ~bits;
    for (int i = 0; i < childCount; i++)
        children[i]->ClearFlags(bits);
    return 1;
}

// Node::SetFlags: sets flag bits on the whole subtree.
int Node::SetFlags(unsigned int bits)
{
    flags |= bits;
    for (int i = 0; i < childCount; i++)
        children[i]->SetFlags(bits);
    return 1;
}

// Node::SetRenderMode: sets the render mode of the subtree (mode 8 is ignored for skinned nodes with bones).
int Node::SetRenderMode(int mode)
{
    if (mode == NODE_DRAW_MODE_SKIN_EXCLUDED && skin != 0 && skin->boneCount != 0)
        drawMode = 0;
    else
        drawMode = mode;
    for (int i = 0; i < childCount; i++)
        children[i]->SetRenderMode(mode);
    return 1;
}

// Node::SetModel: sets the owning model on the whole subtree.
int Node::SetModel(AnimModel *owner)
{
    model = owner;
    for (int i = 0; i < childCount; i++)
        children[i]->SetModel(owner);
    return 1;
}

// Node::ClearHasCollision: clears the collision flag in the whole subtree.
int Node::ClearHasCollision()
{
    hasCollision = 0;
    for (int i = 0; i < childCount; i++)
        children[i]->ClearHasCollision();
    return 1;
}

// Node::StartProximityPass: starts an update pass over the subtree: clears the collision flags, remembers the
// motion frame and runs the recursive pass from depth 0. Called by the model code each frame.
int Node::StartProximityPass(float *position, Matrix4 *matrix, Matrix4 *reference, int frame, int b)
{
    ClearHasCollision();
    g_updatePassFrame = frame;
    UpdateWorldWithIk(position, matrix, reference, frame, b, 0);
    return 1;
}

// Node::GetLastFrame: the last animation frame of the subtree (the highest frame count of any node).
int Node::GetLastFrame()
{
    int last = frameMatrixCount;
    if (last < frameCount)
        last = frameCount;
    for (int i = 0; i < childCount; i++)
    {
        int child = children[i]->GetLastFrame();
        if (child > last)
            last = child;
    }
    return last;
}

// Node::CycleChild: advances the cycling child index (1..childCount) in the whole subtree.
int Node::CycleChild()
{
    if (cycleIndex != 0)
    {
        cycleIndex++;
        if (cycleIndex - 1 >= childCount)
            cycleIndex = 1;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->CycleChild();
    return 0;
}

// Node::RestartColourPlugs: restarts the two colour plugs with motion `motion` over `frames` frames.
int Node::RestartColourPlugs(const char *motion, int frames)
{
    for (int i = 0; i < PLUG_COLOUR_COUNT; i++)
    {
        if (plugType[PLUG_COLOUR_FIRST + i] != 0 && plugType[PLUG_COLOUR_FIRST + i] == PLUG_TYPE_FRAME)
        {
            plugs[PLUG_COLOUR_FIRST + i]->keys.Clear();
            plugs[PLUG_COLOUR_FIRST + i]->keys.InsertByName(0, motion, frames);
        }
    }
    return 0;
}

// Node::TagElements: tags nodes named "fire..." and "thun..." with their element in the whole subtree.
int Node::TagElements()
{
    if (_strnicmp(name, "fire", NODE_ELEMENT_PREFIX_LENGTH) == 0)
        element = NODE_ELEMENT_FIRE;
    if (_strnicmp(name, "thun", NODE_ELEMENT_PREFIX_LENGTH) == 0)
        element |= NODE_ELEMENT_THUNDER;
    for (int i = 0; i < childCount; i++)
        children[i]->TagElements();
    return 0;
}

// IndexLists_Free: frees `count` index lists in a row. Called by FreeExtraIndexLists.
static void IndexLists_Free(IndexList *lists, int count)
{
    for (int i = 0; i < count; i++)
        lists[i].Free();
}

// Node::FreeExtraIndexLists: frees the triangle sphere lists of this node, then runs BuildCollisionSpheres on the
// children. Called while releasing a model.
int Node::FreeExtraIndexLists()
{
    if (triangleSpheres != 0 && packCount != 0)
        IndexLists_Free(triangleSpheres, packCount);
    for (int i = 0; i < childCount; i++)
        children[i]->BuildCollisionSpheres();
    return 0;
}

// Node::RestoreState1000: puts back the saved matrix when state 0x1000 (chasing node) is set and clears that state.
int Node::RestoreState1000()
{
    if (saved != 0 && (flags & NODE_FLAG_CHASING) != 0)
    {
        matrix = saved->matrix;
        flags &= ~NODE_FLAG_CHASING;
    }
    return 1;
}

// Node::RestoreState2000: puts back the saved matrix when state 0x2000 (two-bone limb) is set, clears it and releases
// the saved states of the subtree.
int Node::RestoreState2000()
{
    if (saved != 0 && (flags & NODE_FLAG_TWO_BONE_LIMB) != 0)
    {
        matrix = saved->matrix;
        flags &= ~NODE_FLAG_TWO_BONE_LIMB;
        ReleaseSavedStates();
    }
    return 1;
}

// Node::RestoreState400000: puts back the saved matrix when state 0x400000 (target tracking) is set and clears it.
int Node::RestoreState400000()
{
    if (saved != 0 && (flags & NODE_FLAG_TARGET_TRACKING) != 0)
    {
        matrix = saved->matrix;
        flags &= ~NODE_FLAG_TARGET_TRACKING;
    }
    return 1;
}

// Node::FreeSaved: undoes every saved state (clearing the collision sphere state on the way) and frees the
// saved-state block. Called when the node is released.
int Node::FreeSaved()
{
    if (saved != 0)
    {
        RestoreState1000();
        RestoreState2000();
        if (saved != 0 && (flags & NODE_FLAG_COLLISION_SPHERE) != 0)
        {
            flags &= ~NODE_FLAG_COLLISION_SPHERE;
            ReleaseSavedStates();
        }
        RestoreState400000();
        g_engineFree(saved);
        saved = 0;
    }
    return 1;
}
