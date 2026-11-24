// Direct drawing of a node tree (EN build). Ported from the matched JP source decomp/src/node_draw.cpp.
#include "map_view.h"
#include "node.h"
#include "node_data.h"
#include "slot_table.h"
#include "visibility.h"

const int MESH_RANGE_END_SINGLE_FRAME = 0x400;   // SetRange end for a mesh with a single frame
const int REGION_NAME_LENGTH = 3;                // "CDL" prefix of a collision region node's name
const int REGION_DIGIT_BASE = 10;                // the two digits after "CDL" are the region number

// Draws this node unless its subtree is skipped (flag 0x8000) or it is a collision region "CDLnn" that is switched off:
// a drawn (0x10), shown, normal node with a mesh is culled against the view volume (its bounds centre with its margin,
// unless flag 0x40), then its mesh range is reset, its bones uploaded when it is skinned and the mesh drawn. Then every
// child draws with this node's world matrix in the first argument (the parent matrix, which the header calls `list`)
// and the draw callback runs. Returns 1, or 0 when the subtree is skipped. Called by the shadow and scene passes.
int Node::Draw(DrawList *list, Matrix4 *view, int a3, const Matrix4 *shadowMatrix, float shadowX, float shadowY,
               float shadowZ, int shadowTexture, float shadowScale, int shadowSize, const Vec4 *lightDirection,
               float modelSize, AnimModel *model, float alpha)
{
    Vec4 centre;
    centre.Copy(bounds);
    centre.z = 0;
    if (flags & NODE_FLAG_SKIP_SUBTREE)
        return 0;
    if (g_regionsOnlyEnabled != 0 && name[0] == 'C' && name[1] == 'D' && name[2] == 'L' &&
        g_regionSwitches[(name[3] - '0') * REGION_DIGIT_BASE + (name[4] - '0')] == 0)
        return 0;
    if (mesh != 0 && (flags & 0x10) && !(flags & NODE_FLAG_HIDDEN) && !(flags & 0xe00) && hasCollision != 0)
    {
        if ((flags & NODE_FLAG_NEVER_CULLED) ||
            View_ContainsPoint(&(centre + worldMatrix.GetTranslation()), view, cullMargin) != 0)
        {
            mesh->SetRange(0, MESH_RANGE_END_SINGLE_FRAME);
            if (skin != 0 && skin->boneCount != 0)
                mesh->UploadBones(skin->skinMatrices, skin->boneCount, &worldMatrix, (int)view);
            mesh->Draw(&worldMatrix, view, shadowMatrix, drawMode, shadowX, shadowY, shadowZ, shadowTexture, shadowScale,
                       shadowSize, lightDirection, modelSize, alpha);
        }
    }
    for (int i = 0; i < childCount; i++)
        children[i]->Draw((DrawList *)&worldMatrix, view, a3, shadowMatrix, shadowX, shadowY, shadowZ, shadowTexture,
                          shadowScale, shadowSize, lightDirection, modelSize, model, alpha);
    if (drawCallback != 0)
        drawCallback(&worldMatrix, view, a3, shadowMatrix, shadowX, shadowY, shadowZ, shadowTexture, shadowScale,
                     shadowSize, lightDirection, modelSize, model, alpha);
    return 1;
}
