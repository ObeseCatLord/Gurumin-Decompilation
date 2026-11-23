// CameraTarget, PathSettings: declared here once (first used by src/camera_path_select.cpp).
#ifndef GURUMIN_CAMERA_PATH_SELECT_H
#define GURUMIN_CAMERA_PATH_SELECT_H

#include "matrix4.h"

struct Node;

// The model the camera follows (AnimModel), only its world matrix.
struct CameraTarget
{
    char pad_0000[0x231c];
    Node *collision;                                 // 0x231c: the collision tree the camera casts rays against
    char pad_2320[8];
    Matrix4 world;                                   // 0x2328
};
// Per camera path: two values from the path node, a flag and the target's flag 0x10.
struct PathSettings
{
    int m_0000;
    int m_0004;
    int m_0008;
    int m_000c;
};

#endif
