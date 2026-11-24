// Shadow textures: a shadow caster's model drawn from its light into a texture slot (the texture is then laid on the
// ground below the caster by ModelBank's shadow passes, model_bank.cpp).
// C++ rather than extern "C": the function keeps an exception frame for its matrix temporaries.

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "render_state.h"
#include "render_vars.h"
#include "texture_object.h"

Matrix4 ScaleMatrix(float x, float y, float z);                              // 0x101b13c0
Matrix4 PerspectiveMatrix(float nearPlane, float farPlane, float fov);       // 0x101b0c10
Matrix4 LookAtMatrix(Vec4 eye, Vec4 target, const Vec4 *up, float roll);     // 0x101b1c20

// How far the shadow camera stands from the caster, towards the light.
#define SHADOW_CAMERA_DISTANCE 1000.0f
// The shadow camera's projection: one degree wide, from 20 to 1200 units, with the 4:3 aspect correction.
#define SHADOW_NEAR_PLANE 20.0f
#define SHADOW_FAR_PLANE 1200.0f
#define SHADOW_FIELD_OF_VIEW 0.017452778f  // one degree, from pi rounded to 3.1415
#define SHADOW_ASPECT 1.3333f
// Casters up to this size use the fixed framing below; larger ones are framed by SHADOW_FRAME_NUMERATOR /
// (size - SHADOW_FRAME_OFFSET).
#define SHADOW_SMALL_CASTER_SIZE 80.0f
#define SHADOW_FRAME_NUMERATOR 18.0f
#define SHADOW_FRAME_OFFSET 26.0f
// The final zoom applied to every shadow image (x and y).
#define SHADOW_IMAGE_ZOOM 0.04f
// Node::drawMode while the caster is drawn into its shadow texture (flat, in the shadow colour).
#define NODE_DRAW_MODE_SHADOW 0x10

// 0x101a24e0 (Node::RenderShadowTexture). Called by ModelBank::PrepareFrame (0x10165240) for each of the first ten
// shadow casters. Makes texture slot `texture` the render target (returns 0 and clears DAT_10203434, "shadows drawn",
// when that fails), clears it to white and draws this node tree under `parent` as seen from a camera
// SHADOW_CAMERA_DISTANCE units from `casterPosition` towards `lightPosition`, looking back at the caster. The camera's
// framing depends on `casterSize`. `sceneView`, `a3`, `tileX` and `tileY` (the shadow's tile in the shadow atlas) are not used. Returns 1.
int Node::RenderShadowTexture(const Matrix4 *parent, const Matrix4 *sceneView, int a3, const Vec4 *casterPosition,
                              const Vec4 *lightPosition, int texture, float casterSize, int tileX, int tileY)
{
    if (!DAT_118d0ba8[texture].UpdateImage(0)) {
        DAT_10203434 = 0;
        return 0;
    }
    FUN_1013f070(RENDER_CLEAR, RENDER_COLOUR_WHITE, 0);

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
    view = LookAtMatrix(camera, caster, &up, 0);

    projection = PerspectiveMatrix(SHADOW_NEAR_PLANE, SHADOW_FAR_PLANE, SHADOW_FIELD_OF_VIEW) *
                 ScaleMatrix(1.0f, SHADOW_ASPECT, 1.0f);
    if (casterSize > SHADOW_SMALL_CASTER_SIZE) {
        float frame = SHADOW_FRAME_NUMERATOR / (casterSize - SHADOW_FRAME_OFFSET);
        projection = projection * ScaleMatrix(frame * 4.666655f, frame * -3.5f, 1.0f);
    } else {
        projection = projection * ScaleMatrix(1.1666664f, -0.875f, 1.0f);
    }
    projection = projection * ScaleMatrix(SHADOW_IMAGE_ZOOM, SHADOW_IMAGE_ZOOM, 1.0f);
    viewProjection = view * projection;

    FUN_1013f070(RENDER_BLEND_MODE, BLEND_BLACK, 0);
    FUN_1013f070(RENDER_CULL_MODE, CULL_NONE, 0);
    int savedDrawMode = drawMode;
    drawMode = NODE_DRAW_MODE_SHADOW;
    M_10191dd0();
    M_1019b050(drawMode);
    FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
    QueueDraw(parent, &viewProjection, 0, -1, 0, 0);
    drawMode = savedDrawMode;
    M_10191e10();
    FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    FUN_1013f070(RENDER_CULL_MODE, CULL_DEFAULT, 0);
    FUN_1013f070(RENDER_FLUSH_SCENE, 0, 0);
    return 1;
}

Matrix4 TranslationMatrix(float x, float y, float z);                       // 0x101b1360

// DrawShadow's framing: casters up to SHADOW_SMALL_CASTER_SIZE use SHADOW_SMALL_CASTER_FRAME, larger ones
// SHADOW_FRAME_NUMERATOR / (size - SHADOW_LARGE_FRAME_OFFSET) * SHADOW_LARGE_FRAME_GAIN.
#define SHADOW_SMALL_CASTER_FRAME 0.5f
#define SHADOW_LARGE_FRAME_OFFSET 40.0f
#define SHADOW_LARGE_FRAME_GAIN 1.7f
// Moves the projected shadow from clip space (-1..1) into texture space (0..1).
#define SHADOW_TEXTURE_CENTRE 0.5f

// 0x101a29e0 (Node::DrawShadow). Called by ModelBank::DrawShadows (0x10165e00) and DrawShadowsInto (0x10166100) with
// this node's world matrix already set to the place of the shadow. Builds the projector the shadow texture is laid
// with (the same camera as RenderShadowTexture: SHADOW_CAMERA_DISTANCE units from `casterPosition` towards
// `lightPosition`, framed by `casterSize`, then moved into texture space) and draws the subtree with clamped textures
// and the multiply blend, passing the projector, the caster's position, `texture`, `scale`, `size`, the light
// direction, `casterSize`, `model` and `alpha` down to the mesh. While the node has visibility cells only the enabled
// collision regions are drawn (DAT_11b04514). Returns 1.
int Node::DrawShadow(const Matrix4 *parent, const Matrix4 *view, int a3, const Vec4 *casterPosition,
                     const Vec4 *lightPosition, int texture, float scale, int size, float casterSize,
                     AnimModel *model, float alpha)
{
    if (visibilityCells != 0) {
        UpdateVisibleCells();
        DAT_11b04514 = 1;
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
    camera = LookAtMatrix(eye, caster, &up, 0);

    projection = PerspectiveMatrix(SHADOW_NEAR_PLANE, SHADOW_FAR_PLANE, SHADOW_FIELD_OF_VIEW) *
                 ScaleMatrix(1.0f, SHADOW_ASPECT, 1.0f);
    if (casterSize > SHADOW_SMALL_CASTER_SIZE) {
        float frame = SHADOW_FRAME_NUMERATOR / (casterSize - SHADOW_LARGE_FRAME_OFFSET) * SHADOW_LARGE_FRAME_GAIN;
        projection = projection * ScaleMatrix(frame, frame, 1.0f);
    } else {
        projection = projection * ScaleMatrix(SHADOW_SMALL_CASTER_FRAME, SHADOW_SMALL_CASTER_FRAME, 1.0f);
    }
    projection = projection * ScaleMatrix(SHADOW_IMAGE_ZOOM, SHADOW_IMAGE_ZOOM, 1.0f);
    projector = camera * projection;
    projector = projector * TranslationMatrix(SHADOW_TEXTURE_CENTRE, SHADOW_TEXTURE_CENTRE, 0);

    FUN_1013f070(RENDER_TEXTURE_ADDRESS, TEXTURE_ADDRESS_CLAMP, 0);
    FUN_1013f070(RENDER_BLEND_MODE, BLEND_MULTIPLY, 0);
    Draw(parent, view, a3, &projector, casterPosition->x, casterPosition->y, casterPosition->z, texture, scale,
         size, &lightDirection, casterSize, model, alpha);
    FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    FUN_1013f070(RENDER_TEXTURE_ADDRESS, TEXTURE_ADDRESS_WRAP, 0);
    if (visibilityCells != 0)
        DAT_11b04514 = 0;
    return 1;
}
