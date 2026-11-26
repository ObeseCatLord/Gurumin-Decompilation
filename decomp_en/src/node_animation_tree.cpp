// Copying the animation over a whole node tree (EN build). JP twin: the INFO handling of Node::ScanChunks
// (decomp/src/model_scan_chunks.cpp), which looks up the same node names.
#include <string.h>
#include "node.h"

// Node name prefixes whose animation is copied, and how many characters of the name each one compares.
#define TREE_NAME_BONE "bone"
#define TREE_NAME_BIP "bip"
#define TREE_NAME_CAMERA "camera"
#define TREE_NAME_CHARBASE "charbase"
// "Scene root" in half-width katakana (cp932 bytes), the name of the root node of a model.
#define TREE_NAME_SCENE_ROOT "\xbc\xb0\xdd \xd9\xb0\xc4"
const int TREE_NAME_BONE_LENGTH = 4;
const int TREE_NAME_BIP_LENGTH = 3;
const int TREE_NAME_CAMERA_LENGTH = 6;
const int TREE_NAME_CHARBASE_LENGTH = 8;
const int TREE_NAME_SCENE_ROOT_LENGTH = 7;
// Prefix mark of a node name that is looked up by its beginning only.
const char NODE_PREFIX_SEARCH_MARK = '@';

// Node::CopyTreeAnimation: for this node and, recursively, every child whose name starts with "bone", "bip", "camera",
// "charbase" or the scene root name, finds the node of that name in `source` (Node::Find, which EN inlines) and, when
// there is one, runs CopyAnimation(source, slot) on this node. Returns 1. Called by the model loader.
int Node::CopyTreeAnimation(Node *source, int slot)
{
    if (_strnicmp(name, TREE_NAME_BONE, TREE_NAME_BONE_LENGTH) == 0 || _strnicmp(name, TREE_NAME_BIP, TREE_NAME_BIP_LENGTH) == 0 ||
        _strnicmp(name, TREE_NAME_CAMERA, TREE_NAME_CAMERA_LENGTH) == 0 ||
        _strnicmp(name, TREE_NAME_CHARBASE, TREE_NAME_CHARBASE_LENGTH) == 0 ||
        _strnicmp(name, TREE_NAME_SCENE_ROOT, TREE_NAME_SCENE_ROOT_LENGTH) == 0)
    {
        int skip = 0;
        Node *found;
        if (name[0] == NODE_PREFIX_SEARCH_MARK)
            found = source->FindPrefixRecursive(name + 1, &skip);
        else
            found = source->FindRecursive(name, &skip);
        if (found != 0)
            CopyAnimation(source, slot);
    }
    for (int i = 0; i < childCount; i++)
        children[i]->CopyTreeAnimation(source, slot);
    return 1;
}
