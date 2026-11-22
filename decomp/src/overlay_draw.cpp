// Drawing the model of the overlay mode (DAT_104c03c8, released in app_0010.cpp) with its own camera, a fixed
// light direction and an opacity that fades in with DAT_104c03ec. C++ rather than extern "C": the function keeps
// an exception frame for its Matrix4 locals.

#include "vec4.h"

// Matrix4 with the engine's constructor and empty destructor (node_bones.cpp).
struct Matrix4
{
    float m[16];
    Matrix4();                                   // 0x101ae770
    ~Matrix4();                                  // 0x1013eda0 (empty)
    void SetIdentity();                          // 0x101ae7a0
    Matrix4 operator*(const Matrix4 &other);     // 0x101b0670: matrix product
};

// A model node tree (scene_node.cpp Node).
struct OverlayModel
{
    // 0x10199e20: draws the tree with `world` and `view`, the light direction and colour (r, g, b, alpha).
    int Draw(Matrix4 *world, Matrix4 *view, Vec4 *light, int a, Vec4 *colour, int b);
};

struct ModelBank
{
    // 0x10164c10: the light direction for `model` from the camera position and target (char_ground.cpp).
    Vec4 *M_10164c10(void *model, Vec4 *position, Vec4 *target);
};

extern "C" OverlayModel *DAT_104c03c8;           // the overlay mode's model (0 = none)
extern "C" float DAT_104c03ec;                   // opening progress of the overlay mode
extern "C" Vec4 DAT_10c66edc;                    // overlay camera position
extern "C" Vec4 DAT_10edbdf8;                    // overlay camera target
extern "C" ModelBank DAT_10804bf8;
extern "C" void FUN_10194360(const Vec4 *eye, const Vec4 *target, Matrix4 *view, Matrix4 *projection, int a);  // Camera_SetMatrices
extern "C" void FUN_1013f070(int mode, int a, int b);   // sets a 2D/3D render state

// 0x100236c0 (Overlay_DrawModel). Draws the overlay model, if any, from the overlay camera with the render states
// of the 3D view, light direction (0.12, -1, 1) and full colour; its opacity rises as 1 - (1 - 2p)^2 with the
// opening progress p and is full from p = 0.5. Returns 0.
int Overlay_DrawModel()
{
    if (DAT_104c03c8 != 0)
    {
        Matrix4 world;
        Matrix4 view;
        Matrix4 projection;
        Matrix4 camera;
        world.SetIdentity();
        FUN_10194360(&DAT_10c66edc, &DAT_10edbdf8, &camera, &projection, 0);
        Vec4 colour;
        view = camera * projection;
        FUN_1013f070(0, 0, 0);
        FUN_1013f070(2, 0, 0);
        FUN_1013f070(7, 1, 0);
        FUN_1013f070(3, 1, 0xf);
        FUN_1013f070(0xe, 0, 0);
        FUN_1013f070(10, 0, 0);
        FUN_1013f070(0xb, 0xffff, 0);
        Vec4 *light = DAT_10804bf8.M_10164c10(0, &DAT_10c66edc, &DAT_10edbdf8);
        light->x = 0.12f;
        light->y = -1.0f;
        light->z = 1.0f;
        colour.x = 1.0f;
        colour.y = 1.0f;
        colour.z = 1.0f;
        if (DAT_104c03ec < 0.5f)
        {
            colour.w = 1.0f - (DAT_104c03ec + DAT_104c03ec);
            colour.w = 1.0f - colour.w * colour.w;
        }
        else
            colour.w = 1.0f;
        DAT_104c03c8->Draw(&world, &view, light, -1, &colour, 0);
    }
    return 0;
}
