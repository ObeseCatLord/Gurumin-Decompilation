// Switching the camera to one of the map's camera paths (map camera data, see camera_reset.cpp): the path is looked
// up by its id and the camera starts at the path frame closest to the camera target, and collecting
// the paths from the scene.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vec4.h"

#include "matrix4.h"

// A key track (vec4.cpp KeyTrack), 0x14 bytes: its frames are sorted, so the last one is the length.
struct KeyTrack
{
    int capacity;
    int *frames;
    Vec4 *values;
    char *flags;
    int count;
    // 0x101afe00: the value at `frame` (with `rotation` set, a quaternion interpolated as a rotation).
    int Sample(int frame, float *x, float *y, float *z, float *w, int rotation);
    void Copy(const KeyTrack *source);           // 0x101ade10
    int MarkAll();                               // 0x101adb80
};

// The model the camera follows (AnimModel), only its world matrix.
struct CameraTarget
{
    char pad_0000[0x2328];
    Matrix4 world;                                   // 0x2328
};

extern "C" int DAT_1101d89c;                         // number of camera paths
extern "C" int DAT_10f8d938[];                       // id of each path
extern "C" KeyTrack DAT_118e69f8[];                  // the position track of each path
extern "C" int DAT_1101d8a4;                         // the current path
extern "C" int DAT_1101d890;                         // 1 while a path camera runs
extern "C" int DAT_1101d8a0;
extern "C" float DAT_1101c800;                       // frame of the path
extern "C" char DAT_10faa678;
extern "C" CameraTarget *DAT_1101d884;               // the model the camera follows

extern "C" int FUN_1015c9e0(int mode);               // Screen_StartTransition

// 0x1016b050 (Camera_SelectPath). Id -1 stops the path camera. Otherwise the path with that id becomes current (if
// the camera already runs on it nothing changes; switching paths starts transition 0xf), and the camera starts at
// the frame of the path closest to the camera target (distance with z counted twice). Returns 0.
extern "C" int FUN_1016b050(int id)
{
    if (id == -1)
    {
        DAT_1101d890 = 0;
        return 0;
    }
    int previous = DAT_1101d8a4;
    for (int i = 0; i < DAT_1101d89c; i++)
    {
        if (DAT_10f8d938[i] == id)
        {
            DAT_1101d8a4 = i;
            break;
        }
    }
    if (DAT_1101d890 != 0)
    {
        if (DAT_1101d8a4 == previous)
            return 0;
        FUN_1015c9e0(0xf);
    }
    int path = DAT_1101d8a4;
    float nearest = 0.0f;
    Vec4 point;
    Vec4 target;
    target.Copy(DAT_1101d884->world.GetTranslation());
    float length = (float)DAT_118e69f8[path].frames[DAT_118e69f8[path].count - 1];
    float bestFrame;
    int bestPath;
    int frame = 0;
    for (float f = 0.0f; f < length; f = (float)++frame)
    {
        DAT_118e69f8[path].Sample(frame, &point.x, &point.y, &point.z, &point.w, 0);
        float distance = point.DistanceZWeighted(target);
        if (nearest >= distance || frame == 0)
        {
            nearest = distance;
            bestFrame = f;
            bestPath = path;
        }
    }
    DAT_1101d8a0 = 1;
    DAT_1101d8a4 = bestPath;
    DAT_1101c800 = bestFrame;
    DAT_1101d890 = 1;
    DAT_10faa678 = 1;
    return 0;
}

// The extra data of a scene node (scene_node.cpp Extra): its first key track.
struct NodeExtra
{
    KeyTrack track;
};

// A scene node (scene_node.cpp Node), only the fields used here.
struct PathNode
{
    char name[0x40];
    char pad_0040[0x6c];
    unsigned int flags;          // 0xac: 0x10 = ..., 0x20 = ..., 0x40 = path not active at the start
    char pad_00b0[0x1f8];
    NodeExtra *extra;            // 0x2a8: the node's animation tracks
    char pad_02ac[0x4a4];
    int childCount;              // 0x750
    char pad_0754[0x1000];
    PathNode *children[0x40];    // 0x1754
    char pad_1854[0x25];
    char kind;                   // 0x1879: 2 = camera path
    char pad_187a[2];
    unsigned char m_187c;
    unsigned char m_187d;
    PathNode *Find(const char *name, int index);    // 0x1019ece0
};

// Per camera path: two values from the path node, a flag and the target's flag 0x10.
struct PathSettings
{
    int m_0000;
    int m_0004;
    int m_0008;
    int m_000c;
};

extern "C" KeyTrack DAT_11af3928[];                  // per path: the camera position track
extern "C" KeyTrack DAT_11af3ba8[];                  //   the second target track
extern "C" KeyTrack DAT_118e96f8[];                  //   the "...b" node's track
extern "C" int DAT_11018740[];                       //   1 = active
extern "C" int DAT_11018628[];                       //   1 = has a second target
extern "C" int DAT_10fdeb10[];                       //   1 = has a "...b" node
extern "C" PathSettings DAT_10ff17e0[];

// 0x1016ac90 (Camera_CollectPaths). Adds every camera path below `node` (kind 2, name not ending in 'b') to the
// path tables at *count: the path id is the last character of its name (atol), the camera track comes from the
// node, the target track from "<name>.Target", and optional tracks from "<name>.Target2" and "<name>b" (all looked up
// under `root`). Returns 0.
extern "C" int FUN_1016ac90(PathNode *root, PathNode *node, int *count)
{
    if (node->kind == 2 && node->name[strlen(node->name) - 1] != 'b')
    {
        char name[0x100];
        sprintf(name, "%sb", node->name);
        PathNode *second = root->Find(name, 0);
        sprintf(name, "%s.Target", node->name);
        PathNode *target = root->Find(name, 0);
        sprintf(name, "%s.Target2", node->name);
        PathNode *target2 = root->Find(name, 0);
        if (target != 0 && node->extra != 0 && target->extra != 0)
        {
            DAT_11af3928[*count].Copy(&node->extra->track);
            DAT_118e69f8[*count].Copy(&target->extra->track);
            DAT_10f8d938[*count] = atol(&node->name[strlen(node->name) - 1]);
            DAT_11018740[*count] = 1;
            DAT_11018628[*count] = 0;
            DAT_10fdeb10[*count] = 0;
            DAT_10ff17e0[*count].m_000c = 0;
            if (target2 != 0)
            {
                DAT_11af3ba8[*count].Copy(&target2->extra->track);
                DAT_11018628[*count] = 1;
            }
            if (second != 0)
            {
                DAT_118e96f8[*count].Copy(&second->extra->track);
                DAT_10fdeb10[*count] = 1;
            }
            DAT_118e69f8[*count].MarkAll();
            DAT_10ff17e0[*count].m_0000 = node->m_187c;
            DAT_10ff17e0[*count].m_0004 = node->m_187d;
            DAT_10ff17e0[*count].m_0008 = 0;
            if ((target->flags & 0x10) != 0)
                DAT_10ff17e0[*count].m_000c = 1;
            if ((target->flags & 0x20) != 0)
                DAT_10ff17e0[*count].m_0008 = 1;
            if ((node->flags & 0x20) != 0)
                DAT_10ff17e0[*count].m_0008 = 1;
            if ((node->flags & 0x40) != 0)
                DAT_11018740[*count] = 0;
            (*count)++;
        }
    }
    for (int i = 0; i < node->childCount; i++)
        FUN_1016ac90(root, node->children[i], count);
    return 0;
}
