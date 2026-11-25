// Drawing the overlay's model (EN build). Ported from decomp/src/overlay_draw.cpp.
#include "camera.h"
#include "dialogs.h"
#include "model_bank.h"
#include "texture_mgr.h"

// Render_Command codes used by the 3D view setup (see texture_mgr.h for the named ones).
const int RENDER_COMMAND_FOG = 7;          // render state 7 = 1
const int RENDER_COMMAND_LIGHTING = 3;     // render state 3 = 1, value 15
const int RENDER_COMMAND_STATE_14 = 0xe;   // render state 14 = 0
const int RENDER_COMMAND_STATE_10 = 10;    // render state 10 = 0
const int RENDER_COMMAND_STATE_11 = 0xb;   // render state 11 = 0xffff
const int RENDER_COMMAND_LIGHT_VALUE = 0xf;
const int RENDER_COMMAND_STATE_11_VALUE = 0xffff;
// Light direction of the overlay model.
const float OVERLAY_LIGHT_X = 0.12f;
const float OVERLAY_LIGHT_Y = -1.0f;
const float OVERLAY_LIGHT_Z = 1.0f;
// The overlay is half opened at this progress; before it, opacity = 1 - (1 - 2 * progress)^2.
const float OVERLAY_HALF_OPEN = 0.5f;
const float OVERLAY_OPEN_RATE = 2.0f;
// Pass mask meaning every pass.
const int OVERLAY_ALL_PASSES = -1;

// Overlay_DrawModel: draws the overlay model, if any, from the overlay camera with the render states of the 3D view,
// light direction (0.12, -1, 1) and full colour; its opacity rises as 1 - (1 - 2p)^2 with the opening progress p and is
// full from p = 0.5. Returns 0. Called every frame while the overlay shows.
int Overlay_DrawModel()
{
    if (g_overlayModel != 0)
    {
        Matrix4 world;
        Matrix4 view;
        Matrix4 projection;
        Matrix4 camera;
        world.SetIdentity();
        Camera_SetMatrices(&g_overlayCameraEye, &g_overlayCameraTarget, &camera, &projection, 0);
        Vec4 colour;
        view = camera * projection;
        Render_Command(RENDER_BLEND_MODE, 0, 0);
        Render_Command(RENDER_TEXTURE_ADDRESS, 0, 0);
        Render_Command(RENDER_COMMAND_FOG, 1, 0);
        Render_Command(RENDER_COMMAND_LIGHTING, 1, RENDER_COMMAND_LIGHT_VALUE);
        Render_Command(RENDER_COMMAND_STATE_14, 0, 0);
        Render_Command(RENDER_COMMAND_STATE_10, 0, 0);
        Render_Command(RENDER_COMMAND_STATE_11, RENDER_COMMAND_STATE_11_VALUE, 0);
        Vec4 *light = (Vec4 *)g_modelBank.LightDirection(0, &g_overlayCameraEye, &g_overlayCameraTarget);
        light->x = OVERLAY_LIGHT_X;
        light->y = OVERLAY_LIGHT_Y;
        light->z = OVERLAY_LIGHT_Z;
        colour.x = 1.0f;
        colour.y = 1.0f;
        colour.z = 1.0f;
        if (g_overlayProgress < OVERLAY_HALF_OPEN)
        {
            colour.w = 1.0f - g_overlayProgress * OVERLAY_OPEN_RATE;
            colour.w = 1.0f - colour.w * colour.w;
        }
        else
            colour.w = 1.0f;
        g_overlayModel->QueueDraw(&world, &view, light, OVERLAY_ALL_PASSES, &colour, 0);
    }
    return 0;
}
