// Drawing the trail of effect lights with a width that follows the depth (the thick weapon streaks): like the
// plain trail of effect_light.cpp, but each point's width is also scaled by how close it is to the camera.

#include "vec4.h"

#include "matrix4.h"

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);   // 0x101ae060: to -1..1 screen space

#include "texture_object.h"

#include "key_track.h"

#include "map_vars.h"

#include "render_state.h"

#include "effect_manager.h"

#include "effect_trail.h"

// Trail width factor at the camera (depth 0) for the two kinds, and how it falls towards the far plane.
#define TRAIL_DEPTH_WIDTH_KIND7 8.0f
#define TRAIL_DEPTH_WIDTH_KIND23 10.0f
#define TRAIL_NEAR_SCALE_SLOPE 0.99f
#define TRAIL_NEAR_SCALE_BASE 0.01f
// Kind 0x17 shows the last 13 points regardless of the definition.
#define TRAIL_KIND23_FRAMES 13

// 0x101713f0 (EffectManager::DrawLightKind7). Draws light `index` of kinds 7 and 9 (called by DrawLight).
int EffectManager::DrawLightKind7(int index, const Matrix4 &view, const Matrix4 *camera)
{
    int type = DAT_118e99f8[index].type;
    if (DAT_1101dfdc[type].depthTest != 0)
        FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
    else
        FUN_1013f070(RENDER_DEPTH_WRITE, 0, 0);
    int drawCount;
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
        int vertexCount = 0;
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
            float red = 1.0f > DAT_118e99f8[index].colours[sampleIndex].x ? DAT_118e99f8[index].colours[sampleIndex].x : 1.0f;
            float green = 1.0f > DAT_118e99f8[index].colours[sampleIndex].y ? DAT_118e99f8[index].colours[sampleIndex].y : 1.0f;
            float blue = 1.0f > DAT_118e99f8[index].colours[sampleIndex].z ? DAT_118e99f8[index].colours[sampleIndex].z : 1.0f;
            float fadedAlpha = DAT_118e99f8[index].colours[sampleIndex].w * DAT_118e99f8[index].fade;
            float alpha = 1.0f > fadedAlpha ? fadedAlpha : 1.0f;
            vertex->diffuse = (((int)(alpha * TRAIL_CHANNEL_SCALE) << 8 | (int)(red * TRAIL_CHANNEL_SCALE)) << 8 |
                               (int)(green * TRAIL_CHANNEL_SCALE)) << 8 | (int)(blue * TRAIL_CHANNEL_SCALE);
            float specularBlue = 1.0f > DAT_118e99f8[index].specular[point].z ? DAT_118e99f8[index].specular[point].z : 1.0f;
            float specularGreen = 1.0f > DAT_118e99f8[index].specular[sampleIndex].y ? DAT_118e99f8[index].specular[sampleIndex].y : 1.0f;
            float specularRed = 1.0f > DAT_118e99f8[index].specular[sampleIndex].x ? DAT_118e99f8[index].specular[sampleIndex].x : 1.0f;
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
        drawCount = vertexCount;
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
        int vertexCount = 0;
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
            float nearness = 1.0f - projected[(segment - 1) & 1].z;
            float widen = TRAIL_DEPTH_WIDTH_KIND7;
            side.Copy(side * nearness * widen);
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
            float red = 1.0f > DAT_118e99f8[index].colours[i].x ? DAT_118e99f8[index].colours[i].x : 1.0f;
            float green = 1.0f > DAT_118e99f8[index].colours[i].y ? DAT_118e99f8[index].colours[i].y : 1.0f;
            float blue = 1.0f > DAT_118e99f8[index].colours[i].z ? DAT_118e99f8[index].colours[i].z : 1.0f;
            float fadedAlpha = DAT_118e99f8[index].colours[i].w * DAT_118e99f8[index].fade;
            float alpha = 1.0f > fadedAlpha ? fadedAlpha : 1.0f;
            vertex->diffuse = (((int)(alpha * TRAIL_CHANNEL_SCALE) << 8 | (int)(red * TRAIL_CHANNEL_SCALE)) << 8 |
                               (int)(green * TRAIL_CHANNEL_SCALE)) << 8 | (int)(blue * TRAIL_CHANNEL_SCALE);
            float specularBlue = 1.0f > DAT_118e99f8[index].specular[i].z ? DAT_118e99f8[index].specular[i].z : 1.0f;
            float specularGreen = 1.0f > DAT_118e99f8[index].specular[i].y ? DAT_118e99f8[index].specular[i].y : 1.0f;
            float specularRed = 1.0f > DAT_118e99f8[index].specular[i].x ? DAT_118e99f8[index].specular[i].x : 1.0f;
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
        drawCount = vertexCount;
    }
    if (drawCount != 0)
        DAT_118d0ba8[DAT_1101dfdc[type].texture].DrawStrip(DAT_110187c0, drawCount);
    return 0;
}

// 0x10170580 (EffectManager::DrawLightKind23). Draws light `index` of kind 0x17 (called by DrawLight).
int EffectManager::DrawLightKind23(int index, const Matrix4 &view, const Matrix4 *camera)
{
    int type = DAT_118e99f8[index].type;
    if (DAT_1101dfdc[type].depthTest != 0)
        FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
    else
        FUN_1013f070(RENDER_DEPTH_WRITE, 0, 0);
    int drawCount;
    if (DAT_1101dfdc[type].kind == EFFECT_KIND_LIGHT_INTERLEAVED)
    {
        if (DAT_118e99f8[index].pointCount < 3)
            return 0;
        Vec4 projected[2];
        Vec4 screen;
        int first = DAT_118e99f8[index].frame - TRAIL_KIND23_FRAMES + DAT_118e99f8[index].pointCount;
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
        int vertexCount = 0;
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
            float red = 1.0f > DAT_118e99f8[index].colours[sampleIndex].x ? DAT_118e99f8[index].colours[sampleIndex].x : 1.0f;
            float green = 1.0f > DAT_118e99f8[index].colours[sampleIndex].y ? DAT_118e99f8[index].colours[sampleIndex].y : 1.0f;
            float blue = 1.0f > DAT_118e99f8[index].colours[sampleIndex].z ? DAT_118e99f8[index].colours[sampleIndex].z : 1.0f;
            float fadedAlpha = DAT_118e99f8[index].colours[sampleIndex].w * DAT_118e99f8[index].fade;
            float alpha = 1.0f > fadedAlpha ? fadedAlpha : 1.0f;
            vertex->diffuse = (((int)(alpha * TRAIL_CHANNEL_SCALE) << 8 | (int)(red * TRAIL_CHANNEL_SCALE)) << 8 |
                               (int)(green * TRAIL_CHANNEL_SCALE)) << 8 | (int)(blue * TRAIL_CHANNEL_SCALE);
            float specularBlue = 1.0f > DAT_118e99f8[index].specular[point].z ? DAT_118e99f8[index].specular[point].z : 1.0f;
            float specularGreen = 1.0f > DAT_118e99f8[index].specular[sampleIndex].y ? DAT_118e99f8[index].specular[sampleIndex].y : 1.0f;
            float specularRed = 1.0f > DAT_118e99f8[index].specular[sampleIndex].x ? DAT_118e99f8[index].specular[sampleIndex].x : 1.0f;
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
        drawCount = vertexCount;
    }
    else
    {
        if (DAT_118e99f8[index].pointCount < 3)
            return 0;
        Vec4 projected[2];
        Vec4 side;
        int first = DAT_118e99f8[index].frame - TRAIL_KIND23_FRAMES + DAT_118e99f8[index].pointCount;
        first = first > 0 ? first : 0;
        if (m_0050 != 0)
        {
            if (!(DAT_10f668a8.min.x - TRAIL_CULL_MARGIN < DAT_118e99f8[index].points[first].x && DAT_10f668a8.min.y - TRAIL_CULL_MARGIN < DAT_118e99f8[index].points[first].y &&
                  DAT_10f668a8.min.z - TRAIL_CULL_MARGIN_NEAR < DAT_118e99f8[index].points[first].z && DAT_118e99f8[index].points[first].x < DAT_10f668a8.max.x + TRAIL_CULL_MARGIN &&
                  DAT_118e99f8[index].points[first].y < DAT_10f668a8.max.y + TRAIL_CULL_MARGIN && DAT_118e99f8[index].points[first].z < DAT_10f668a8.max.z + TRAIL_CULL_MARGIN_FAR))
                return 0;
        }
        projected[0].Copy(Project(DAT_118e99f8[index].points[first], view));
        int vertexCount = 0;
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
            float nearness = (1.0f - projected[(segment - 1) & 1].z) * TRAIL_NEAR_SCALE_SLOPE + TRAIL_NEAR_SCALE_BASE;
            float widen = TRAIL_DEPTH_WIDTH_KIND23;
            side.Copy(side * nearness * widen);
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
            float red = 1.0f > DAT_118e99f8[index].colours[i].x ? DAT_118e99f8[index].colours[i].x : 1.0f;
            float green = 1.0f > DAT_118e99f8[index].colours[i].y ? DAT_118e99f8[index].colours[i].y : 1.0f;
            float blue = 1.0f > DAT_118e99f8[index].colours[i].z ? DAT_118e99f8[index].colours[i].z : 1.0f;
            float fadedAlpha = DAT_118e99f8[index].colours[i].w * DAT_118e99f8[index].fade;
            float alpha = 1.0f > fadedAlpha ? fadedAlpha : 1.0f;
            vertex->diffuse = (((int)(alpha * TRAIL_CHANNEL_SCALE) << 8 | (int)(red * TRAIL_CHANNEL_SCALE)) << 8 |
                               (int)(green * TRAIL_CHANNEL_SCALE)) << 8 | (int)(blue * TRAIL_CHANNEL_SCALE);
            float specularBlue = 1.0f > DAT_118e99f8[index].specular[i].z ? DAT_118e99f8[index].specular[i].z : 1.0f;
            float specularGreen = 1.0f > DAT_118e99f8[index].specular[i].y ? DAT_118e99f8[index].specular[i].y : 1.0f;
            float specularRed = 1.0f > DAT_118e99f8[index].specular[i].x ? DAT_118e99f8[index].specular[i].x : 1.0f;
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
        drawCount = vertexCount;
    }
    if (drawCount != 0)
        DAT_118d0ba8[DAT_1101dfdc[type].texture].DrawStrip(DAT_110187c0, drawCount);
    return 0;
}
