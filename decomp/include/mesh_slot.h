// MeshSlot: an attached mesh slot (64 at DAT_10edbe08, 0xb08 bytes each): extra models such as held items.
#ifndef GURUMIN_MESH_SLOT_H
#define GURUMIN_MESH_SLOT_H

#include "vec4.h"
#include "matrix4.h"

struct Node;

struct MeshSlot
{
    int pad_0000;
    Node *mesh;               // 0x04: the attached model (0 = slot free)
    int delay;                // 0x08: frames before it is drawn
    int pad_000c;
    Vec4 colour;              // 0x10
    char pad_0020[0x10];
    int visible;              // 0x30
    int frame;                // 0x34: frames since it was attached
    int pad_0038;
    int ownCamera;            // 0x3c: non-zero = drawn with its own fixed camera
    Matrix4 matrix;            // 0x40: placement of the model
    char pad_0080[0xa88];
    int Draw(const Matrix4 &view);                // 0x1000baf0
    void M_10037b40(const void *a, const void *b);   // called by ai_pick.cpp
};

#endif
