// Collecting the camera paths of a scene (EN build). Ported from decomp/src/camera_path_select.cpp.
#include "camera.h"
#include "node.h"
#include "node_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Node::pathKind of a camera path node.
const char NODE_KIND_CAMERA_PATH = 2;
// Camera path nodes whose name ends with this character are the second node of another path.
const char CAMERA_PATH_SECOND_SUFFIX = 'b';
// Bytes of a node name buffer.
const int CAMERA_PATH_NAME_SIZE = 0x100;
// Node flags that matter to a camera path: the target's 0x10 and 0x20, the path's own 0x20 and 0x40.
const unsigned int PATH_TARGET_FLAG_CUT = 0x10;
const unsigned int PATH_FLAG_HOLD = 0x20;
const unsigned int PATH_FLAG_INACTIVE = 0x40;

// Camera_CollectPaths: adds every camera path below `node` (kind 2, name not ending in 'b') to the path tables at
// *count: the path id is the last character of its name (atol), the camera track comes from the node, the target track
// from "<name>.Target", and optional tracks from "<name>.Target2" and "<name>b" (all looked up under `root`). Recurses
// into the children of `node`. Returns 0. Called when a camera move starts.
int Camera_CollectPaths(Node *root, Node *node, int *count)
{
    if (node->pathKind == NODE_KIND_CAMERA_PATH &&
        node->name[strlen(node->name) - 1] != CAMERA_PATH_SECOND_SUFFIX)
    {
        char name[CAMERA_PATH_NAME_SIZE];
        sprintf(name, "%sb", node->name);
        Node *second = root->Find(name, 0);
        sprintf(name, "%s.Target", node->name);
        Node *target = root->Find(name, 0);
        sprintf(name, "%s.Target2", node->name);
        Node *target2 = root->Find(name, 0);
        if (target != 0 && node->extra != 0 && target->extra != 0)
        {
            g_pathCameraTracks[*count].CopyFrom(&node->extra->tracks[NODE_TRACK_TRANSLATION]);
            g_pathTargetTracks[*count].CopyFrom(&target->extra->tracks[NODE_TRACK_TRANSLATION]);
            g_pathIds[*count] = atol(&node->name[strlen(node->name) - 1]);
            g_pathActive[*count] = 1;
            g_pathHasSecondTarget[*count] = 0;
            g_pathHasSecondNode[*count] = 0;
            g_pathSettings[*count].m_000c = 0;
            if (target2 != 0)
            {
                g_pathSecondTargetTracks[*count].CopyFrom(&target2->extra->tracks[NODE_TRACK_TRANSLATION]);
                g_pathHasSecondTarget[*count] = 1;
            }
            if (second != 0)
            {
                g_pathSecondTracks[*count].CopyFrom(&second->extra->tracks[NODE_TRACK_TRANSLATION]);
                g_pathHasSecondNode[*count] = 1;
            }
            g_pathTargetTracks[*count].MarkAll();
            g_pathSettings[*count].m_0000 = node->m_187c;
            g_pathSettings[*count].m_0004 = node->m_187d;
            g_pathSettings[*count].m_0008 = 0;
            if ((target->flags & PATH_TARGET_FLAG_CUT) != 0)
                g_pathSettings[*count].m_000c = 1;
            if ((target->flags & PATH_FLAG_HOLD) != 0)
                g_pathSettings[*count].m_0008 = 1;
            if ((node->flags & PATH_FLAG_HOLD) != 0)
                g_pathSettings[*count].m_0008 = 1;
            if ((node->flags & PATH_FLAG_INACTIVE) != 0)
                g_pathActive[*count] = 0;
            (*count)++;
        }
    }
    for (int i = 0; i < node->childCount; i++)
        Camera_CollectPaths(root, node->children[i], count);
    return 0;
}
