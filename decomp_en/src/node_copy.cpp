// Scene node copying and building (EN build): child copies, clones, mesh and animation copies. Ported from the matched
// JP source decomp/src/scene_node.cpp. Kept apart from scene_node.cpp so the calls to Node::ReleaseMesh stay calls.
#include "node.h"
#include "node_data.h"
#include "containers.h"
#include "engine_memory.h"
#include "mesh_cache.h"
#include "slot_table.h"
#include "index_list.h"
#include "texture_mgr.h"
#include <stdio.h>
#include <string.h>

// Node::CreateChildCopies: makes NODE_CHILD_COPY_COUNT copies of this node as its children (for effects drawn many
// times, such as particles): each is cloned from this node and shaded as a wave copy with its own phase, and the
// node's own mesh is released afterwards. The node's box z range collapses to the first vertex's z. Returns 1.
int Node::CreateChildCopies()
{
    Node *created[NODE_CHILD_COPY_COUNT];
    box[NODE_BOX_MIN].z = packs->vertices[0].position[2];
    box[NODE_BOX_MAX].z = packs->vertices[0].position[2];
    cycleIndex = 1;
    for (int i = 0; i < NODE_CHILD_COPY_COUNT; i++)
    {
        Node *node = (Node *)g_engineAlloc(NODE_SIZE, NODE_CHILD_ALLOC_TAG);
        created[i] = node;
        node->Init();
        node->Clone(this, 0, 1);
    }
    childCount = NODE_CHILD_COPY_COUNT;
    for (int i = 0; i < NODE_CHILD_COPY_COUNT; i++)
    {
        children[i] = created[i];
        created[i]->parent = this;
        children[i]->ShadeWaveCopy(i);
        children[i]->drawMode = NODE_DRAW_MODE_CHILD_COPY;
    }
    ReleaseMesh();
    return 1;
}

// Name given to a child created without one.
#define NODE_DEFAULT_CHILD_NAME "none"

// Node::AddChild: creates a child node called `childName` (NODE_DEFAULT_CHILD_NAME for 0) at `position` in the child
// list (appended when the position is negative or past the end). The child inherits the folder names, starts in
// render mode NODE_DRAW_MODE_NEW_CHILD with the never-culled flag and gets a unique name. Returns the child, or 0 when
// this node already has the most children it can hold.
Node *Node::AddChild(const char *childName, int position)
{
    if (childCount > NODE_MAX_CHILDREN - 1)
        return 0;
    Node *child = (Node *)g_engineAlloc(NODE_SIZE, NODE_CHILD_ALLOC_TAG);
    child->Init();
    child->parent = this;
    strcpy(child->folder, folder);
    strcpy(child->folderB, folderB);
    child->drawMode = NODE_DRAW_MODE_NEW_CHILD;
    child->flags = NODE_FLAG_NEVER_CULLED;
    if (childName == 0)
        strcpy(child->name, NODE_DEFAULT_CHILD_NAME);
    else
        strcpy(child->name, childName);
    if (position >= childCount || position < 0)
    {
        children[childCount] = child;
        childCount++;
        children[childCount - 1]->MakeNameUnique();
        return children[childCount - 1];
    }
    for (int i = childCount - 1; i >= position; i--)
        children[i + 1] = children[i];
    children[position] = child;
    childCount++;
    children[position]->MakeNameUnique();
    return children[position];
}

// Allocator tags of the copies CopyMesh makes (they show up in the engine's leak report).
#define PACK_ALLOC_TAG "m_vpack"
#define INDEX_LIST_ALLOC_TAG "m_vpackindex"
#define MESH_ALLOC_TAG "CpMCObj"
#define MATERIAL_ALLOC_TAG "m_material"

// Node::CopyMesh: gives this node the mesh of `source`. With `slot` 0 the cached mesh of this node's folder and name is
// shared when there is one (otherwise the free cache slot found is used); with `slot` non-zero a new name slot is
// taken. Without a shared mesh the packs and index lists are copied, a new mesh is made and cached in the slot. Then
// the materials are copied (loading their textures), and the draw mode and bounds. Returns 0 without a source, else 1.
// Called while a model is copied.
int Node::CopyMesh(Node *source, int slot)
{
    int cacheSlot;
    if (source == 0)
        return 0;
    hasCollision = 0;
    if (source->packCount != 0)
    {
        bool shared = false;
        if (slot == 0)
        {
            int found = ModelNames_Find(folder, name, &cacheSlot);
            if (found >= 0)
            {
                mesh = g_modelFileMeshes[found];
                packs = g_modelFilePacks[found];
                indexLists = g_modelFileIndexLists[found];
                packCount = g_modelFilePackCount[found];
                MeshCache_AddRef(mesh);
                shared = true;
            }
        }
        else
        {
            cacheSlot = ModelNames_FindFree();
            ModelNames_SetJoined(cacheSlot, folder, name);
        }
        if (!shared)
        {
            if (packs != 0)
                FreePackData(packs, packCount);
            if (indexLists != 0)
                FreeIndexListData(indexLists, packCount);
            if (packs != 0)
                g_engineFree(packs);
            if (indexLists != 0)
                g_engineFree(indexLists);
            if (materials != 0)
                g_engineFree(materials);
            packs = (VertexPack *)g_engineAlloc(source->packCount * sizeof(VertexPack), PACK_ALLOC_TAG);
            indexLists = (IndexList *)g_engineAlloc(source->packCount * sizeof(IndexList), INDEX_LIST_ALLOC_TAG);
            packCount = source->packCount;
            for (int i = 0; i < packCount; i++)
            {
                packs[i].Init();
                packs[i].CopyFrom(&source->packs[i]);
                indexLists[i].Init();
                indexLists[i].CopyFrom(&source->indexLists[i]);
            }
            if (mesh != 0)
                fread(0, 1, 1, 0);
            mesh = (SlotTable *)g_engineAlloc(sizeof(SlotTable), MESH_ALLOC_TAG);
            mesh->Reset();
            mesh->bufferSlot = cacheSlot;
            if (cacheSlot >= 0)
            {
                g_modelFileMeshes[cacheSlot] = mesh;
                g_modelFilePacks[cacheSlot] = packs;
                g_modelFileIndexLists[cacheSlot] = indexLists;
                g_modelFilePackCount[cacheSlot] = packCount;
                MeshCache_AddRef(mesh);
            }
        }
    }
    if (source->materialCount != 0)
    {
        materials = (Material *)g_engineAlloc(source->materialCount * sizeof(Material), MATERIAL_ALLOC_TAG);
        g_engineCopy(materials, source->materials, source->materialCount * sizeof(Material));
        materialCount = source->materialCount;
        for (int i = 0; i < materialCount; i++)
        {
            if (materials[i].textures[0][0] != 0)
                Texture_LoadIti(materials[i].textures[0], 0, 0);
        }
    }
    drawMode = source->drawMode;
    bounds.Copy(source->bounds);
    box[NODE_BOX_MIN].Copy(source->box[NODE_BOX_MIN]);
    box[NODE_BOX_MAX].Copy(source->box[NODE_BOX_MAX]);
    return 1;
}
