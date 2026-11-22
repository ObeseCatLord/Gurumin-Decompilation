// Drawing a scene node tree (Node, scene_node.cpp for the full layout): each visible node with a mesh draws it, then
// its children draw, then the node's draw callback runs. The thirteen draw arguments after the view matrix are passed
// through unchanged (to MeshData::Draw, the children and the callback).

#include "matrix4.h"

#include "vec4.h"

struct MeshData
{
    int SetRange(int first, int last);           // 0x10146d90 (SlotTable::M_10146d90)
    void UploadBones(Matrix4 *bones, int count, Matrix4 *world, int view);   // 0x10145a50
    // 0x10151150: draws the mesh with the world matrix and the pass-through arguments.
    int Draw(Matrix4 *world, const Matrix4 *view, int a4, int a0a0, int a5, int a6, int a7, int a8, int a9,
             int a10, int a11, int a12, int a14);
};

// Skinning data of a node: the bone matrices and their count.
struct NodeSkin
{
    char pad_0000[0x4618];
    Matrix4 bones[0x40];      // 0x4618
    int boneCount;            // 0x5618
};

typedef void (*NodeDrawCallback)(Matrix4 *world, const Matrix4 *view, int a3, int a4, int a5, int a6, int a7,
                                 int a8, int a9, int a10, int a11, int a12, int a13, int a14);

struct Node
{
    char name[0x40];          // 0x000: "CDLnn..." names are collision regions
    char pad_0040[0x38];
    NodeDrawCallback drawCallback;   // 0x078
    char pad_007c[0x20];
    MeshData *mesh;           // 0x09c
    int m_00a0;
    char pad_00a4[4];
    int m_00a8;
    unsigned int flags;       // 0x0ac: 0x10 = drawn, 0x20 = hidden, 0x40 = never culled, 0xe00 = special passes,
                              // 0x8000 = skip the subtree
    float cullMargin;         // 0x0b0: margin of the view-volume test
    char pad_00b4[0x198];
    Matrix4 world;         // 0x24c: world matrix
    char pad_028c[0x448];
    Vec4 boundsCentre;        // 0x6d4
    char pad_06e4[0x24];
    NodeSkin *skin;           // 0x708
    char pad_070c[0x44];
    int childCount;           // 0x750
    char childNames[0x40][0x40];
    Node *children[0x40];     // 0x1754
    int Draw(const Matrix4 *parent, const Matrix4 *view, int a3, int a4, int a5, int a6, int a7, int a8, int a9,
             int a10, int a11, int a12, int a13, int a14);
};

extern "C" char DAT_11b04514;            // non-zero = only the enabled collision regions are drawn
extern "C" char DAT_11affdd8[100];       // per collision region "CDLnn": 1 = enabled
extern "C" int FUN_101ae4d0(const Vec4 *point, const Matrix4 *matrix, float margin);   // View_ContainsPoint

// 0x101995a0 (Node::Draw). Draws this node unless its subtree is skipped (flag 0x8000) or it is a collision region
// "CDLnn" that is switched off: a drawn (0x10), shown, normal node with a mesh is culled against the view volume (its
// bounds centre with its margin, unless flag 0x40), then its mesh range is reset, its bones uploaded when it is
// skinned and the mesh drawn. Then every child draws with this node's world matrix as the parent, and the draw
// callback runs. Returns 1, or 0 when the subtree is skipped.
int Node::Draw(const Matrix4 *parent, const Matrix4 *view, int a3, int a4, int a5, int a6, int a7, int a8, int a9,
               int a10, int a11, int a12, int a13, int a14)
{
    Vec4 centre;
    centre.Copy(boundsCentre);
    centre.z = 0;
    if (flags & 0x8000)
        return 0;
    if (DAT_11b04514 != 0 && name[0] == 'C' && name[1] == 'D' && name[2] == 'L' &&
        DAT_11affdd8[name[3] * 10 + name[4]] == 0)
        return 0;
    if (mesh != 0 && (flags & 0x10) && !(flags & 0x20) && !(flags & 0xe00) && m_00a8 != 0)
    {
        if ((flags & 0x40) || FUN_101ae4d0(&(centre + world.GetTranslation()), view, cullMargin) != 0)
        {
            mesh->SetRange(0, 0x400);
            if (skin != 0 && skin->boneCount != 0)
                mesh->UploadBones(skin->bones, skin->boneCount, (Matrix4 *)&world, (int)view);
            mesh->Draw(&world, view, a4, m_00a0, a5, a6, a7, a8, a9, a10, a11, a12, a14);
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->Draw((Matrix4 *)&world, view, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
    if (drawCallback != 0)
        drawCallback(&world, view, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
    return 1;
}
