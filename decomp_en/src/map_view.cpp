// Overhead view of the map (EN build). JP twin: map_view.cpp.
#include "camera.h"
#include "collide_regions.h"
#include "map_view.h"

// Takes the map's model (its single child when the map root has 0x20 children, else the root itself; the world root
// when none): `out` becomes an overhead projection (z mirrored, moved up twice the view height) times `matrix`, where
// the view height is the middle of the model's box. The box becomes the pit area and the visible area (both at the
// view height) and the camera position and target are projected with it. Returns the node.
Node *Map_BuildTopView(Matrix4 *out, const Matrix4 &matrix)
{
    Matrix4 placement;
    Vec4 origin;
    origin.Set(0, 0, 0, 0);
    placement.SetTranslation(origin);
    Node *map = g_mapNode;
    if (map->childCount == MAP_ROOT_CHILD_COUNT)
        map = map->children[0];
    if (map == 0)
        map = g_worldRoot;
    Matrix4 view;
    Matrix4 projection;
    view = map->matrix;
    g_mapBottomZ = map->box[0].z;
    view.Translate(0, 0, (map->box[1].z + map->box[0].z) * MAP_VIEW_HEIGHT_HALF);
    projection.SetIdentity();
    projection.m[MATRIX_TRANSLATION_Z] = view.m[MATRIX_TRANSLATION_Z] * MAP_PROJECTION_W_SCALE;
    projection.m[10] = MAP_PROJECTION_MIRROR;
    g_topViewHeight = view.m[MATRIX_TRANSLATION_Z];
    *out = projection * matrix;
    g_mapPitMin.Copy(map->box[0]);
    g_mapPitMax.Copy(map->box[1]);
    g_mapVisibleArea.min.Copy(g_mapPitMin);
    g_mapVisibleArea.max.Copy(g_mapPitMax);
    g_mapVisibleArea.min.z = g_topViewHeight;
    g_mapVisibleArea.max.z = g_topViewHeight;
    g_cameraEye.Copy(Project(g_cameraEye, projection));
    g_cameraTarget.Copy(Project(g_cameraTarget, projection));
    return map;
}
