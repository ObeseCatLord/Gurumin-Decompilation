// Drawing the trail of an effect light (the weapon streaks and glints): the trail is a list of world points with a
// width and two colours each, projected to the 640x480 screen and drawn as one textured triangle strip.

#include "vec4.h"

#include "matrix4.h"

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);   // 0x101ae060: to -1..1 screen space

#include "texture_object.h"

#include "key_track.h"

#include "map_vars.h"

#include "render_state.h"

#include "effect_manager.h"

// The 640x480 screen the strip is laid out on, and how far outside it a vertex may land before it is hidden.
#define TRAIL_SCREEN_WIDTH 640.0f
#define TRAIL_SCREEN_HEIGHT 480.0f
#define TRAIL_HIDE_DISTANCE 1000.0f
// A projected depth in this range is in front of the camera; anything else hides the vertex.
#define TRAIL_NEAR_DEPTH 0.9
#define TRAIL_FAR_DEPTH 1.0
// How far outside the visible area (and in front of it) a point may be and still be drawn.
#define TRAIL_CULL_MARGIN 200.0f
#define TRAIL_CULL_MARGIN_NEAR 20.0f
#define TRAIL_CULL_MARGIN_FAR 1000.0f
// Scale of EffectLight::widths (stored in 1/1000).
#define TRAIL_WIDTH_SCALE 0.001f
// Turning the direction of travel a quarter turn (clockwise) gives the way the trail widens.
#define TRAIL_SIDE_TURN (-1.5707964f)
// Keys of the interleaved trail's banks are 40 frames apart; the trail is sampled four times as densely (10 apart).
#define TRAIL_KEY_SPACING 0x28
#define TRAIL_SAMPLE_SPACING 10
// Weight of the previous side when two neighbouring segments are blended.
#define TRAIL_BLEND_WEIGHT 0.5f
// A colour channel of 1.0 is 255.
#define TRAIL_CHANNEL_SCALE 255.0f

// 0x10187520 (EffectManager::DrawLight). Draws light `index` as a textured strip. Kinds 7, 9, 0x16 and 0x17 go to
// their own functions. Otherwise the points from the first one still alive (the trail shortens by one point per
// frame once the light stops, see EffectLight::frame) are projected with `view`; unless the manager is hiding
// off-screen effects, the first point must be inside the visible area. The strip has two vertices per point, spread
// to both sides of the direction of travel by the point's width; the vertex colours are the point colours with the
// light's fade in the alpha. Kind 0x11 instead samples two interleaved key banks four times as densely and puts one
// vertex per sample on alternate sides. The strip is drawn with the definition's blend mode and texture. Called by
// EffectManager::DrawRecord and EffectManager::Draw (pass 0). Returns 0.
int EffectManager::DrawLight(int index, const Matrix4 &view, const Matrix4 *camera)
{

    int type = DAT_118e99f8[index].type;
    if (DAT_1101dfdc[type].kind == EFFECT_KIND_LIGHT_23)
        return DrawLightKind23(index, view, camera);
    if (DAT_1101dfdc[type].kind == EFFECT_KIND_LIGHT_7 || DAT_1101dfdc[type].kind == EFFECT_KIND_LIGHT_9)
        return DrawLightKind7(index, view, camera);
    if (DAT_1101dfdc[type].kind == EFFECT_KIND_LIGHT_22)
        return DrawLightKind22(index, view, camera);
    if (DAT_1101dfdc[type].depthTest != 0)
        FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
    else
        FUN_1013f070(RENDER_DEPTH_WRITE, 0, 0);
    int vertexCount;
    if (DAT_1101dfdc[type].kind == EFFECT_KIND_LIGHT_INTERLEAVED)
    {
        if (DAT_118e99f8[index].pointCount < 3)
            return 0;
        Vec4 projected[2];
        Vec4 screen;
        int first = (DAT_118e99f8[index].frame - DAT_1101dfdc[type].lightFrames) + DAT_118e99f8[index].pointCount;
        first = first > 0 ? first : 0;
        int point = first * 2;
        if (m_0050 != 0)
        {
            if (!(DAT_10f668a8.min.x - TRAIL_CULL_MARGIN < DAT_118e99f8[index].points[point].x && DAT_10f668a8.min.y - TRAIL_CULL_MARGIN < DAT_118e99f8[index].points[point].y &&
                  DAT_10f668a8.min.z - TRAIL_CULL_MARGIN_NEAR < DAT_118e99f8[index].points[point].z && DAT_118e99f8[index].points[point].x < DAT_10f668a8.max.x + TRAIL_CULL_MARGIN &&
                  DAT_118e99f8[index].points[point].y < DAT_10f668a8.max.y + TRAIL_CULL_MARGIN && DAT_118e99f8[index].points[point].z < DAT_10f668a8.max.z + TRAIL_CULL_MARGIN_FAR))
                return 0;
        }
        projected[0].Copy(Project(DAT_118e99f8[index].points[point], view));
        vertexCount = 0;
        Vec4 scratch;
        Vec4 none;
        scratch.Set(1.0f, 1.0f, 1.0f, 1.0f);
        KeyBank banks[2];
        for (int i = point; i < DAT_118e99f8[index].pointCount; i++)
            banks[i & 1].Set((i / 2) * TRAIL_KEY_SPACING, DAT_118e99f8[index].points[i].x,
                             DAT_118e99f8[index].points[i].y, DAT_118e99f8[index].points[i].z, 0);
        for (; point < DAT_118e99f8[index].pointCount * 4; point++)
        {
            Vec4 sample;
            banks[point & 1].Sample((point / 2) * TRAIL_SAMPLE_SPACING, &sample.x, &sample.y, &sample.z, &sample.w, 0);
            screen.Copy(Project(sample, view));
            scratch.Copy(none);
            ScreenVertex *vertex = DAT_110187c0 + vertexCount;
            float x = screen.x * TRAIL_SCREEN_WIDTH / 2.0f + 320.0f;
            float y = 240.0f - screen.y * TRAIL_SCREEN_HEIGHT / 2.0f;
            vertex->z = screen.z;
            vertex->rhw = 1.0f;
            vertex->u = 0.5f;
            vertex->x = x;
            vertex->y = y;
            vertex->v = (point & 1) ? 1.0f : 0.0f;
            int sampleIndex = point / 4;
            float red = DAT_118e99f8[index].colours[sampleIndex].x >= 1.0f ? 1.0f : DAT_118e99f8[index].colours[sampleIndex].x;
            float green = DAT_118e99f8[index].colours[sampleIndex].y >= 1.0f ? 1.0f : DAT_118e99f8[index].colours[sampleIndex].y;
            float blue = DAT_118e99f8[index].colours[sampleIndex].z >= 1.0f ? 1.0f : DAT_118e99f8[index].colours[sampleIndex].z;
            float fadedAlpha = DAT_118e99f8[index].colours[sampleIndex].w * DAT_118e99f8[index].fade;
            float alpha = fadedAlpha >= 1.0f ? 1.0f : fadedAlpha;
            vertex->diffuse = (((int)(alpha * TRAIL_CHANNEL_SCALE) << 8 | (int)(red * TRAIL_CHANNEL_SCALE)) << 8 |
                               (int)(green * TRAIL_CHANNEL_SCALE)) << 8 | (int)(blue * TRAIL_CHANNEL_SCALE);
            float specularBlue = DAT_118e99f8[index].specular[point].z >= 1.0f ? 1.0f : DAT_118e99f8[index].specular[point].z;
            float specularGreen = DAT_118e99f8[index].specular[sampleIndex].y >= 1.0f ? 1.0f : DAT_118e99f8[index].specular[sampleIndex].y;
            float specularRed = DAT_118e99f8[index].specular[sampleIndex].x >= 1.0f ? 1.0f : DAT_118e99f8[index].specular[sampleIndex].x;
            vertex->specular = ((int)(specularBlue * TRAIL_CHANNEL_SCALE) << 8 |
                                (int)(specularGreen * TRAIL_CHANNEL_SCALE)) << 8 |
                               (int)(specularRed * TRAIL_CHANNEL_SCALE);
            if (x < -TRAIL_HIDE_DISTANCE || x > TRAIL_HIDE_DISTANCE)
                vertex->alpha = 0;
            if (y < -TRAIL_HIDE_DISTANCE || y > TRAIL_HIDE_DISTANCE)
                vertex->alpha = 0;
            if (screen.z < TRAIL_NEAR_DEPTH)
                vertex->alpha = 0;
            if (screen.z > TRAIL_FAR_DEPTH || screen.z < TRAIL_NEAR_DEPTH)
                vertex->alpha = 0;
            vertexCount++;
        }
        int blend = DAT_1101dfdc[type].blend;
        if (blend == 0)
            FUN_1013f070(RENDER_BLEND_MODE, 0, 0);
        else if (blend == 1)
            FUN_1013f070(RENDER_BLEND_MODE, 1, 0);
        else if (blend == 2)
            FUN_1013f070(RENDER_BLEND_MODE, 2, 0);
        else if (blend == 3)
            FUN_1013f070(RENDER_BLEND_MODE, 10, 0);
        else if (blend == 4)
            FUN_1013f070(RENDER_BLEND_MODE, 11, 0);
        else if (blend == 5)
            FUN_1013f070(RENDER_BLEND_MODE, 12, 0);
        else if (blend == 6)
            FUN_1013f070(RENDER_BLEND_MODE, 13, 0);
    }
    else
    {
        if (DAT_118e99f8[index].pointCount < 3)
            return 0;
        Vec4 projected[2];
        Vec4 side;
        int first = (DAT_118e99f8[index].frame - DAT_1101dfdc[type].lightFrames) + DAT_118e99f8[index].pointCount;
        first = first > 0 ? first : 0;
        if (m_0050 != 0)
        {
            if (!(DAT_10f668a8.min.x - TRAIL_CULL_MARGIN < DAT_118e99f8[index].points[first].x && DAT_10f668a8.min.y - TRAIL_CULL_MARGIN < DAT_118e99f8[index].points[first].y &&
                  DAT_10f668a8.min.z - TRAIL_CULL_MARGIN_NEAR < DAT_118e99f8[index].points[first].z && DAT_118e99f8[index].points[first].x < DAT_10f668a8.max.x + TRAIL_CULL_MARGIN &&
                  DAT_118e99f8[index].points[first].y < DAT_10f668a8.max.y + TRAIL_CULL_MARGIN && DAT_118e99f8[index].points[first].z < DAT_10f668a8.max.z + TRAIL_CULL_MARGIN_FAR))
                return 0;
        }
        projected[0].Copy(Project(DAT_118e99f8[index].points[first], view));
        vertexCount = 0;
        Vec4 normal;
        Vec4 previousNormal;
        previousNormal.Set(1.0f, 1.0f, 1.0f, 1.0f);
        bool havePrevious = false;
        int segment = 0;
        for (int i = first; i < DAT_118e99f8[index].pointCount - 1; i++)
        {
            projected[(segment - 1) & 1].Copy(Project(DAT_118e99f8[index].points[i + 1], view));
            side.Copy(projected[(segment - 1) & 1] - projected[segment & 1]);
            side.RotateZ(TRAIL_SIDE_TURN);
            side.Normalized3();
            normal.Copy(side);
            if (havePrevious)
            {
                float half = TRAIL_BLEND_WEIGHT;
                side.Copy((side + previousNormal) * half);
            }
            havePrevious = true;
            previousNormal.Copy(normal);
            float width = DAT_118e99f8[index].widths[i].x * TRAIL_WIDTH_SCALE;
            side.Copy(side * width);
            const float *centre = &projected[segment & 1].x;
            const float *depth = &projected[segment & 1].z;
            ScreenVertex *vertex = DAT_110187c0 + vertexCount;
            float x = (side.x + projected[segment & 1].x) * TRAIL_SCREEN_WIDTH / 2.0f + 320.0f;
            float y = 240.0f - (side.y + projected[segment & 1].y) * TRAIL_SCREEN_HEIGHT / 2.0f;
            vertex->x = x;
            vertex->y = y;
            vertex->z = *depth;
            vertex->rhw = 1.0f;
            vertex->u = 0.0f;
            vertex->v = 0.5f;
            if (i == first)
                vertex->v = 0.0f;
            else if (i == DAT_118e99f8[index].pointCount - 2)
                vertex->v = 1.0f;
            float red = DAT_118e99f8[index].colours[i].x >= 1.0f ? 1.0f : DAT_118e99f8[index].colours[i].x;
            float green = DAT_118e99f8[index].colours[i].y >= 1.0f ? 1.0f : DAT_118e99f8[index].colours[i].y;
            float blue = DAT_118e99f8[index].colours[i].z >= 1.0f ? 1.0f : DAT_118e99f8[index].colours[i].z;
            float fadedAlpha = DAT_118e99f8[index].colours[i].w * DAT_118e99f8[index].fade;
            float alpha = fadedAlpha >= 1.0f ? 1.0f : fadedAlpha;
            vertex->diffuse = (((int)(alpha * TRAIL_CHANNEL_SCALE) << 8 | (int)(red * TRAIL_CHANNEL_SCALE)) << 8 |
                               (int)(green * TRAIL_CHANNEL_SCALE)) << 8 | (int)(blue * TRAIL_CHANNEL_SCALE);
            float specularBlue = DAT_118e99f8[index].specular[i].z >= 1.0f ? 1.0f : DAT_118e99f8[index].specular[i].z;
            float specularGreen = DAT_118e99f8[index].specular[i].y >= 1.0f ? 1.0f : DAT_118e99f8[index].specular[i].y;
            float specularRed = DAT_118e99f8[index].specular[i].x >= 1.0f ? 1.0f : DAT_118e99f8[index].specular[i].x;
            vertex->specular = ((int)(specularBlue * TRAIL_CHANNEL_SCALE) << 8 |
                                (int)(specularGreen * TRAIL_CHANNEL_SCALE)) << 8 |
                               (int)(specularRed * TRAIL_CHANNEL_SCALE);
            if (x < -TRAIL_HIDE_DISTANCE || x > TRAIL_HIDE_DISTANCE)
                vertex->alpha = 0;
            if (y < -TRAIL_HIDE_DISTANCE || y > TRAIL_HIDE_DISTANCE)
                vertex->alpha = 0;
            if (*depth < TRAIL_NEAR_DEPTH)
                vertex->alpha = 0;
            if (*depth > TRAIL_FAR_DEPTH || *depth < TRAIL_NEAR_DEPTH)
                vertex->alpha = 0;
            vertexCount++;
            ScreenVertex *other = DAT_110187c0 + vertexCount;
            float x2 = (*centre - side.x) * TRAIL_SCREEN_WIDTH / 2.0f + 320.0f;
            float y2 = 240.0f - (projected[segment & 1].y - side.y) * TRAIL_SCREEN_HEIGHT / 2.0f;
            other->x = x2;
            other->y = y2;
            other->z = *depth;
            other->rhw = 1.0f;
            other->u = 1.0f;
            other->v = 0.5f;
            if (i == first)
                other->v = 0.0f;
            else if (i == DAT_118e99f8[index].pointCount - 2)
                other->v = 1.0f;
            other->diffuse = vertex->diffuse;
            other->specular = vertex->specular;
            if (x2 < -TRAIL_HIDE_DISTANCE || x2 > TRAIL_HIDE_DISTANCE)
                other->alpha = 0;
            if (y2 < -TRAIL_HIDE_DISTANCE || y2 > TRAIL_HIDE_DISTANCE)
                other->alpha = 0;
            if (*depth < TRAIL_NEAR_DEPTH)
                other->alpha = 0;
            if (*depth > TRAIL_FAR_DEPTH || *depth < TRAIL_NEAR_DEPTH)
                other->alpha = 0;
            vertexCount++;
            segment++;
        }
        int blend = DAT_1101dfdc[type].blend;
        if (blend == 0)
            FUN_1013f070(RENDER_BLEND_MODE, 0, 0);
        else if (blend == 1)
            FUN_1013f070(RENDER_BLEND_MODE, 1, 0);
        else if (blend == 2)
            FUN_1013f070(RENDER_BLEND_MODE, 2, 0);
        else if (blend == 3)
            FUN_1013f070(RENDER_BLEND_MODE, 10, 0);
        else if (blend == 4)
            FUN_1013f070(RENDER_BLEND_MODE, 11, 0);
        else if (blend == 5)
            FUN_1013f070(RENDER_BLEND_MODE, 12, 0);
        else if (blend == 6)
            FUN_1013f070(RENDER_BLEND_MODE, 13, 0);
    }
    if (vertexCount != 0)
        DAT_118d0ba8[DAT_1101dfdc[type].texture].DrawStrip(DAT_110187c0, vertexCount);
    return 0;
}
