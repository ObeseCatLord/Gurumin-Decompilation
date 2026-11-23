// The overhead view of the map (0x1000f5a0, called by 0x1000f7f0): a matrix that looks straight down on the map
// node, and the map's bounds copied to the pit and visible-area globals.

#include "vec4.h"

#include "matrix4.h"

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);   // 0x101ae060: to -1..1 screen space
Matrix4 LookAt(Vec4 eye, Vec4 target, const Vec4 *up, int a);        // 0x101b1c20
Matrix4 Multiply(const Matrix4 &left, const Matrix4 &right);         // 0x101b10f0

// A scene node (scene_node.cpp Node), only what is used here.

#include "node.h"

#include "anim_model.h"

#include "model_bank.h"

struct EffectManager
{
    int Draw(const Matrix4 &view, const Matrix4 *camera, unsigned char pass);   // 0x10190e30
};

extern "C" Node *DAT_104c007c;    // the map's model
extern "C" Node *DAT_10255f08;    // the map's root node
extern "C" float DAT_104bfe24;    // the pit's floor height
extern "C" Vec4 DAT_104d29e8;     // the pit area: minimum corner
extern "C" Vec4 DAT_104d29f8;     // the pit area: maximum corner
extern "C" Vec4 DAT_10f668a8;     // the visible area: minimum corner
extern "C" Vec4 DAT_10f668b8;     // the visible area: maximum corner
extern "C" float DAT_10f65594;    // height of the view
extern "C" Vec4 DAT_10f65688;     // camera position (projected here)
extern "C" Vec4 DAT_10f668e8;     // camera target (projected here)

// 0x1000f5a0 (Map_BuildTopView). Takes the map's model (its single child when it has 0x20 children, or the map's
// root node without one): `out` becomes an overhead projection (z mirrored, moved up twice the view height) times
// `matrix`, where the view height is the middle of the model's box. The box becomes the pit area and the visible
// area (both at the view height), and the camera position and target are projected with it. Returns the node.
Node *FUN_1000f5a0(Matrix4 *out, const Matrix4 &matrix)
{
    Matrix4 placement;
    Vec4 origin;
    origin.Set(0, 0, 0, 0);
    placement.SetTranslation(origin);
    Node *map = DAT_104c007c;
    if (map->childCount == 0x20)
        map = map->children[0];
    if (map == 0)
        map = DAT_10255f08;
    Matrix4 view;
    Matrix4 projection;
    view = map->matrix;
    DAT_104bfe24 = map->boundsMin.z;
    view.Translate(0, 0, (map->boundsMax.z + map->boundsMin.z) * 0.5f);
    projection.SetIdentity();
    projection.m[14] = view.m[14] + view.m[14];
    DAT_10f65594 = view.m[14];
    projection.m[10] = -1.0f;
    *out = projection * matrix;
    DAT_104d29e8.Copy(map->boundsMin);
    DAT_104d29f8.Copy(map->boundsMax);
    DAT_10f668a8.Copy(DAT_104d29e8);
    DAT_10f668b8.Copy(DAT_104d29f8);
    DAT_10f668a8.z = DAT_10f65594;
    DAT_10f668b8.z = DAT_10f65594;
    DAT_10f65688.Copy(Project(DAT_10f65688, projection));
    DAT_10f668e8.Copy(Project(DAT_10f668e8, projection));
    return map;
}

extern "C" EffectManager DAT_10daaa80;
extern "C" Matrix4 DAT_10836ec8;           // parent matrix of the overhead pass (identity, set in game_reset.cpp)
extern "C" Matrix4 DAT_108392d8;           // the current view-projection matrix
extern "C" Matrix4 DAT_10836330;           // the camera matrix
extern "C" Matrix4 DAT_10866da8;           // the projection of the main view
extern "C" Matrix4 DAT_104c03f8;           // a second projection
extern "C" Matrix4 DAT_10836cd8;           // the camera under that projection
extern "C" Vec4 DAT_118e69e8;              // camera position
extern "C" Vec4 DAT_118e6cb8;              // camera target
extern "C" Vec4 DAT_10da90d0;              // current colour
extern "C" Vec4 DAT_10f66918;              // the overhead pass's area: minimum corner
extern "C" Vec4 DAT_10f66928;              // maximum corner
extern "C" int DAT_102022c4;               // non-zero = draw the overhead pass
extern "C" int DAT_10f654cc;               // non-zero when off-screen render targets are supported
extern "C" int DAT_10f65520;               // 1 while the overhead pass draws
extern "C" int DAT_10f655bc;               // 1 while the map nodes are queued for it
extern "C" int DAT_10daaad0;               // 1 while the effects draw into it
extern "C" char DAT_11affdd8[100];         // per collision region "CDLnn": 1 = enabled
extern "C" void FUN_1013f070(int mode, int a, int b);              // sets a 2D/3D render state
extern "C" void FUN_10199860(void);                                // RenderQueue_Clear
// 0x101999a0 (draw_list.cpp DrawList_DrawPass): draws the queued nodes of pass `pass` (the first argument is unused).
extern "C" int FUN_101999a0(const Matrix4 *unused, const Matrix4 *view, Vec4 *lights, unsigned int pass);

// 0x1000f7f0 (Map_DrawTopView). Builds the overhead view of the map; when the overhead pass is on (and render
// targets work, and the map node is not a switched-off collision region) it draws the map (with its model hidden)
// into the pass's draw list over the visible area widened by 1100 units (1200 below, 2000 above), then the effects,
// resetting the render states around each step. Then the camera matrix is rebuilt from the camera position and
// target (y up) and the view-projection matrices from it.
void FUN_1000f7f0(void)
{
    Matrix4 view;
    Node *map = FUN_1000f5a0(&view, DAT_108392d8);
    if (map != 0)
    {
        if (DAT_102022c4 != 0 && DAT_10f654cc != 0)
        {
            DAT_10f65520 = 1;
            if (DAT_10255f08->visibilityCells == 0 || map->name[0] != 'C' || map->name[1] != 'D' || map->name[2] != 'L' ||
                DAT_11affdd8[map->name[3] * 10 + map->name[4]] != 0)
            {
                FUN_1013f070(0, 0, 0);
                FUN_1013f070(10, 0, 0);
                FUN_1013f070(3, 1, 15);
                FUN_10199860();
                unsigned flags = DAT_104c007c->flags;
                DAT_104c007c->flags = flags | 0x20;
                DAT_10f655bc = 1;
                DAT_10f66918.Copy(DAT_10f668a8);
                DAT_10f66928.Copy(DAT_10f668b8);
                DAT_10f66918.z -= 1200.0f;
                DAT_10f66918.x -= 1100.0f;
                DAT_10f66918.y -= 1100.0f;
                DAT_10f66928.z += 2000.0f;
                DAT_10f66928.x += 1100.0f;
                DAT_10f66928.y += 1100.0f;
                DAT_10255f08->QueueDraw(&DAT_10836ec8, &view,
                                        DAT_10804bf8.LightDirection(&DAT_10db6ee8[0], &DAT_118e69e8, &DAT_118e6cb8), -1,
                                        &DAT_10da90d0, 0);
                DAT_104c007c->flags = flags;
                DAT_10f655bc = 0;
                FUN_1013f070(0, 0, 0);
                FUN_1013f070(10, 0, 0);
                FUN_1013f070(3, 1, 15);
                FUN_101999a0(&DAT_10836ec8, &view, &DAT_10f65688, 0);
                DAT_10f65520 = 0;
                FUN_1013f070(0, 0, 0);
                FUN_1013f070(10, 0, 0);
                FUN_1013f070(3, 1, 15);
                DAT_10daaad0 = 1;
                DAT_10daaa80.Draw(view, &DAT_10836330, 0);
                DAT_10daaad0 = 0;
                FUN_1013f070(0, 0, 0);
                FUN_1013f070(10, 0, 0);
                FUN_1013f070(3, 1, 15);
            }
        }
        DAT_10f655bc = 0;
        DAT_10f65520 = 0;
        Vec4 up;
        up.x = 0;
        up.y = 1.0f;
        up.z = 0;
        DAT_10f65688.Copy(DAT_118e69e8);
        DAT_10f668e8.Copy(DAT_118e6cb8);
        DAT_10836330 = LookAt(DAT_10f65688, DAT_10f668e8, &up, 0);
        DAT_108392d8 = Multiply(DAT_10866da8, DAT_10836330);
        DAT_10836cd8 = Multiply(DAT_104c03f8, DAT_10836330);
    }
}
