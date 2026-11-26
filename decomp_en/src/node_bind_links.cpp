// Linking the nodes and bones of a model before it is skinned (EN build, JP twin M_1019eeb0 in scene_node.cpp).
#include <string.h>

#include "engine_memory.h"
#include "node.h"
#include "node_data.h"
#include "vec4.h"

// Names of the nodes that are bones (compared case-insensitively on the name's first characters).
static const char BONE_PREFIX_BIP[] = "bip";
static const char BONE_PREFIX_BONE[] = "bone";
// Allocation tags of the skin and saved-state blocks.
static const char SKIN_TAG[] = "bone";
static const char SAVED_TAG[] = "ik";
// First character of a name that asks for a prefix search.
const char NAME_PREFIX_MARK = '@';

// The node under `root` that a skin's link or bone name stands for: a name starting with '@' is a prefix search for the
// rest of the name, any other name an exact search.
static inline Node *FindByName(Node *root, const char *name)
{
    int skip = 0;
    if (*name == NAME_PREFIX_MARK)
        return root->FindPrefixRecursive(name + 1, &skip);
    return root->FindRecursive(name, &skip);
}

// Node::BindLinks. Called by SetupModelSkins for the root of a loaded model and (through its own recursion) for every
// child: stores the inverse of this node's matrix; marks "bip"/"bone" nodes as skinned and gives them skin data. A node
// whose skin has bones looks up its linked nodes and its bones under `root`, marks them skinned, and hands them its own
// bone name (the bones only while no earlier bone is a skinned leaf-less node). Nodes with the tracking flag, the chasing
// flag or the collision sphere flag get a saved-state block (a chasing node stores its linked root in it, a collision
// sphere is added to `list`, `*count` entries long). Returns 1.
int Node::BindLinks(Node *root, Node **list, int *count)
{
    inverseMatrix = InverseMatrix(matrix);
    if (_strnicmp(name, BONE_PREFIX_BIP, 3) == 0 || _strnicmp(name, BONE_PREFIX_BONE, 4) == 0)
    {
        skinned = 1;
        if (skin == 0)
        {
            skin = (NodeSkin *)g_engineAlloc(NODE_SKIN_SIZE, SKIN_TAG);
            skin->Clear();
        }
    }
    if (skin != 0 && skin->boneCount != 0)
    {
        for (int i = 0; i < SKIN_MAX_LINKS; i++)
        {
            if (skin->linkNames[i][0] != 0)
            {
                skin->links[i] = FindByName(root, skin->linkNames[i]);
                skin->links[i]->skinned = 1;
                if (skin->links[i]->skin == 0)
                {
                    skin->links[i]->skin = (NodeSkin *)g_engineAlloc(NODE_SKIN_SIZE, SKIN_TAG);
                    skin->links[i]->skin->Clear();
                }
                skin->links[i]->skin->isLink = 1;
                strcpy(skin->links[i]->skin->boneName, skin->boneName);
            }
        }
        int chain = 1;
        for (int i = 0; i < SKIN_MAX_BONES; i++)
        {
            if (skin->boneNames[i][0] != 0)
            {
                skin->bones[i] = FindByName(root, skin->boneNames[i]);
                if (skin->bones[i] != 0)
                {
                    if (_strnicmp(skin->bones[i]->name, BONE_PREFIX_BIP, 3) == 0 ||
                        _strnicmp(skin->bones[i]->name, BONE_PREFIX_BONE, 4) == 0)
                        skin->bones[i]->skinned = 1;
                    if (skin->bones[i]->skinned == 0 && skin->bones[i]->childCount != 0)
                        chain = 0;
                    if (i > 0 && chain && skin->bones[i] != 0)
                    {
                        if (skin->bones[i]->skin == 0)
                        {
                            skin->bones[i]->skin = (NodeSkin *)g_engineAlloc(NODE_SKIN_SIZE, SKIN_TAG);
                            skin->bones[i]->skin->Clear();
                        }
                        strcpy(skin->bones[i]->skin->boneName, skin->boneName);
                    }
                }
            }
        }
    }
    if ((flags & NODE_FLAG_TARGET_TRACKING) == 0)
    {
        if ((flags & NODE_FLAG_CHASING) == 0)
        {
            if ((flags & NODE_FLAG_COLLISION_SPHERE) != 0)
            {
                if (saved == 0)
                {
                    saved = (NodeSaved *)g_engineAlloc(NODE_SAVED_SIZE, SAVED_TAG);
                    saved->Reset();
                }
                list[*count] = this;
                (*count)++;
            }
        }
        else
        {
            if (saved == 0)
            {
                saved = (NodeSaved *)g_engineAlloc(NODE_SAVED_SIZE, SAVED_TAG);
                saved->Reset();
            }
            if (parent == 0)
                saved->linkedRoot = this;
            else if ((parent->flags & NODE_FLAG_CHASING) == 0)
                saved->linkedRoot = this;
            else
                saved->linkedRoot = GetLinkedRoot();
        }
    }
    else if (saved == 0)
    {
        saved = (NodeSaved *)g_engineAlloc(NODE_SAVED_SIZE, SAVED_TAG);
        saved->Reset();
    }
    for (int child = 0; child < childCount; child++)
        children[child]->BindLinks(root, list, count);
    return 1;
}
