// Shadow textures (EN build): a shadow caster's model drawn from its light into a texture slot. Ported from the
// matched JP source decomp/src/shadow_texture.cpp. C++ rather than extern "C": the function keeps an exception frame
// for its matrix temporaries.
#include "model_bank.h"
#include "node.h"
#include "shadow_objects.h"
#include "texture_mgr.h"
#include "vec4.h"
#include "visibility.h"

// How far the shadow camera stands from the caster, towards the light.
const float SHADOW_CAMERA_DISTANCE = 1000.0f;
// The shadow camera's projection: one degree wide, from 20 to 1200 units, with the 4:3 aspect correction.
const float SHADOW_NEAR_PLANE = 20.0f;
const float SHADOW_FAR_PLANE = 1200.0f;
const float SHADOW_FIELD_OF_VIEW = 0.017452778f;   // one degree, from pi rounded to 3.1415
// Aspect factor passed to PerspectiveMatrix for none.
const float NO_ASPECT_CHANGE = 1.0f;
const float SHADOW_ASPECT = 1.3333f;
// Casters up to this size use the fixed framing, larger ones are framed by SHADOW_FRAME_NUMERATOR / (size - SHADOW_FRAME_OFFSET).
const float SHADOW_SMALL_CASTER_SIZE = 80.0f;
const float SHADOW_FRAME_NUMERATOR = 18.0f;
const float SHADOW_FRAME_OFFSET = 26.0f;
// Height of the framed area in world units (its width is SHADOW_ASPECT times that) per unit of frame.
const float SHADOW_FRAME_HEIGHT = 3.5f;
// The final zoom applied to every shadow image (x and y).
const float SHADOW_IMAGE_ZOOM = 0.04f;
// Node::drawMode while the caster is drawn into its shadow texture (flat, in the shadow colour).
const int NODE_DRAW_MODE_SHADOW = 0x10;
// Render_Command value that clears to white; the render command codes for the cull mode and the blends used here.
const int RENDER_COLOUR_WHITE = -1;
const int RENDER_CULL_MODE = 10;
const int CULL_NONE = 1;
const int CULL_DEFAULT = 0;
const int BLEND_BLACK = 3;

// Makes texture slot `texture` the render target (returns 0 and clears g_shadowsEnabled when that fails), clears it
// to white and draws this node tree under `list` as seen from a camera SHADOW_CAMERA_DISTANCE units from
// `casterPosition` towards `lightPosition`, looking back at the caster. The camera's framing depends on `casterSize`.
// Returns 1. Called by ModelBank::PrepareFrame for each of the first ten shadow casters.
int Node::RenderShadowTexture(DrawList *list, const Matrix4 *sceneView, int a3, const Vec4 *casterPosition,
                              const Vec4 *lightPosition, int texture, float casterSize, int tileX, int tileY)
{
    if (!g_textures[texture].UpdateImage(0))
    {
        g_shadowsEnabled = 0;
        return 0;
    }
    Render_Command(RENDER_CLEAR, RENDER_COLOUR_WHITE, 0);

    Matrix4 unused;          // constructed and destroyed, never used
    Matrix4 view;            // the shadow camera
    Matrix4 viewProjection;  // view * projection
    Matrix4 projection;
    Vec4 camera;
    Vec4 caster;
    Vec4 up;

    up.x = 0;
    up.y = 1.0f;
    up.z = 0;
    caster.x = casterPosition->x;
    caster.y = casterPosition->y;
    caster.z = casterPosition->z;

    // camera = caster + (direction to the light) * SHADOW_CAMERA_DISTANCE
    camera.Copy(*lightPosition);
    camera.Copy(camera - caster);
    camera.Copy(Vec3_Normalized(&camera));
    {
        float distance = SHADOW_CAMERA_DISTANCE;
        camera.Copy(camera * distance + caster);
    }
    view = LookAt(camera, caster, &up, 0);

    projection = PerspectiveMatrix(SHADOW_NEAR_PLANE, SHADOW_FAR_PLANE, SHADOW_FIELD_OF_VIEW, NO_ASPECT_CHANGE) *
                 ScaleMatrix(1.0f, SHADOW_ASPECT, 1.0f);
    if (casterSize > SHADOW_SMALL_CASTER_SIZE)
    {
        float frame = SHADOW_FRAME_NUMERATOR / (casterSize - SHADOW_FRAME_OFFSET);
        projection = projection * ScaleMatrix(frame * SHADOW_ASPECT * SHADOW_FRAME_HEIGHT, frame * -SHADOW_FRAME_HEIGHT, 1.0f);
    }
    else
    {
        projection = projection * ScaleMatrix(1.1666664f, -0.875f, 1.0f);
    }
    projection = projection * ScaleMatrix(SHADOW_IMAGE_ZOOM, SHADOW_IMAGE_ZOOM, 1.0f);
    viewProjection = view * projection;

    Render_Command(RENDER_BLEND_MODE, BLEND_BLACK, 0);
    Render_Command(RENDER_CULL_MODE, CULL_NONE, 0);
    int savedMode = drawMode;
    drawMode = NODE_DRAW_MODE_SHADOW;
    SaveRenderMode();
    SetRenderMode(drawMode);
    Render_Command(RENDER_FLUSH_SCENE, 0, 0);
    QueueDraw(list, &viewProjection, 0, -1, 0, 0);
    drawMode = savedMode;
    RestoreRenderMode();
    Render_Command(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    Render_Command(RENDER_CULL_MODE, CULL_DEFAULT, 0);
    Render_Command(RENDER_FLUSH_SCENE, 0, 0);
    return 1;
}

// DrawShadow's framing: casters up to SHADOW_SMALL_CASTER_SIZE use SHADOW_SMALL_CASTER_FRAME, larger ones
// SHADOW_FRAME_NUMERATOR / (size - SHADOW_LARGE_FRAME_OFFSET) * SHADOW_LARGE_FRAME_GAIN.
const float SHADOW_SMALL_CASTER_FRAME = 0.5f;
const float SHADOW_LARGE_FRAME_OFFSET = 40.0f;
const float SHADOW_LARGE_FRAME_GAIN = 1.7f;
// Moves the projected shadow from clip space (-1..1) into texture space (0..1).
const float SHADOW_TEXTURE_CENTRE = 0.5f;
// Render_Command codes and values used while the shadow is laid on the ground.
const int RENDER_TEXTURE_ADDRESS_CLAMP = 1;
const int BLEND_MULTIPLY = 2;

// Builds the projector the shadow texture is laid with (the same camera as RenderShadowTexture: SHADOW_CAMERA_DISTANCE
// units from `casterPosition` towards `lightPosition`, framed by `casterSize`, then moved into texture space) and
// draws the subtree with clamped textures and the multiply blend, passing the projector, the caster's position,
// `texture`, `scale`, `size`, the light direction, `casterSize`, `model` and `alpha` down to the mesh. While the node
// has visibility cells only the enabled collision regions are drawn. Returns 1. Called by ModelBank::DrawShadows
// and DrawShadowsInto with this node's world matrix already set to the place of the shadow.
int Node::DrawShadow(DrawList *list, Matrix4 *view, int a3, const Vec4 *casterPosition, const Vec4 *lightPosition,
                     int texture, float scale, int size, float casterSize, AnimModel *model, float alpha)
{
    if (visibilityCells != 0)
    {
        UpdateVisibleCells();
        g_regionsOnlyEnabled = 1;
    }

    Matrix4 projector;       // shadow camera * projection, in texture space
    Matrix4 camera;          // the shadow camera
    Matrix4 unused;          // constructed and destroyed, never used
    Matrix4 projection;
    Vec4 lightDirection;
    Vec4 eye;
    Vec4 caster;
    Vec4 up;

    up.x = 0;
    up.y = 1.0f;
    up.z = 0;
    caster.x = casterPosition->x;
    caster.y = casterPosition->y;
    caster.z = casterPosition->z;

    // eye = caster + (direction to the light) * SHADOW_CAMERA_DISTANCE
    eye.Copy(*lightPosition);
    eye.Copy(eye - caster);
    eye.Copy(Vec3_Normalized(&eye));
    lightDirection.Copy(eye);
    {
        float distance = SHADOW_CAMERA_DISTANCE;
        eye.Copy(eye * distance + caster);
    }
    camera = LookAt(eye, caster, &up, 0);

    projection = PerspectiveMatrix(SHADOW_NEAR_PLANE, SHADOW_FAR_PLANE, SHADOW_FIELD_OF_VIEW, NO_ASPECT_CHANGE) *
                 ScaleMatrix(1.0f, SHADOW_ASPECT, 1.0f);
    if (casterSize > SHADOW_SMALL_CASTER_SIZE)
    {
        float frame = SHADOW_FRAME_NUMERATOR / (casterSize - SHADOW_LARGE_FRAME_OFFSET) * SHADOW_LARGE_FRAME_GAIN;
        projection = projection * ScaleMatrix(frame, frame, 1.0f);
    }
    else
    {
        projection = projection * ScaleMatrix(SHADOW_SMALL_CASTER_FRAME, SHADOW_SMALL_CASTER_FRAME, 1.0f);
    }
    projection = projection * ScaleMatrix(SHADOW_IMAGE_ZOOM, SHADOW_IMAGE_ZOOM, 1.0f);
    projector = camera * projection;
    projector = projector * TranslationMatrix(SHADOW_TEXTURE_CENTRE, SHADOW_TEXTURE_CENTRE, 0);

    Render_Command(RENDER_TEXTURE_ADDRESS, RENDER_TEXTURE_ADDRESS_CLAMP, 0);
    Render_Command(RENDER_BLEND_MODE, BLEND_MULTIPLY, 0);
    Draw(list, view, a3, &projector, casterPosition->x, casterPosition->y, casterPosition->z, texture, scale, size,
         &lightDirection, casterSize, model, alpha);
    Render_Command(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    Render_Command(RENDER_TEXTURE_ADDRESS, 0, 0);
    if (visibilityCells != 0)
        g_regionsOnlyEnabled = 0;
    return 1;
}

// ShadowPass_DrawNearbyObjects: draws the object slots near the shadow centre (shadowX, shadowY, shadowZ) with the shadow
// pass arguments of the caster `model`, so objects standing next to a character cast onto the same shadow. Returns 1. Not
// called directly.
extern "C" int ShadowPass_DrawNearbyObjects(DrawList *list, Matrix4 *view, int a3, const Matrix4 *shadowMatrix, float shadowX,
                                            float shadowY, float shadowZ, int shadowTexture, float shadowScale, int shadowSize,
                                            const Vec4 *lightDirection, float modelSize, AnimModel *model, float alpha)
{
    Vec4 centre;
    centre.Set(shadowX, shadowY, shadowZ, 0);
    for (int slot = OBJECT_SLOT_FIRST; slot < ENEMY_SLOT_END; slot++)
    {
        if (model != &g_characterModels[slot] && g_characterModels[slot].root != 0 &&
            g_characterModels[slot].matrix.m[MATRIX_TRANSLATION_X] > MODEL_NOT_IN_WORLD_X &&
            (centre - g_characterModels[slot].matrix.GetTranslation()).Length2D() < SHADOW_OBJECT_RANGE)
            g_characterModels[slot].root->Draw(list, view, 0, shadowMatrix, shadowX, shadowY, shadowZ, shadowTexture, shadowScale, shadowSize,
                              lightDirection, modelSize, model, alpha);
    }
    return 1;
}
