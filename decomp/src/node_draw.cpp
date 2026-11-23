// Drawing a scene node tree (Node, scene_node.cpp for the full layout): each visible node with a mesh draws it, then
// its children draw, then the node's draw callback runs. The thirteen draw arguments after the view matrix are passed
// through unchanged (to MeshData::Draw, the children and the callback).

#include "matrix4.h"

#include "vec4.h"

#include "mesh.h"

#include "node_data.h"

#include "node.h"

#include "map_vars.h"
#include "camera_vars.h"
#include "render_vars.h"

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
    centre.Copy(bounds);
    centre.z = 0;
    if (flags & 0x8000)
        return 0;
    if (DAT_11b04514 != 0 && name[0] == 'C' && name[1] == 'D' && name[2] == 'L' &&
        DAT_11afffe8[(name[3] - '0') * 10 + (name[4] - '0')] == 0)
        return 0;
    if (mesh != 0 && (flags & 0x10) && !(flags & 0x20) && !(flags & 0xe00) && hasCollision != 0)
    {
        if ((flags & 0x40) || FUN_101ae4d0(&(centre + worldMatrix.GetTranslation()), view, cullMargin) != 0)
        {
            mesh->SetRange(0, 0x400);
            if (skin != 0 && skin->boneCount != 0)
                mesh->UploadBones(skin->skinMatrices, skin->boneCount, (Matrix4 *)&worldMatrix, (int)view);
            mesh->Draw(&worldMatrix, view, a4, drawMode, a5, a6, a7, a8, a9, a10, a11, a12, a14);
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->Draw((Matrix4 *)&worldMatrix, view, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
    if (drawCallback != 0)
        drawCallback(&worldMatrix, view, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
    return 1;
}
