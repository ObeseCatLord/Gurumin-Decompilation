// Effect models and node skin data (EN build). Ported from the matched JP source decomp/src/scene_misc.cpp.
#include <string.h>
#include "effect_nodes.h"
#include "engine.h"
#include "node_data.h"

// Effect_LoadModel: loads the model of effect `index` when its kind is 4, 10 or 20 (from the effect table), builds its
// mesh and marks the effect node as used. Returns 0. Called when a map's effects are set up.
extern "C" int __stdcall Effect_LoadModel(int index)
{
    int kind = g_effectNames[index].kind;
    if (kind == EFFECT_KIND_MODEL_A || kind == EFFECT_KIND_MODEL_B || kind == EFFECT_KIND_MODEL_C)
    {
        g_effectNodes[index].LoadIt3(Path_Data(g_effectNames[index].name, 0), 0, 0);
        g_effectNodes[index].BuildMesh(1, 0);
        g_effectNodeLoaded[index] = 1;
    }
    return 0;
}

// NodeSkin::Clear: empties a node's skin data: no bone, no bone name, not a link, no links or bones, all names
// cleared. Called when a node is reset or reloaded.
void NodeSkin::Clear()
{
    boneCount = 0;
    boneName[0] = 0;
    isLink = 0;
    bone = 0;
    memset(linkNames, 0, sizeof(linkNames));
    memset(boneNames, 0, sizeof(boneNames));
    memset(links, 0, sizeof(links));
    memset(bones, 0, sizeof(bones));
}
