// Overhead view of the map (EN build). JP twin: map_view.cpp.
#include "camera.h"
#include "char.h"
#include "effect_manager.h"
#include "game_vars.h"
#include "model_bank.h"
#include "render_queue.h"
#include "texture_mgr.h"
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

// Builds the overhead view of the map. When the overhead pass is on (and render targets work, and the map node is not a
// switched-off collision region) it draws the map (with its model hidden) into the pass's draw list over the visible area
// widened by 1100 units (1200 below, 2000 above), then the effects, resetting the render states around each step. Then
// the camera matrix is rebuilt from the camera position and target (y up) and the view-projection matrices from it.
void Map_DrawTopView(void)
{
    Matrix4 view;
    Node *map = Map_BuildTopView(&view, g_cameraViewProjection);
    if (map != 0)
    {
        if (g_overheadPassOn != 0 && g_renderTargetsWork != 0)
        {
            g_mapPassRunning = 1;
            if (g_worldRoot->visibilityCells == 0 || map->name[0] != 'C' || map->name[1] != 'D' || map->name[2] != 'L' ||
                g_regionSwitches[(map->name[3] - '0') * 10 + (map->name[4] - '0')] != 0)
            {
                Render_Command(0, 0, 0);
                Render_Command(10, 0, 0);
                Render_Command(3, 1, 15);
                RenderQueue_Clear();
                unsigned int flags = g_mapNode->flags;
                g_mapNode->flags = flags | NODE_FLAG_HIDDEN;
                g_mapHidden = 1;
                g_mapDrawArea.min.Copy(g_mapVisibleArea.min);
                g_mapDrawArea.max.Copy(g_mapVisibleArea.max);
                g_mapDrawArea.min.z -= MAP_DRAW_BELOW;
                g_mapDrawArea.min.x -= MAP_DRAW_MARGIN;
                g_mapDrawArea.min.y -= MAP_DRAW_MARGIN;
                g_mapDrawArea.max.z += MAP_DRAW_ABOVE;
                g_mapDrawArea.max.x += MAP_DRAW_MARGIN;
                g_mapDrawArea.max.y += MAP_DRAW_MARGIN;
                g_worldRoot->QueueDraw(&g_mapDrawList, &view,
                                       g_modelBank.LightDirection(&g_characterModels[0], &g_cameraPositionNow, &g_cameraTargetNow),
                                       -1, &g_fadeColour, 0);
                g_mapNode->flags = flags;
                g_mapHidden = 0;
                Render_Command(0, 0, 0);
                Render_Command(10, 0, 0);
                Render_Command(3, 1, 15);
                DrawList_DrawPass(&g_mapDrawList, &view, &g_cameraEye, 0);
                g_mapPassRunning = 0;
                Render_Command(0, 0, 0);
                Render_Command(10, 0, 0);
                Render_Command(3, 1, 15);
                g_effectPassRunning = 1;
                g_effectManager.Draw(view, &g_cameraView, 0);
                g_effectPassRunning = 0;
                Render_Command(0, 0, 0);
                Render_Command(10, 0, 0);
                Render_Command(3, 1, 15);
            }
        }
        g_mapHidden = 0;
        g_mapPassRunning = 0;
        Vec4 up;
        up.x = 0;
        up.y = 1.0f;
        up.z = 0;
        g_cameraEye.Copy(g_cameraPositionNow);
        g_cameraTarget.Copy(g_cameraTargetNow);
        g_cameraView = LookAt(g_cameraEye, g_cameraTarget, &up, 0);
        g_cameraViewProjection = Multiply(g_projection, g_cameraView);
        g_cameraViewBase = Multiply(g_baseProjection, g_cameraView);
    }
}
