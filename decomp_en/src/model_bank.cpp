// The model bank (EN build): construction, textures, lens flares and shadows. Ported from the matched JP source
// decomp/src/model_bank.cpp.
#include <stdio.h>

#include "engine.h"
#include "map_view.h"
#include "model_bank.h"
#include "render.h"
#include "texture_mgr.h"

// Side of a shadow or mirror render target in pixels.
const int BANK_TARGET_SIZE = 0x100;
// Alpha mode and flags passed to Texture_CreateFromPixels for a render target.
const int TARGET_ALPHA = 1;
const int TARGET_FLAGS = 0;
// Bytes of a render target name.
const int TARGET_NAME_SIZE = 0x80;
// Folder argument of Path_Data for the data images, and no alpha image / reload for Texture_LoadBmp.
const int PATH_FOLDER_DATA = 0;

// Loads BMP texture `name` from the data folder.
inline int LoadBmp(const char *name)
{
    return Texture_LoadBmp(Path_Data(name, PATH_FOLDER_DATA), 0, 0);
}

// Constructs the bank: the vector tables and model records are constructed (zeroed vectors) and the bank starts
// disabled. Called when the game starts.
ModelBank::ModelBank()
{
    enabled = 0;
}

// Creates the bank's textures for a new game: the ten shadow targets (when shadows are on), the lens-flare, shadow and
// cel-shading images, and, when render-to-texture works, the crossfade, mirror, save-thumbnail and minimap targets; the
// shadow targets are cleared once. Returns 0 when one of the needed targets could not be created, else 1.
int ModelBank::CreateTextures()
{
    int pixel;
    char name[TARGET_NAME_SIZE];
    m_313e8 = 0;
    g_bankFlag = 0;
    if (g_shadowsEnabled != 0)
    {
        for (int i = 0; i < MODEL_BANK_SHADOW_TARGETS; i++)
        {
            sprintf(name, "shadow%0d", i);
            shadows[i] = Texture_CreateFromPixels(name, BANK_TARGET_SIZE, BANK_TARGET_SIZE, (unsigned char *)&pixel,
                                                  TARGET_ALPHA, TARGET_FLAGS);
            g_shadowTargetsCopy[i] = shadows[i];
        }
    }
    int loading = g_sharedTextureLoading;
    g_sharedTextureLoading = 1;
    lens[0] = LoadBmp("lens01.bmp");
    lens[1] = LoadBmp("lens02.bmp");
    lens[2] = LoadBmp("lens03.bmp");
    lens[3] = LoadBmp("lens04.bmp");
    shadowTexture = LoadBmp("kage.bmp");
    g_celTexture = LoadBmp("cel.bmp");
    g_celTexture2 = LoadBmp("cel2.bmp");
    g_sharedTextureLoading = loading;
    if (g_renderTargetsWork != 0 && g_renderToTextureWorks != 0)
    {
        if (g_crossfadeTargets != 0)
        {
            crossfade[0] = Texture_CreateFromPixels("crossfade0", (int)g_maxViewportSize, (int)g_maxViewportSize,
                                                    (unsigned char *)&pixel, TARGET_ALPHA, TARGET_FLAGS);
            crossfade[1] = Texture_CreateFromPixels("crossfade1", (int)g_maxViewportSize, (int)g_maxViewportSize,
                                                    (unsigned char *)&pixel, TARGET_ALPHA, TARGET_FLAGS);
            crossfade[2] = Texture_CreateFromPixels("crossfade2", (int)g_maxViewportSize, (int)g_maxViewportSize,
                                                    (unsigned char *)&pixel, TARGET_ALPHA, TARGET_FLAGS);
        }
        mirror = Texture_CreateFromPixels("mirror", BANK_TARGET_SIZE, BANK_TARGET_SIZE, (unsigned char *)&pixel,
                                          TARGET_ALPHA, TARGET_FLAGS);
        saveBuffer = Texture_CreateFromPixels("savebuf", BANK_TARGET_SIZE, BANK_TARGET_SIZE, (unsigned char *)&pixel,
                                              TARGET_ALPHA, TARGET_FLAGS);
        minimap = Texture_CreateFromPixels("minimap", BANK_TARGET_SIZE, BANK_TARGET_SIZE, (unsigned char *)&pixel,
                                           TARGET_ALPHA, TARGET_FLAGS);
        mirror2 = Texture_CreateFromPixels("mirror2", BANK_TARGET_SIZE, BANK_TARGET_SIZE, (unsigned char *)&pixel,
                                           TARGET_ALPHA, TARGET_FLAGS);
        saveBuffer2 = Texture_CreateFromPixels("savebuf2", BANK_TARGET_SIZE, BANK_TARGET_SIZE, (unsigned char *)&pixel,
                                               TARGET_ALPHA, TARGET_FLAGS);
        minimap2 = Texture_CreateFromPixels("minimap2", BANK_TARGET_SIZE, BANK_TARGET_SIZE, (unsigned char *)&pixel,
                                            TARGET_ALPHA, TARGET_FLAGS);
        g_mirrorTargetB = g_mirrorTargetA = mirror;
        if (g_shadowsEnabled != 0)
        {
            for (int i = 0; i < MODEL_BANK_SHADOW_TARGETS; i++)
            {
                TextureObject &shadow = g_textures[shadows[i]];
                shadow.BeginRenderTo(shadow.sourceWidth, shadow.sourceHeight, 0);
                Render_Command(RENDER_CLEAR, -1, 0);
                Render_Command(RENDER_FLUSH_SCENE, 0, 0);
            }
        }
        if (g_renderToBackBuffer != 0)
            Render_ResetTarget();
        else
            Render_ResetSwapTarget();
        if (g_crossfadeTargets != 0)
        {
            if (crossfade[0] < 0 || crossfade[1] < 0 || crossfade[2] < 0 || mirror < 0 || saveBuffer < 0 || minimap < 0)
                return 0;
        }
        else if (minimap2 < 0 || saveBuffer2 < 0 || mirror2 < 0 || minimap < 0 || saveBuffer < 0 || mirror < 0)
            return 0;
    }
    return 1;
}

// Number of discs drawn for each flare point, and the length of the disc tables (the extra entries stay zero).
const int FLARE_DISC_COUNT = 8;
const int FLARE_TABLE_SIZE = 10;
// Flickering alpha of the first disc: 0.25 plus up to 0.05 from the random number.
const int FLARE_FLICKER_STEPS = 10;
const float FLARE_FLICKER_DIVISOR = 200.0f;
const float FLARE_FLICKER_BASE = 0.25f;
// Screen centre and half extents of the 640x480 screen the disc positions are computed in.
const float SCREEN_CENTRE_X = 320.0f;
const float SCREEN_CENTRE_Y = 240.0f;
// How far a disc's step moves it along x and y per unit of step.
const float FLARE_STEP_X = 32.0f;
const float FLARE_STEP_Y = 24.0f;

// For every flare point collected this frame draws eight discs from the lens textures on the line from the point
// through the screen centre: each with its size (half-width in 640x480 pixels, scaled by the point's brightness),
// position along the line and opacity; the first disc flickers a little. Returns 0 when the bank or the flares are
// off, else 1. Called by the scene drawing after the 3D pass.
int ModelBank::DrawLensFlares()
{
    if (enabled == 0)
        return 0;
    if (g_lensFlaresEnabled == 0)
        return 0;
    Render_Command(RENDER_BLEND_MODE, BLEND_ADD, 0);
    Render_Command(RENDER_DEPTH_WRITE, 0, 0);
    int sizes[FLARE_TABLE_SIZE] = {220, 100, 50, 30, 20, 20, 80, 160};
    int steps[FLARE_TABLE_SIZE] = {0, -4, 4, 6, 8, 11, 15, 19};
    int lenses[FLARE_TABLE_SIZE] = {0, 3, 1, 2, 3, 2, 2, 1};
    float alphas[FLARE_TABLE_SIZE] = {0.0f, 0.4f, 0.4f, 0.4f, 0.3f, 0.2f, 0.2f, 0.15f};
    alphas[0] = (float)(Random() % FLARE_FLICKER_STEPS) / FLARE_FLICKER_DIVISOR + FLARE_FLICKER_BASE;
    for (int flare = 0; flare < flareCount; flare++)
    {
        for (int i = 0; i < FLARE_DISC_COUNT; i++)
        {
            float size = (float)sizes[i] * flareAlpha[flare];
            float step = (float)steps[i];
            g_textures[lens[lenses[i]]].DrawScreenCopy(
                (int)(flareScreen[flare].x * SCREEN_CENTRE_X + SCREEN_CENTRE_X - size -
                      flareScreen[flare].x * FLARE_STEP_X * step),
                (int)(SCREEN_CENTRE_Y - flareScreen[flare].y * SCREEN_CENTRE_Y - size +
                      flareScreen[flare].y * FLARE_STEP_Y * step),
                (int)(flareScreen[flare].x * SCREEN_CENTRE_X + SCREEN_CENTRE_X + size -
                      flareScreen[flare].x * FLARE_STEP_X * step),
                (int)(SCREEN_CENTRE_Y - flareScreen[flare].y * SCREEN_CENTRE_Y + size +
                      flareScreen[flare].y * FLARE_STEP_Y * step),
                0.0f, alphas[i]);
        }
    }
    Render_Command(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    Render_Command(RENDER_DEPTH_WRITE, 1, 0);
    return 1;
}

// State written to AnimModel::m_17e8 when DrawShadows resets a caster (DrawShadowsInto writes 0).
const char CASTER_RESET_STATE_SHADOWS = 2;
// DrawShadowsInto raises the caster's shadow matrix by this much along z before drawing.
const float SHADOW_INTO_LIFT = 6.1f;
// Model alpha a reset caster gets when the shadows have their own render targets.
const float CASTER_RESET_ALPHA = 1.0f;

// Draws the shadows of the first ten shadow casters through `scene`: with shadow targets each caster uses its own
// target, else all use the shadow blob (kage.bmp). A caster without a timeout is first reset: identity matrix at its
// position parameter, shown, opacity 15 (with targets also model alpha 1). Casters with model data that are shown and
// not transparent are drawn. Returns 0 when shadows are off, else 1. Called every frame by the world drawing.
int ModelBank::DrawShadows(Node *scene, DrawList *list, Matrix4 *view, int a3)
{
    if (enabled == 0)
        return 0;
    if (g_shadowsSwitchedOn == 0)
        return 0;
    if (g_blueSquareSeen == 0)
        return 0;
    if (g_shadowsEnabled != 0)
    {
        for (int i = 0; i < shadowCasterCount && i < MODEL_BANK_SHADOW_TARGETS; i++)
        {
            if (litModels[casters[i]]->collisionLimit == MODEL_BANK_NO_TIMEOUT)
            {
                litModels[casters[i]]->m_17e8 = CASTER_RESET_STATE_SHADOWS;
                litModels[casters[i]]->matrix.SetIdentity();
                litModels[casters[i]]->matrix.SetTranslation(*params[litModels[casters[i]]->bankIndex]);
                litModels[casters[i]]->shadowHidden = 0;
                litModels[casters[i]]->shadowOpacity = SHADOW_OPACITY_PLACED;
                litModels[casters[i]]->alpha = CASTER_RESET_ALPHA;
            }
            AnimModel *model = litModels[casters[i]];
            if (model->loadedRoot != 0 && model->shadowHidden == 0 && model->shadowOpacity > 0)
                scene->DrawShadow(list, view, a3, &casterPosition[i], &casterParams[i], shadows[i], model->uniformScale,
                                  (int)casterSize[i], model->loadedRoot->bounds.w, model, model->alpha);
        }
    }
    else
    {
        for (int i = 0; i < shadowCasterCount && i < MODEL_BANK_SHADOW_TARGETS; i++)
        {
            if (litModels[casters[i]]->collisionLimit == MODEL_BANK_NO_TIMEOUT)
            {
                litModels[casters[i]]->m_17e8 = CASTER_RESET_STATE_SHADOWS;
                litModels[casters[i]]->matrix.SetIdentity();
                litModels[casters[i]]->matrix.SetTranslation(*params[litModels[casters[i]]->bankIndex]);
                litModels[casters[i]]->shadowHidden = 0;
                litModels[casters[i]]->shadowOpacity = SHADOW_OPACITY_PLACED;
            }
            AnimModel *model = litModels[casters[i]];
            if (model->loadedRoot != 0 && model->shadowHidden == 0 && model->shadowOpacity > 0)
                scene->DrawShadow(list, view, a3, &casterPosition[i], &casterParams[i], shadowTexture, model->uniformScale,
                                  (int)casterSize[i], model->loadedRoot->bounds.w, model, model->alpha);
        }
    }
    return 1;
}

// Like DrawShadows, drawing each shadow with the caster's shadow matrix raised 6.1 units, set as `scene`'s world
// matrix, and with the depth writes off around it. A caster reset here also takes its shadow matrix from its world
// matrix. Returns 0 when shadows are off or there is no scene, else 1. Called by the map top view drawing.
int ModelBank::DrawShadowsInto(Node *scene, DrawList *list, Matrix4 *view, int a3)
{
    if (enabled == 0)
        return 0;
    if (scene == 0)
        return 0;
    if (g_shadowsSwitchedOn == 0)
        return 0;
    if (g_blueSquareSeen == 0)
        return 0;
    if (g_shadowsEnabled != 0)
    {
        for (int i = 0; i < shadowCasterCount && i < MODEL_BANK_SHADOW_TARGETS; i++)
        {
            if (litModels[casters[i]]->collisionLimit == MODEL_BANK_NO_TIMEOUT)
            {
                litModels[casters[i]]->m_17e8 = 0;
                litModels[casters[i]]->matrix.SetIdentity();
                litModels[casters[i]]->matrix.SetTranslation(*params[litModels[casters[i]]->bankIndex]);
                litModels[casters[i]]->shadowMatrix = litModels[casters[i]]->matrix;
                litModels[casters[i]]->shadowHidden = 0;
                litModels[casters[i]]->shadowOpacity = SHADOW_OPACITY_PLACED;
                litModels[casters[i]]->alpha = CASTER_RESET_ALPHA;
            }
            if (litModels[casters[i]]->loadedRoot != 0 && litModels[casters[i]]->shadowHidden == 0 &&
                litModels[casters[i]]->shadowOpacity > 0)
            {
                scene->worldMatrix = litModels[casters[i]]->shadowMatrix;
                scene->worldMatrix.Translate(0, 0, SHADOW_INTO_LIFT);
                Render_Command(RENDER_DEPTH_WRITE, 0, 0);
                scene->DrawShadow(list, view, a3, &casterPosition[i], &casterParams[i], shadows[i],
                                  litModels[casters[i]]->uniformScale, (int)casterSize[i],
                                  litModels[casters[i]]->loadedRoot->bounds.w, litModels[casters[i]],
                                  litModels[casters[i]]->alpha);
                Render_Command(RENDER_DEPTH_WRITE, 1, 0);
            }
        }
    }
    else
    {
        for (int i = 0; i < shadowCasterCount && i < MODEL_BANK_SHADOW_TARGETS; i++)
        {
            if (litModels[casters[i]]->collisionLimit == MODEL_BANK_NO_TIMEOUT)
            {
                litModels[casters[i]]->m_17e8 = 0;
                litModels[casters[i]]->matrix.SetIdentity();
                litModels[casters[i]]->matrix.SetTranslation(*params[litModels[casters[i]]->bankIndex]);
                litModels[casters[i]]->shadowMatrix = litModels[casters[i]]->matrix;
                litModels[casters[i]]->shadowHidden = 0;
                litModels[casters[i]]->shadowOpacity = SHADOW_OPACITY_PLACED;
            }
            if (litModels[casters[i]]->loadedRoot != 0 && litModels[casters[i]]->shadowHidden == 0 &&
                litModels[casters[i]]->shadowOpacity > 0)
            {
                scene->worldMatrix = litModels[casters[i]]->shadowMatrix;
                scene->worldMatrix.Translate(0, 0, SHADOW_INTO_LIFT);
                Render_Command(RENDER_DEPTH_WRITE, 0, 0);
                scene->DrawShadow(list, view, a3, &casterPosition[i], &casterParams[i], shadowTexture,
                                  litModels[casters[i]]->uniformScale, (int)casterSize[i],
                                  litModels[casters[i]]->loadedRoot->bounds.w, litModels[casters[i]],
                                  litModels[casters[i]]->alpha);
                Render_Command(RENDER_DEPTH_WRITE, 1, 0);
            }
        }
    }
    return 1;
}
