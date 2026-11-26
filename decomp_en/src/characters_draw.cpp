// Drawing every character (EN build). Ported from the matched JP source decomp/src/character_draw.cpp.
#include "characters_draw.h"
#include "anim_model.h"
#include "camera.h"
#include "char_status.h"
#include "map_view.h"
#include "model_bank.h"
#include "render_command.h"
#include "render_queue.h"
#include "texture_mgr.h"

// Slots from here on are drawn with their own g_drawFade value.
const int FIRST_AREA_FADE_SLOT = 0x3a;
// Slots from here on are lit by the caller's light set.
const int FIRST_UNLIT_SLOT = 0x5c;
// AnimModel::shadowOpacity of a model that is placed.
const int MODEL_ACTIVE_OPACITY = 0xf;
// How far outside the visible area (x and y, then z) a character is still drawn in the overhead pass.
const float OVERHEAD_MARGIN_XY = 200.0f;
const float OVERHEAD_MARGIN_Z = 500.0f;
// The outline quad: the screen rectangle (640x480), the texture cell and the depth mapping of the projected point.
const int OUTLINE_LEFT = -1;
const int OUTLINE_TOP = -1;
const int OUTLINE_RIGHT = 0x27f;
const int OUTLINE_BOTTOM = 0x1df;
const float OUTLINE_UV_MIN = 1.0f / 1024;
const float OUTLINE_UV_MAX = 1023.0f / 1024;
const double OUTLINE_DEPTH_SCALE = 0.98;
const double OUTLINE_DEPTH_BIAS = 0.01;
// The outline opacity is stored in percent.
const float OUTLINE_PERCENT = 100.0f;

// Characters_Draw: queues the model of every character slot in the draw list, last first, under `view` * `projection`:
// slots from 0x3a on set g_drawFade to their value in g_charDrawFade (0 when the slot is drawn plain), the lights come
// from the model bank (or `defaultLight` for slots from 0x5c on). In the normal pass only placed models (shadow opacity
// 15) are drawn, and the slot g_outlineSlot gets an outline: with the depth test kept and depth writes off, a full-screen
// quad of one of the two outline textures is drawn at the slot's projected depth with g_outlinePercent percent opacity
// (added to the scene, or darkened into it when g_outlineDarken is set). In the overhead pass (g_mapPassRunning) every
// character inside the visible area plus a margin is drawn. Returns 1. Called by the scene draw.
int Characters_Draw(Matrix4 *view, const Matrix4 &projection, Vec4 *defaultLight)
{
    Matrix4 viewProjection = (*view) * projection;
    if (g_mapPassRunning != 0)
    {
        for (int i = g_charDrawCount - 1; i >= 0; i--)
        {
            int slot = g_charDrawList[i];
            Vec4 position = g_characterModels[slot].matrix.GetTranslation();
            if (g_mapVisibleArea.min.x - OVERHEAD_MARGIN_XY < position.x && g_mapVisibleArea.min.y - OVERHEAD_MARGIN_XY < position.y &&
                g_mapVisibleArea.min.z - OVERHEAD_MARGIN_Z < position.z && g_mapVisibleArea.max.x + OVERHEAD_MARGIN_XY > position.x &&
                g_mapVisibleArea.max.y + OVERHEAD_MARGIN_XY > position.y && g_mapVisibleArea.max.z + OVERHEAD_MARGIN_Z > position.z)
            {
                g_drawColour.Copy(g_charOffset[slot] + position);
                if (slot >= FIRST_AREA_FADE_SLOT)
                    g_drawFade = g_charDrawFade[slot];
                if (g_charDrawPlain[slot] != 0)
                    g_drawFade = 0;
                if (slot < FIRST_UNLIT_SLOT)
                    g_characterModels[slot].Draw(&viewProjection,
                                                 g_modelBank.LightDirection(&g_characterModels[slot], &g_cameraPositionNow, &g_cameraTargetNow), 0);
                else
                    g_characterModels[slot].Draw(&viewProjection, defaultLight, 0);
                g_drawFade = 0;
            }
        }
    }
    else
    {
        for (int i = g_charDrawCount - 1; i >= 0; i--)
        {
            int slot = g_charDrawList[i];
            if (g_characterModels[slot].shadowOpacity == MODEL_ACTIVE_OPACITY)
            {
                Vec4 position = g_characterModels[slot].matrix.GetTranslation();
                g_drawColour.Copy(g_charOffset[slot] + position);
                if (slot >= FIRST_AREA_FADE_SLOT)
                    g_drawFade = g_charDrawFade[slot];
                if (g_charDrawPlain[slot] != 0)
                    g_drawFade = 0;
                if (slot < FIRST_UNLIT_SLOT)
                    g_characterModels[slot].Draw(&viewProjection,
                                                 g_modelBank.LightDirection(&g_characterModels[slot], &g_cameraPositionNow, &g_cameraTargetNow), 0);
                else
                    g_characterModels[slot].Draw(&viewProjection, defaultLight, 0);
                if (g_outlineSlot != 0 && g_renderTargetsWork != 0 && slot == g_outlineSlot)
                {
                    Render_Command(RENDER_DEPTH_TEST, 1, 0);
                    Render_Command(RENDER_DEPTH_WRITE, 0, 0);
                    if (g_outlineDarken == 0)
                        Render_Command(RENDER_BLEND_MODE, BLEND_ADD, 0);
                    else
                        Render_Command(RENDER_BLEND_MODE, BLEND_DARKEN, 0);
                    Vec4 screen;
                    screen.Copy(Project(position, viewProjection));
                    g_textures[g_outlineTextures[g_outlineParity % 2][0]].DrawTinted(
                        OUTLINE_LEFT, OUTLINE_TOP, OUTLINE_RIGHT, OUTLINE_BOTTOM, OUTLINE_UV_MIN, OUTLINE_UV_MIN,
                        OUTLINE_UV_MAX, OUTLINE_UV_MAX, (float)(screen.z * OUTLINE_DEPTH_SCALE - OUTLINE_DEPTH_BIAS),
                        (float)g_outlinePercent / OUTLINE_PERCENT, 0, 0, 0);
                    Render_Command(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
                    Render_Command(RENDER_DEPTH_TEST, 1, 0);
                    Render_Command(RENDER_DEPTH_WRITE, 1, 0);
                }
                g_drawFade = 0;
            }
        }
    }
    return 1;
}
