// Scene node methods (EN build). Ported from the matched JP source decomp/src/scene_node.cpp.
#include "node.h"
#include "node_data.h"
#include "index_list.h"
#include "slot_table.h"
#include "motion_list.h"
#include <string.h>
#include "engine_memory.h"
#include "render_queue.h"
#include "containers.h"
#include "mesh_cache.h"
#include <stdio.h>
#include <stdlib.h>

// Highest override matrix a negative frame of FindBone can pick.
const int BONE_FRAME_LIMIT = 15;

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
        lists[i].FreeSpheres();
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

// Node::ReplaceMaterial: runs the material swap on this node's mesh when it has materials and packed data (or no
// packs at all with packCount -1), then on every child. Returns 1. Called by AnimModel::ApplyMaterialSwap.
int Node::ReplaceMaterial(int key, int count)
{
    if (materialCount != 0)
    {
        if (packCount == -1 && packs == 0 && mesh != 0)
            mesh->ReplaceMaterial(key, count);
        else if (packCount != 0 && packs != 0 && mesh != 0)
            mesh->ReplaceMaterial(key, count);
    }
    for (int i = 0; i < childCount; i++)
        children[i]->ReplaceMaterial(key, count);
    return 1;
}

// Node::FindRecursiveStripped: like FindRecursive, but compares the name without the prefix of special nodes: collision
// ("CDL", kinds below 0x40) names skip 6 characters and overlay ("OVF", kind 0xfd) names 7. Nodes without a kind
// (0xff) are recognised by the prefix itself. Called by Find2.
Node *Node::FindRecursiveStripped(const char *name, int *skip)
{
    int offset = 0;
    if (kind != NODE_KIND_NONE)
    {
        if (kind < NODE_KIND_REGION_LIMIT)
            offset = NODE_COLLISION_PREFIX_LENGTH;
        else if (kind == NODE_KIND_OVERLAY)
            offset = NODE_OVERLAY_PREFIX_LENGTH;
    }
    else
    {
        if (this->name[0] == 'C' && this->name[1] == 'D' && this->name[2] == 'L')
            offset = NODE_COLLISION_PREFIX_LENGTH;
        if (this->name[0] == 'O' && this->name[1] == 'V' && this->name[2] == 'F')
            offset = NODE_OVERLAY_PREFIX_LENGTH;
    }
    if (_stricmp(this->name + offset, name) == 0)
    {
        if (skip == 0)
            return this;
        if (*skip == 0)
            return this;
        (*skip)--;
    }
    for (int i = 0; i < childCount; i++)
    {
        if (children[i] != 0)
        {
            Node *found = children[i]->FindRecursiveStripped(name, skip);
            if (found != 0)
                return found;
        }
    }
    return 0;
}

// Node::FindPrefixRecursiveStripped: prefix version of FindRecursiveStripped (names that start with `prefix` after the
// special-node prefix). Called by Find2.
Node *Node::FindPrefixRecursiveStripped(const char *prefix, int *skip)
{
    int offset = 0;
    if (kind != NODE_KIND_NONE)
    {
        if (kind < NODE_KIND_REGION_LIMIT)
            offset = NODE_COLLISION_PREFIX_LENGTH;
        else if (kind == NODE_KIND_OVERLAY)
            offset = NODE_OVERLAY_PREFIX_LENGTH;
    }
    else
    {
        if (name[0] == 'C' && name[1] == 'D' && name[2] == 'L')
            offset = NODE_COLLISION_PREFIX_LENGTH;
        if (name[0] == 'O' && name[1] == 'V' && name[2] == 'F')
            offset = NODE_OVERLAY_PREFIX_LENGTH;
    }
    if (_strnicmp(name + offset, prefix, strlen(prefix)) == 0)
    {
        if (skip == 0)
            return this;
        if (*skip == 0)
            return this;
        (*skip)--;
    }
    for (int i = 0; i < childCount; i++)
    {
        if (children[i] != 0)
        {
            Node *found = children[i]->FindPrefixRecursiveStripped(prefix, skip);
            if (found != 0)
                return found;
        }
    }
    return 0;
}

// Node::FindRecursive: the node called `name` (case-insensitive) in this subtree, depth first, this node first. With
// `skip` set, the first *skip matches are passed over (and *skip counts down), so repeated calls find the second,
// third, ... node of that name. Returns 0 when there is no (further) match. Called by Find and the renaming code.
Node *Node::FindRecursive(const char *name, int *skip)
{
    if (_stricmp(this->name, name) == 0)
    {
        if (skip == 0)
            return this;
        if (*skip == 0)
            return this;
        (*skip)--;
    }
    for (int i = 0; i < childCount; i++)
    {
        if (children[i] != 0)
        {
            Node *found = children[i]->FindRecursive(name, skip);
            if (found != 0)
                return found;
        }
    }
    return 0;
}

// Node::FindPrefixRecursive: like FindRecursive, but matches nodes whose name starts with `prefix` (case-insensitive),
// e.g. "@Spot" finds "@Spot01" and "@Spot02". Called by Find.
Node *Node::FindPrefixRecursive(const char *prefix, int *skip)
{
    if (_strnicmp(name, prefix, strlen(prefix)) == 0)
    {
        if (skip == 0)
            return this;
        if (*skip == 0)
            return this;
        (*skip)--;
    }
    for (int i = 0; i < childCount; i++)
    {
        if (children[i] != 0)
        {
            Node *found = children[i]->FindPrefixRecursive(prefix, skip);
            if (found != 0)
                return found;
        }
    }
    return 0;
}

// Node::FindByFlags: the nth node (0-based) in the subtree whose flags contain every bit of `mask`; `counter` carries the
// number of matches seen so far across the recursion (0 on the first call). Called by the model code.
Node *Node::FindByFlags(unsigned int mask, int nth, int *counter)
{
    int local = 0;
    if (counter == 0)
        counter = &local;
    if ((flags & mask) == mask)
    {
        if (++*counter > nth)
            return this;
    }
    for (int i = 0; i < childCount; i++)
    {
        if (children[i] != 0)
        {
            Node *found = children[i]->FindByFlags(mask, nth, counter);
            if (found != 0)
                return found;
        }
    }
    return 0;
}

// Node::MarkSpecialPass: marks this node for the special pass (NODE_FLAG_SKIP_SUBTREE) when it is hidden or has skinning
// data, unless it or its parent is in one of the excluded modes; otherwise passes the request on to the children.
// Returns 0 when this node was marked, 1 otherwise. Called by RunSpecialPass.
int Node::MarkSpecialPass()
{
    if ((flags & NODE_FLAG_SPECIAL_SHAPES) == 0 && (parent == 0 || (parent->flags & NODE_FLAG_LIMB_STATES) == 0) &&
        ((flags & NODE_FLAG_HIDDEN) != 0 || skinned != 0))
    {
        flags |= NODE_FLAG_SKIP_SUBTREE;
        return 0;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->MarkSpecialPass();
    return 1;
}

// Node::ClearSpecialPassOnLinks: follow-up of MarkSpecialPass: clears the marker again on nodes in the limb states
// (0x2000 and 0x1000), clears it on every node a skinned node depends on (and their ancestors), and recurses into the
// children. Returns 1. Called by RunSpecialPass.
int Node::ClearSpecialPassOnLinks()
{
    if (flags & NODE_FLAG_TWO_BONE_LIMB)
        flags &= ~NODE_FLAG_SKIP_SUBTREE;
    if (flags & NODE_FLAG_CHASING)
        flags &= ~NODE_FLAG_SKIP_SUBTREE;
    if (skin != 0 && skin->boneCount != 0)
    {
        for (int i = 0; i < skin->boneCount; i++)
            skin->links[i]->ClearSpecialPassUp();
    }
    for (int i = 0; i < childCount; i++)
        children[i]->ClearSpecialPassOnLinks();
    return 1;
}

// Node::SetColourA: sets colourA on this node when its flags match `mask` and it is one of the shape modes
// (0x400, 0x200, 0x800), then does the same on every child. Returns 0.
int Node::SetColourA(unsigned char value, unsigned int mask)
{
    if ((flags & mask) != 0)
    {
        if ((flags & NODE_FLAG_SHAPE_SPHERE) != 0)
            colourA = value;
        if ((flags & NODE_FLAG_SHAPE_BOX) != 0)
            colourA = value;
        if ((flags & NODE_FLAG_SHAPE_SOLID_BOX) != 0)
            colourA = value;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->SetColourA(value, mask);
    return 0;
}

// Node::SetColourB: like SetColourA for colourB.
int Node::SetColourB(unsigned char value, unsigned int mask)
{
    if ((flags & mask) != 0)
    {
        if ((flags & NODE_FLAG_SHAPE_SPHERE) != 0)
            colourB = value;
        if ((flags & NODE_FLAG_SHAPE_BOX) != 0)
            colourB = value;
        if ((flags & NODE_FLAG_SHAPE_SOLID_BOX) != 0)
            colourB = value;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->SetColourB(value, mask);
    return 0;
}

// Node::SetColourC: like SetColourA for colourC.
int Node::SetColourC(char value, unsigned int mask)
{
    if ((flags & mask) != 0)
    {
        if ((flags & NODE_FLAG_SHAPE_SPHERE) != 0)
            colourC = value;
        if ((flags & NODE_FLAG_SHAPE_BOX) != 0)
            colourC = value;
        if ((flags & NODE_FLAG_SHAPE_SOLID_BOX) != 0)
            colourC = value;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->SetColourC(value, mask);
    return 0;
}

// Node::SetRenderModeExcept: sets the render mode of the subtree except for nodes in mode `except1` or `except2`
// (-1 = no exception). Returns 1.
int Node::SetRenderModeExcept(int value, int except1, int except2)
{
    if ((except1 == NODE_NO_RENDER_MODE || drawMode != except1) && (except2 == NODE_NO_RENDER_MODE || drawMode != except2))
        drawMode = value;
    for (int i = 0; i < childCount; i++)
        children[i]->SetRenderModeExcept(value, except1, except2);
    return 1;
}

// Node::CopyPlugData: makes private copies of every animation plug's channel data in the subtree. Returns 0.
int Node::CopyPlugData()
{
    for (int i = 0; i < NODE_PLUG_COUNT; i++)
    {
        if (plugs[i] != 0)
        {
            int size = plugs[i]->size;
            if (plugData[i] != 0)
                g_engineFree(plugData[i]);
            plugData[i] = g_engineAlloc(size, "->m_plugdata[g]");
            memcpy(plugData[i], plugs[i], size);
            plugTypeCopy[i] = plugType[i];
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->CopyPlugData();
    return 0;
}

// Node::ReleaseTexture: drops the node's shared texture reference and frees the texture data when nobody else uses it.
// Called by ReleaseTexturesExcept and while releasing a model.
void Node::ReleaseTexture()
{
    if (TextureFile_Release(textureRef) == 0)
    {
        g_engineFree(textureRef);
        textureRef = 0;
        if (frameMatrices != 0)
            g_engineFree(frameMatrices);
        frameMatrices = 0;
        frameMatrixCount = 0;
    }
    frameMatrices = 0;
    frameMatrixCount = 0;
    textureRef = 0;
}

// Node::ReleaseTexturesExcept: releases the textures of every node in the subtree that is not in the `keep` list.
// Returns 1.
int Node::ReleaseTexturesExcept(Node **keep, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (this == keep[i])
            break;
    }
    if (i == count && textureRef != 0)
        ReleaseTexture();
    for (int k = 0; k < childCount; k++)
        children[k]->ReleaseTexturesExcept(keep, count);
    return 1;
}

// Node::SetMaterialValue: sets `value` on every mesh subset that uses the material called `material`, in the whole
// subtree. Returns 1.
int Node::SetMaterialValue(const char *material, int value)
{
    if (materialCount != 0)
    {
        for (int m = 0; m < materialCount; m++)
        {
            if (_stricmp(materials[m].name, material) == 0 && packCount != 0 && packs != 0 && mesh != 0)
            {
                for (int i = 0; i < packCount; i++)
                {
                    if (packs[i].material == m)
                        mesh->SetMaterial(i, value);
                }
            }
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->SetMaterialValue(material, value);
    return 1;
}

// Node::MakeNameUnique: makes this node's name unique in its tree (called when a child is added): the node first takes
// a random name "uni<rand><rand>" so that the search does not find the node itself, then gets its old name, or
// "name(1)", "name(2)", ... when another node already has it.
void Node::MakeNameUnique()
{
    char unique[NODE_NAME_SCRATCH_SIZE];
    char base[NODE_NAME_SCRATCH_SIZE];
    Node *top = Root();
    strcpy(base, name);
    strcpy(unique, name);
    sprintf(name, "uni%d%d", rand(), rand());
    int number = 1;
    while (top->Find(unique, 0) != 0)
    {
        sprintf(unique, "%s(%d)", base, number);
        number++;
    }
    strcpy(name, unique);
}

// Node::SetUniqueName: names this node `name`, or "name(0)", "name(1)", ... (the first that no node of the tree uses
// yet) when the tree already has a node called `name`. Returns 1. Called while building the node tree.
int Node::SetUniqueName(const char *name)
{
    char unique[NODE_NAME_SCRATCH_SIZE];
    int number = 0;
    Node *top = Root();
    if (top->Find(name, 0) == 0)
    {
        strcpy(this->name, name);
        return 1;
    }
    sprintf(unique, "%s(%d)", name, 0);
    while (top->Find(unique, 0) != 0)
    {
        number++;
        sprintf(unique, "%s(%d)", name, number);
    }
    strcpy(this->name, unique);
    return 1;
}

// Node::Rename: renames a node and every reference to it. Called with `oldName` = 0 on the node itself: the whole tree
// (from its root) replaces the node's current name by `newName` in the skinning names (bound-to name, link names, bone
// names), then the node takes the new name (returns 0). Called with a name, it does the replacing in this node and all
// nodes below it (returns 1).
int Node::Rename(const char *newName, const char *oldName)
{
    if (oldName == 0)
    {
        Node *top = Root();
        if (top != 0)
        {
            top->Rename(newName, name);
            strcpy(name, newName);
            return 0;
        }
        Rename(newName, name);
        strcpy(name, newName);
        return 0;
    }
    if (skin != 0)
    {
        if (_stricmp(skin->boneName, oldName) == 0)
            strcpy(skin->boneName, newName);
        int i;
        for (i = 0; i < SKIN_MAX_LINKS; i++)
        {
            if (_stricmp(skin->linkNames[i], oldName) == 0)
                strcpy(skin->linkNames[i], newName);
        }
        for (i = 0; i < SKIN_MAX_BONES; i++)
        {
            if (_stricmp(skin->boneNames[i], oldName) == 0)
                strcpy(skin->boneNames[i], newName);
        }
    }
    for (int j = 0; j < childCount; j++)
        children[j]->Rename(newName, oldName);
    return 1;
}

// Node::TagRegion: finds the first of the 64 regions (`boxes`, with names in `names`; an empty name = unused) that
// contains the position of `matrix`, or of this node when `matrix` is 0, and prefixes this node's name with
// "CDL<region>-". Returns 1 when a region was found, else 0. Called while loading a map.
int Node::TagRegion(const Box2 *boxes, const char (*names)[NODE_NAME_SIZE], Matrix4 *matrix)
{
    char tagged[NODE_NAME_SCRATCH_SIZE];
    int i;
    if (matrix != 0)
    {
        for (i = 0; i < NODE_REGION_COUNT; i++)
        {
            if (names[i][0] != 0 && matrix->GetTranslation().InBox(&boxes[i]) != 0)
            {
                sprintf(tagged, "CDL%02d-%s", i, name);
                sprintf(name, tagged);
                return 1;
            }
        }
    }
    else
    {
        for (i = 0; i < NODE_REGION_COUNT; i++)
        {
            if (names[i][0] != 0 && worldMatrix.GetTranslation().InBox(&boxes[i]) != 0)
            {
                sprintf(tagged, "CDL%02d-%s", i, name);
                sprintf(name, tagged);
                return 1;
            }
        }
    }
    return 0;
}

// Node::EnterTargetState: enters the target-tracking state (saved matrix 0x400000) with `target` (or the default
// target object) as the object to follow. Any other saved state is left first. Returns 1.
int Node::EnterTargetState(void *target)
{
    if (saved != 0)
    {
        if (target != 0)
            targetObject = target;
        else
            targetObject = g_defaultTargetObject;
        if ((flags & NODE_FLAG_TARGET_TRACKING) != 0)
        {
            if (saved->state == SAVED_RETURNING)
                saved->state = SAVED_HOLDING;
        }
        else
        {
            RestoreState1000();
            RestoreState2000();
            if (saved != 0 && (flags & NODE_FLAG_COLLISION_SPHERE) != 0)
            {
                flags &= ~NODE_FLAG_COLLISION_SPHERE;
                ReleaseSavedStates();
            }
            RestoreState400000();
            saved->Reset();
            flags |= NODE_FLAG_TARGET_TRACKING;
        }
    }
    return 1;
}

// Node::Rebuild: rebuilds the subtree's links after loading: collects linked nodes, rebinds skins, copies the bind
// poses, collects the nodes in the collision-sphere state and links them, then stores the links. Returns 1.
int Node::Rebuild()
{
    int linkCount = 0;
    Node *links[NODE_LINKS_MAX];
    BindLinks(this, links, &linkCount);
    SetupSkins(this);
    CopyBindPoses();
    int trackedCount = 0;
    Node *tracked[NODE_TRACKED_MAX];
    CollectTrackingNodes(tracked, &trackedCount);
    if (trackedCount != 0)
        LinkToChasing(tracked, trackedCount);
    StoreLinks(links, &linkCount);
    return 1;
}

// Node::ReleaseUniqueMeshData: frees the pack and index data of every node in this subtree whose mesh buffer is not
// shared (its model file has a single user), and resets the file's slot. Nodes are skipped when their name (after a
// "CDL..." or "OvF..." prefix) starts with "CA", "CS" or "CT", when they are flagged or have no mesh buffer.
void Node::ReleaseUniqueMeshData()
{
    int skip = 0;
    if (name[0] == 'C' && name[1] == 'D' && name[2] == 'L')
        skip = NODE_COLLISION_PREFIX_LENGTH;
    if (name[0] == 'O' && name[1] == 'v' && name[2] == 'F')
        skip = NODE_OVERLAY_PREFIX_LENGTH;
    if (!((name[skip] == 'C' && name[skip + 1] == 'A') || (name[skip] == 'C' && name[skip + 1] == 'S') ||
          (name[skip] == 'C' && name[skip + 1] == 'T')) &&
        (savedFlags & NODE_SAVED_FLAG_SHARED) == 0 && (flags & NODE_FLAG_KEEP_MESH) == 0 &&
        (flags & NODE_FLAG_SPECIAL_PASSES) == 0 && mesh != 0 && mesh->bufferSlot >= 0 &&
        g_modelFileRefCount[mesh->bufferSlot] == 1)
    {
        g_modelFilePackCount[mesh->bufferSlot] = -1;
        g_modelFilePacks[mesh->bufferSlot] = 0;
        g_modelFileIndexLists[mesh->bufferSlot] = 0;
        FreePackData(packs, packCount);
        FreeIndexListData(indexLists, packCount);
        if (triangleSpheres != 0)
        {
            FreeSphereListData(triangleSpheres, packCount);
            g_engineFree(triangleSpheres);
            triangleSpheres = 0;
        }
        if (packs != 0)
            g_engineFree(packs);
        packs = 0;
        if (indexLists != 0)
            g_engineFree(indexLists);
        indexLists = 0;
        packCount = -1;
    }
    for (int i = 0; i < childCount; i++)
        children[i]->ReleaseUniqueMeshData();
}

// Node::FreePackData: frees the vertex buffer of each of `count` vertex packs. Called while a node releases its
// packs (`this` is not used).
void Node::FreePackData(VertexPack *packList, int packTotal)
{
    if (packList != 0)
    {
        for (int i = 0; i < packTotal; i++)
            packList[i].FreeVertices();
    }
}

// Node::FreeIndexListData: frees the index buffer of each of `count` index lists (`this` is not used).
void Node::FreeIndexListData(IndexList *lists, int listTotal)
{
    if (lists != 0)
    {
        for (int i = 0; i < listTotal; i++)
            lists[i].FreeIndices();
    }
}

// Node::FreeSphereListData: frees the sphere buffer of each of `count` triangle sphere lists (`this` is not used).
void Node::FreeSphereListData(IndexList *lists, int listTotal)
{
    if (lists != 0)
    {
        for (int i = 0; i < listTotal; i++)
            lists[i].FreeSpheres();
    }
}

// Material::SetBlank: turns the material into a blank one: no name or textures, colour (r, g, b), alpha 0 and the first
// edge flag `edgeFlag`. Called for each new material of a node (Node::AllocMaterials, which EN inlines this into).
void Material::SetBlank(float r, float g, float b, char edgeFlag)
{
    colour[0] = r;
    colour[1] = g;
    m_0174 = MATERIAL_BLANK_VALUE;
    colour[2] = b;
    textures[3][0] = 0;
    textures[2][0] = 0;
    textures[1][0] = 0;
    textures[0][0] = 0;
    name[0] = 0;
    edge[3] = 0;
    edge[2] = 0;
    edge[1] = 0;
    m_0170 = 0;
    m_016c = 0;
    m_0168 = 0;
    m_0164 = 0;
    m_0160 = 0;
    m_015c = 0;
    m_0154 = 0;
    m_0150 = 0;
    m_014c = 0;
    alpha = MATERIAL_BLANK_ALPHA;
    edge[0] = edgeFlag;
}

// Node::RestoreSavedMatrix: puts the saved matrix back when the saved block holds one and marks the block empty (the
// steps of ReleaseSavedStates; inline in the original).
__forceinline void Node::RestoreSavedMatrix()
{
    if (saved->state != SAVED_EMPTY)
        matrix = saved->matrix;
    saved->state = SAVED_EMPTY;
}

// Node::ReleaseSavedStates: releases the saved matrix states of the whole subtree: the limb, two-bone and tracking
// states put their saved matrix back, the collision-sphere state just empties the block, and the temporary states
// 0x40000 and 0x80000 are cleared and free the saved block. Returns 1. Called while releasing and restarting models.
int Node::ReleaseSavedStates()
{
    if (saved != 0)
    {
        if (flags & NODE_FLAG_CHASING)
            RestoreSavedMatrix();
        if (flags & NODE_FLAG_COLLISION_SPHERE)
            saved->state = SAVED_EMPTY;
        if (flags & NODE_FLAG_TWO_BONE_LIMB)
            RestoreSavedMatrix();
        if (flags & NODE_FLAG_TARGET_TRACKING)
            RestoreSavedMatrix();
        if (flags & NODE_FLAG_TEMPORARY_A)
        {
            flags &= ~NODE_FLAG_TEMPORARY_A;
            RestoreSavedMatrix();
            FreeSaved();
        }
        else if (flags & NODE_FLAG_TEMPORARY_B)
        {
            flags &= ~NODE_FLAG_TEMPORARY_B;
            RestoreSavedMatrix();
            FreeSaved();
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->ReleaseSavedStates();
    return 1;
}

// Node::CollectStatic: called with no collector on the root, which becomes the collector: every node below it gets its
// kind from its name ("CDLnn..." = nn, "OvF..." = overlay, else NODE_KIND_PLAIN) and nodes with mesh data that are
// flagged (flags or savedFlags bit 4) or whose name (after the prefix) starts with "CS", "CA" or "CT" are marked static
// and appended to the collector's list ("FCHILD", allocated on first use). Called while loading a map.
void Node::CollectStatic(Node *collector)
{
    int i;
    if (collector == 0)
    {
        isStatic = 1;
        for (i = 0; i < childCount; i++)
            children[i]->CollectStatic(this);
        return;
    }
    int skip = 0;
    kind = NODE_KIND_PLAIN;
    if (name[0] == 'C' && name[1] == 'D' && name[2] == 'L')
    {
        kind = (name[3] - '0') * 10 + name[4] - '0';
        skip = NODE_COLLISION_PREFIX_LENGTH;
    }
    if (name[0] == 'O' && name[1] == 'v' && name[2] == 'F')
    {
        kind = NODE_KIND_OVERLAY;
        skip = NODE_OVERLAY_PREFIX_LENGTH;
    }
    if (packs == 0 || indexLists == 0 || packCount == 0 ||
        ((flags & NODE_FLAG_KEEP_MESH) == 0 && (savedFlags & NODE_SAVED_FLAG_SHARED) == 0 &&
         (name[skip] != 'C' || (name[skip + 1] != 'S' && name[skip + 1] != 'A' && name[skip + 1] != 'T'))))
    {
        for (i = 0; i < childCount; i++)
            children[i]->CollectStatic(collector);
        return;
    }
    isStatic = 1;
    if (collector->staticNodes == 0)
        collector->staticNodes = (Node **)g_engineAlloc(NODE_STATIC_LIST_SIZE, "FCHILD");
    collector->staticNodes[collector->staticCount] = this;
    collector->staticCount++;
    for (i = 0; i < childCount; i++)
        children[i]->CollectStatic(collector);
}

// Node::BlendFrames: sets the current pose of this node and its subtree to frame `from` (a stored pose for -1, -2, ...)
// blended towards frame `to` by `t`. The skeleton root "Bip01" (while an important model changes motion) and "charbase..."
// nodes blend their rotation (mode 1; charbase only while an important model changes motion). Returns 0 for no node,
// else 1. Called by the model code while it blends two motions. `t` is the blend weight (a float) passed through as its
// 32 bits: the EN code only matches with an integer-typed parameter, so it is declared int here and in MatrixBlend.
int Node::BlendFrames(int from, int to, int t)
{
    if (this == 0)
        return 0;
    if (frameMatrices != 0)
    {
        if (from < 0)
        {
            if (frameMatrixCount > to)
            {
                int pose = -1 - from;
                if (g_importantMotionChange != 0 && _stricmp(name, "Bip01") == 0)
                    MatrixBlend(&poses[0], poses[pose], frameMatrices[to], t, BLEND_ROTATION);
                else if (_strnicmp(name, "charbase", NODE_CHARBASE_PREFIX_LENGTH) == 0)
                {
                    if (g_importantMotionChange != 0)
                        MatrixBlend(&poses[0], poses[pose], frameMatrices[to], t, BLEND_ROTATION);
                    else
                        MatrixBlend(&poses[0], poses[pose], frameMatrices[to], t, BLEND_LINEAR);
                }
                else
                    MatrixBlend(&poses[0], poses[pose], frameMatrices[to], t, BLEND_LINEAR);
            }
        }
        else if (frameMatrixCount > from && frameMatrixCount > to)
        {
            if (g_importantMotionChange != 0 && _stricmp(name, "Bip01") == 0)
                MatrixBlend(&poses[0], frameMatrices[from], frameMatrices[to], t, BLEND_ROTATION);
            else if (_strnicmp(name, "charbase", NODE_CHARBASE_PREFIX_LENGTH) == 0)
            {
                if (g_importantMotionChange != 0)
                    MatrixBlend(&poses[0], frameMatrices[from], frameMatrices[to], t, BLEND_ROTATION);
                else
                    MatrixBlend(&poses[0], frameMatrices[from], frameMatrices[to], t, BLEND_LINEAR);
            }
            else
                MatrixBlend(&poses[0], frameMatrices[from], frameMatrices[to], t, BLEND_LINEAR);
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->BlendFrames(from, to, t);
    return 1;
}

// Node::ReleaseMesh: drops this node's reference to its shared mesh and, when it was the last user, frees the mesh,
// the vertex packs and the index lists. Called by Release.
void Node::ReleaseMesh()
{
    if (mesh != 0 && MeshCache_Release(mesh) == 0)
    {
        mesh->Release();
        g_engineFree(mesh);
        mesh = 0;
        FreePackData(packs, packCount);
        FreeIndexListData(indexLists, packCount);
        if (packs != 0)
            g_engineFree(packs);
        packs = 0;
        if (indexLists != 0)
            g_engineFree(indexLists);
        indexLists = 0;
        packCount = 0;
    }
}

// Node::BuildMesh: builds the Direct3D buffers of the node from its packs (or from its index lists alone when it has
// no packs), unless it is a bone, and marks it as built; with `recurse` the whole subtree follows. Returns 1. Called
// after a model file is loaded.
int Node::BuildMesh(int recurse, int bufferFlags)
{
    if (packs == 0 && packCount == -1)
    {
        if (skinned == 0)
        {
            mesh->BuildBuffers(0, indexLists, -1, materials, bufferFlags);
            built = 1;
        }
    }
    else if (packs != 0 && skinned == 0)
    {
        mesh->BuildBuffers(packs, indexLists, packCount, materials, bufferFlags);
        built = 1;
    }
    if (recurse != 0)
    {
        for (int i = 0; i < childCount; i++)
            children[i]->BuildMesh(1, 0);
    }
    return 1;
}

// Node::Release: frees everything the node owns, then its children (recursively), and resets it with Init: the static
// node list, the second pack and index lists, the extra data, the skin and saved blocks, the visibility cells, the
// materials, the texture reference, the triangle spheres, the mesh and the plug channels and data. A node with more
// than one frame matrix but no texture reference, or with packs but no mesh, stops the game on purpose (fread from
// address 0), as in the original.
void Node::Release()
{
    if (staticNodes != 0)
    {
        g_engineFree(staticNodes);
        staticNodes = 0;
    }
    isStatic = 0;
    staticCount = 0;
    FreePackData(altPacks, altPackCount);
    FreeIndexListData(altIndexLists, altPackCount);
    if (altPacks != 0)
        g_engineFree(altPacks);
    if (altIndexLists != 0)
        g_engineFree(altIndexLists);
    altPacks = 0;
    altIndexLists = 0;
    if (extra != 0)
    {
        extra->Clear();
        g_engineFree(extra);
    }
    extra = 0;
    if (skin != 0)
        g_engineFree(skin);
    skin = 0;
    if (saved != 0)
        g_engineFree(saved);
    saved = 0;
    if (visibilityCells != 0)
        g_engineFree(visibilityCells);
    visibilityCells = 0;
    if (materials != 0)
        g_engineFree(materials);
    materials = 0;
    if (textureRef != 0)
        ReleaseTexture();
    else if (frameMatrixCount > 1)
        fread(0, 1, 1, 0);
    if (triangleSpheres != 0)
    {
        FreeSphereListData(triangleSpheres, packCount);
        g_engineFree(triangleSpheres);
        triangleSpheres = 0;
    }
    if (mesh != 0)
        ReleaseMesh();
    else if (packCount != 0)
        fread(0, 1, 1, 0);
    mesh = 0;
    packs = 0;
    indexLists = 0;
    packCount = 0;
    for (int j = 0; j < NODE_PLUG_COUNT; j++)
    {
        if (plugs[j] != 0)
        {
            g_engineFree(plugs[j]);
            plugs[j] = 0;
            plugType[j] = 0;
        }
    }
    for (int k = 0; k < NODE_PLUG_COUNT; k++)
    {
        if (plugData[k] != 0)
        {
            g_engineFree(plugData[k]);
            plugData[k] = 0;
            plugTypeCopy[k] = 0;
        }
    }
    for (int i = 0; i < childCount; i++)
    {
        children[i]->Release();
        g_engineFree(children[i]);
    }
    Init();
}

// Node::CollectNearSegment: finds the collision nodes (NODE_FLAG_SHAPE_SPHERE, with collision active) in the subtree
// that lie near the segment from-to: within `range` plus NODE_SEGMENT_RADIUS_FACTOR of their bounds radius. Skinned
// nodes are skipped unless g_testSkinnedNodes is set. Each hit is appended to `found` (when given) and counted in
// *foundCount; without a list the search stops at the first hit. *closest receives the closest segment point of the
// last node tested. Returns 1 when a node was found. Called by the map collision code.
int Node::CollectNearSegment(int unused, const Vec4 *from, const Vec4 *to, float range, Node **found, int *foundCount,
                             Vec4 *closest)
{
    if (g_testSkinnedNodes == 0 && skinned != 0)
        return 0;
    if ((flags & NODE_FLAG_SHAPE_SPHERE) == 0)
    {
        for (int i = 0; i < childCount; i++)
        {
            int hit = children[i]->CollectNearSegment(unused, from, to, range, found, foundCount, closest);
            if (hit != 0 && found == 0)
                return hit;
        }
        return 0;
    }
    if (hasCollision == 0)
        return 0;
    if (worldMatrix.GetTranslation().DistanceToSegment(*from, *to, *closest) <
        bounds.w * NODE_SEGMENT_RADIUS_FACTOR + range)
    {
        if (found != 0)
        {
            found[*foundCount] = this;
            (*foundCount)++;
        }
        return 1;
    }
    return 0;
}

// Name prefix of the biped bones whose skin matrices CopyBindPoses hands on.
#define BIPED_BONE_PREFIX "bip"

// Node::CopyBindPoses: for every bone of this node's skin that is a skinned "bip" node, copies the skin's inverse bone
// matrix and bind pose into the bone's own skin, then does the same for the children. Returns 1. Called after the
// skins are set up.
int Node::CopyBindPoses()
{
    if (skin != 0 && skin->boneCount != 0)
    {
        for (int i = 0; i < SKIN_MAX_BONES; i++)
        {
            Node *bone = skin->bones[i];
            if (bone != 0 && bone->skinned != 0 && _strnicmp(bone->name, BIPED_BONE_PREFIX, sizeof(BIPED_BONE_PREFIX) - 1) == 0)
            {
                skin->bones[i]->skin->inverseBone = skin->inverseBones[i];
                skin->bones[i]->skin->bindPoses[0] = skin->bindPoses[i];
            }
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->CopyBindPoses();
    return 1;
}

// Node::MoveToParent: moves this node under `newParent`, inserting it at `position` in the child list (a position past
// the end appends). Within the same parent the node is only reordered. Fails (returns 0) for a missing new parent, a
// node without a parent, a full child list, a new parent that lies inside this node's own subtree, or a reorder to
// the place the node already has. Returns 1 when the node was moved.
int Node::MoveToParent(Node *newParent, int position)
{
    if (newParent == 0)
        return 0;
    if (parent == 0)
        return 0;
    if (newParent->childCount >= NODE_MAX_CHILDREN)
        return 0;
    if (Find(newParent->name, 0) != 0)
        return 0;
    if (parent == newParent)
    {
        int index = 0;
        for (index = 0; index < newParent->childCount; index++)
        {
            if (this == newParent->children[index])
                break;
        }
        if (index == newParent->childCount)
            return 0;
        int target = position;
        if (index == position)
            return 0;
        for (int i = index; i < newParent->childCount - 1; i++)
        {
            newParent->children[i] = newParent->children[i + 1];
            strcpy(newParent->childNames[i], newParent->childNames[i + 1]);
        }
        newParent->childCount--;
        if (position > index)
            target = position - 1;
        for (int i = newParent->childCount - 1; i >= target; i--)
        {
            newParent->children[i + 1] = newParent->children[i];
            strcpy(newParent->childNames[i + 1], newParent->childNames[i]);
        }
        if (newParent->childCount < target)
            target = newParent->childCount;
        newParent->children[target] = this;
        strcpy(newParent->childNames[target], name);
        parent = newParent;
        strcpy(parentName, newParent->name);
        newParent->childCount++;
        return 1;
    }
    parent->RemoveChild(this, 1);
    if (position >= newParent->childCount)
    {
        newParent->children[newParent->childCount] = this;
        strcpy(newParent->childNames[newParent->childCount], name);
        parent = newParent;
        strcpy(parentName, newParent->name);
        newParent->childCount++;
        return 1;
    }
    for (int i = newParent->childCount - 1; i >= position; i--)
    {
        newParent->children[i + 1] = newParent->children[i];
        strcpy(newParent->childNames[i + 1], newParent->childNames[i]);
    }
    newParent->children[position] = this;
    strcpy(newParent->childNames[position], name);
    parent = newParent;
    strcpy(parentName, newParent->name);
    newParent->childCount++;
    return 1;
}

// Node::SortPacks: drops this node's mesh when it was its last user, copies the packs and their index lists into new
// arrays (first those whose material has alpha 1, then the others), frees the old ones and gives the node a new, empty
// mesh. Returns 0 for a node without packs, else 1.
int Node::SortPacks()
{
    int i;
    if (packCount == 0)
        return 0;
    if (mesh != 0 && MeshCache_Release(mesh) == 0)
    {
        mesh->Release();
        g_engineFree(mesh);
        mesh = 0;
    }
    VertexPack *newPacks = (VertexPack *)g_engineAlloc(packCount * sizeof(VertexPack), "vp");
    IndexList *newLists = (IndexList *)g_engineAlloc(packCount * sizeof(IndexList), "vpi");
    for (i = 0; i < packCount; i++)
    {
        newPacks[i].Init();
        newLists[i].Init();
    }
    int count = 0;
    for (i = 0; i < packCount; i++)
    {
        if (materials[packs[i].material].alpha == 1.0f)
        {
            newPacks[count].CopyFrom(&packs[i]);
            newLists[count].CopyFrom(&indexLists[i]);
            count++;
        }
    }
    for (i = 0; i < packCount; i++)
    {
        if (materials[packs[i].material].alpha != 1.0f)
        {
            newPacks[count].CopyFrom(&packs[i]);
            newLists[count].CopyFrom(&indexLists[i]);
            count++;
        }
    }
    FreePackData(packs, packCount);
    FreeIndexListData(indexLists, packCount);
    if (packs != 0)
        g_engineFree(packs);
    if (indexLists != 0)
        g_engineFree(indexLists);
    packs = newPacks;
    indexLists = newLists;
    mesh = (SlotTable *)g_engineAlloc(SLOT_TABLE_SIZE, "CVPCObjB");
    mesh->Reset();
    mesh->bufferSlot = NO_BUFFER_SLOT;
    return 1;
}

// Node::FindBone: searches this subtree for the node `bone`, accumulating world matrices: this node's matrix for `frame`
// (a negative frame -1-n picks override matrix n, at most BONE_FRAME_LIMIT; animated nodes sample their extra data;
// otherwise the override matrix of the frame, clamped to the last one, or the local matrix) times `parent`. The bone's
// matrix is stored in `work` and returned; 0 when the subtree does not hold it. Called by BoneMatrix.
Matrix4 *Node::FindBone(Node *bone, Matrix4 *parent, Matrix4 *work, int frame)
{
    Matrix4 accumulated;
    if (frame < 0)
    {
        int index = -1 - frame;
        accumulated = frameMatrices[index > BONE_FRAME_LIMIT ? BONE_FRAME_LIMIT : index] * *parent;
    }
    else if ((flags & NODE_FLAG_ANIMATED) && extra != 0)
        accumulated = extra->AnimatedMatrix(frame) * *parent;
    else if (frameMatrices != 0)
    {
        accumulated = frameMatrices[frameMatrixCount > frame ? frame : frameMatrixCount - 1] * *parent;
    }
    else
        accumulated = matrix * *parent;
    if (bone == this)
    {
        *work = accumulated;
        return work;
    }
    for (int i = 0; i < childCount; i++)
    {
        Matrix4 *found = children[i]->FindBone(bone, &accumulated, work, frame);
        if (found != 0)
            return found;
    }
    return 0;
}

// Node::BoneMatrix: returns the world matrix of `bone` in this model's tree, starting from the identity; when no child
// has the bone, the identity itself. Called by SetupSkins and the bone attachments.
Matrix4 Node::BoneMatrix(Node *bone, int frame)
{
    Matrix4 parent;
    parent = IdentityMatrix();
    Matrix4 work;
    for (int i = 0; i < childCount; i++)
    {
        Matrix4 *found = children[i]->FindBone(bone, &parent, &work, frame);
        if (found != 0)
            return *found;
    }
    return parent;
}

// Node::SetupSkins: for every skinned node of this subtree: stores its bind pose (its animated matrix when it is
// animated through its extra data, else its local matrix or the override), looks up its bone by name in `root` (not
// itself) and stores the inverse of the bone's world matrix. Returns 1. Called after a model is loaded.
int Node::SetupSkins(Node *root)
{
    if (skinned != 0)
    {
        if ((flags & NODE_FLAG_ANIMATED) && extra != 0)
            skin->bindPoses[0] = extra->AnimatedMatrix(0);
        else if (frameMatrixCount != 0)
            skin->bindPoses[0] = *frameMatrices;
        else
            skin->bindPoses[0] = matrix;
        skin->bone = root->Find(skin->boneName, 0);
        if (skin->bone == this)
            skin->bone = 0;
        if (skin->bone != 0)
            skin->inverseBone = InverseMatrix(skin->bone->BoneMatrix(this, 0));
    }
    for (int i = 0; i < childCount; i++)
        children[i]->SetupSkins(root);
    return 1;
}

// Node::RemoveChild: removes `child` from the child list (destroying it, as Release does, unless `keep`) and closes
// the gap in the child and child name lists. Returns 0. Called by the model code when an attached model goes away.
int Node::RemoveChild(Node *child, int keep)
{
    int i;
    for (i = 0; i < childCount; i++)
    {
        if (child == children[i])
            break;
    }
    if (i == childCount)
        return 0;
    if (keep == 0)
    {
        children[i]->Release();
        g_engineFree(children[i]);
    }
    int j;
    for (j = i; j < childCount - 1; j++)
    {
        children[j] = children[j + 1];
        strcpy(childNames[j], childNames[j + 1]);
    }
    childCount--;
    children[j] = 0;
    return 0;
}
