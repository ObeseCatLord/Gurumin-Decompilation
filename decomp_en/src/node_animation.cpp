// Copying the animation and the whole tree of a scene node (EN build). Ported from the matched JP source
// decomp/src/scene_node.cpp (Node::CopyAnimation, Node::Clone).
#include <string.h>
#include "node.h"
#include "node_data.h"
#include "motion_list.h"
#include "mesh_cache.h"
#include "engine_memory.h"

// Highest stored pose a negative frame can select.
const int NODE_LAST_POSE = NODE_POSE_COUNT - 1;
// Node flag: the node's matrix comes from its animation tracks.
const unsigned int NODE_FLAG_TRACKED_MATRIX = 0x100000;
// Value of motionSlot while no cache entry was found.
const int MOTION_SLOT_NONE = -1;

// Node::CopyAnimation: gives this node the animation of `source`: the plug channels (copied whole), the extra key
// tracks (the first three all from the source's second track, as in the original), the node matrix and the frame
// matrices. With `slot` 0 the cached motion of this node's name is shared when there is one; otherwise a copy is made
// and cached (in a new slot for `slot` non-zero). The pose the node had (its current frame or stored pose) is kept in
// poses[NODE_LAST_POSE]. Returns 0. Called by Node::Clone.
int Node::CopyAnimation(Node *source, int slot)
{
    Matrix4 current;
    if (source != 0)
    {
        if (frame < 0)
        {
            int pose = -1 - frame;
            pose = pose < NODE_LAST_POSE ? pose : NODE_LAST_POSE;
            current = poses[pose];
        }
        else if ((flags & NODE_FLAG_TRACKED_MATRIX) != 0 && extra != 0)
            current = extra->AnimatedMatrix(frame);
        else if (frameMatrices != 0)
        {
            int index = frame;
            if (frameMatrixCount <= index)
                index = frameMatrixCount - 1;
            current = frameMatrices[index];
        }
        else
            current = matrix;
    }
    int g;
    for (g = 0; g < NODE_PLUG_COUNT; g++)
    {
        if (plugs[g] != 0)
        {
            g_engineFree(plugs[g]);
            plugs[g] = 0;
            plugType[g] = 0;
        }
    }
    if (textureRef != 0)
        ReleaseTexture();
    if (source == 0)
        return 0;
    strcpy(folderB, source->folderB);
    for (g = 0; g < NODE_PLUG_COUNT; g++)
    {
        if (source->plugs[g] != 0)
        {
            int size = source->plugs[g]->size;
            plugs[g] = (Channel *)g_engineAlloc(size, "->m_plugdata[g]");
            memcpy(plugs[g], source->plugs[g], size);
            plugType[g] = source->plugType[g];
        }
    }
    if (source->extra != 0)
    {
        if (extra == 0)
        {
            extra = (NodeExtra *)g_engineAlloc(NODE_EXTRA_SIZE, "keymat");
            extra->Init();
        }
        extra->tracks[0].CopyFrom(&source->extra->tracks[1]);
        extra->tracks[1].CopyFrom(&source->extra->tracks[1]);
        extra->tracks[2].CopyFrom(&source->extra->tracks[1]);
        extra->tracks[3].CopyFrom(&source->extra->tracks[3]);
        extra->tracks[4].CopyFrom(&source->extra->tracks[4]);
        extra->m_0070[0] = source->extra->m_0070[0];
        extra->m_0070[1] = source->extra->m_0070[1];
        extra->m_0070[2] = source->extra->m_0070[2];
        extra->m_0070[3] = source->extra->m_0070[3];
    }
    matrix = source->matrix;
    if (source->textureRef != 0)
    {
        int motionSlot = MOTION_SLOT_NONE;
        if (slot == 0)
        {
            int found = MotionCache_Find(folderB, name, &motionSlot);
            if (found >= 0)
            {
                textureRef = g_motionCacheRef[found];
                frameMatrices = g_motionCacheMatrices[found];
                frameMatrixCount = g_motionCacheCount[found];
                TextureFile_AddRef(textureRef);
                goto keepPose;
            }
        }
        else
        {
            motionSlot = TextureNames_FindFree();
            MotionCache_SetNames(motionSlot, folderB, name);
        }
        textureRef = (TextureRef *)g_engineAlloc(sizeof(TextureRef), "motion");
        frameMatrixCount = source->frameMatrixCount;
        frameMatrices = (Matrix4 *)g_engineAlloc(source->frameMatrixCount << 6, "tmp[i]->m_animat");
        memcpy(frameMatrices, source->frameMatrices, source->frameMatrixCount << 6);
        textureRef->slot = motionSlot;
        if (motionSlot >= 0)
        {
            g_motionCacheRef[motionSlot] = textureRef;
            g_motionCacheMatrices[motionSlot] = frameMatrices;
            g_motionCacheCount[motionSlot] = frameMatrixCount;
            TextureFile_AddRef(textureRef);
        }
    }
keepPose:
    poses[NODE_LAST_POSE] = current;
    return 0;
}

// Node::Clone: makes this node a copy of `source` and its whole subtree: names, matrices and stored poses, flags and
// settings, the saved block, the plug channels, the animation (CopyAnimation, cache flag `motionSlot`) and the mesh
// (CopyMesh, cache flag `meshSlot`), the skin data, and a new copy of every child. Returns 1.
int Node::Clone(Node *source, int motionSlot, int meshSlot)
{
    Release();
    strcpy(name, source->name);
    strcpy(folder, source->folder);
    strcpy(folderB, source->folderB);
    matrix = source->matrix;
    worldMatrix = source->worldMatrix;
    int i;
    for (i = 0; i < NODE_POSE_COUNT; i++)
        poses[i] = source->poses[i];
    flags = source->flags;
    cullMargin = source->cullMargin;
    group = source->group;
    fogColour.Copy(source->fogColour);
    m_1878 = source->m_1878;
    fogDensity = source->fogDensity;
    lightConeInner = source->lightConeInner;
    m_1888 = source->m_1888;
    fogNear = source->fogNear;
    fogFar = source->fogFar;
    pathKind = source->pathKind;
    m_187a = source->m_187a;
    m_187b = source->m_187b;
    m_187c = source->m_187c;
    m_187d = source->m_187d;
    m_187e = source->m_187e;
    drawMode = source->drawMode;
    animated = source->animated;
    frameCount = source->frameCount;
    frameSize = source->frameSize;
    if (source->saved != 0)
    {
        saved = (NodeSaved *)g_engineAlloc(NODE_SAVED_SIZE, "ik");
        saved->Reset();
        *saved = *source->saved;
    }
    for (int g = 0; g < NODE_PLUG_COUNT; g++)
    {
        if (source->plugs[g] != 0)
        {
            int size = source->plugs[g]->size;
            plugs[g] = (Channel *)g_engineAlloc(size, "->m_plugdata[g]");
            memcpy(plugs[g], source->plugs[g], size);
            plugType[g] = source->plugType[g];
        }
    }
    CopyAnimation(source, motionSlot);
    CopyMesh(source, meshSlot);
    ComputeBounds();
    if (source->skin != 0 && source->skin->boneCount != 0)
    {
        skin = (NodeSkin *)g_engineAlloc(NODE_SKIN_SIZE, "bone");
        skin->Clear();
        strcpy(skin->boneName, source->skin->boneName);
        skin->boneCount = source->skin->boneCount;
        for (i = 0; i < source->skin->boneCount; i++)
            strcpy(skin->linkNames[i], source->skin->linkNames[i]);
        for (i = 0; i < SKIN_MAX_BONES; i++)
        {
            strcpy(skin->boneNames[i], source->skin->boneNames[i]);
            skin->inverseBones[i] = source->skin->inverseBones[i];
            skin->bindPoses[i] = source->skin->bindPoses[i];
        }
    }
    strcpy(parentName, source->parentName);
    childCount = source->childCount;
    for (i = 0; i < source->childCount; i++)
    {
        strcpy(childNames[i], source->childNames[i]);
        children[i] = (Node *)g_engineAlloc(NODE_SIZE, "dup");
        children[i]->Init();
        children[i]->Clone(source->children[i], motionSlot, meshSlot);
        children[i]->parent = this;
    }
    hasCollision = 0;
    return 1;
}
