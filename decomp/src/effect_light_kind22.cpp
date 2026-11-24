// Drawing the ribbon of an effect light of kind 0x16: two crossed strips (an X seen from the front) that follow the
// trail and spin around the direction of travel, like a twisting streamer. Each pass draws one strip.

#include "vec4.h"

#include "matrix4.h"

#include "texture_object.h"

#include "map_vars.h"

#include "render_state.h"

#include "effect_manager.h"

#include "effect_trail.h"

// Direction of travel -> the matrix that turns it into the strip's frame (0x101b1f30, built from two rotations).
Matrix4 DirectionMatrix(Vec4 direction);

// The two corners of a strip start on a diagonal (+-0.7, +-0.7) in the plane facing the camera.
#define STREAMER_CORNER 0.7f
// The corners are turned this far per trail point before the first segment, and by STREAMER_TWIST_STEP per segment.
#define STREAMER_TWIST_PER_POINT 0.1
#define STREAMER_TWIST_STEP 0.1f
// The strip fades in over this many segments from the first point and out over the last STREAMER_FADE_OUT_START.
#define STREAMER_FADE_IN_SEGMENTS 4.0f
#define STREAMER_FADE_OUT_SEGMENTS 10.0f
#define STREAMER_FADE_OUT_START 9
// The two strips of the X.
#define STREAMER_PASS_COUNT 2

// 0x1016fb80 (EffectManager::DrawLightKind22). Draws light `index` of kind 0x16 (called by DrawLight). Like the
// plain trail it starts at the first point still alive, hides itself when that point is off screen and needs 3
// points. Each segment makes two vertices: the trail point plus a corner offset, one corner on each side. The
// offsets are the diagonal turned into the direction of travel and scaled by the point's width; they turn a little
// more with every point, and the second pass uses the mirrored diagonal so the two strips cross. The colour fades
// in over the first four segments and out over the last nine and is taken from the point colours as in DrawLight.
// Returns 0.
int EffectManager::DrawLightKind22(int index, const Matrix4 &view, const Matrix4 *camera)
{
    int type = DAT_118e99f8[index].type;
    if (DAT_1101dfdc[type].depthTest != 0)
        FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);
    else
        FUN_1013f070(RENDER_DEPTH_WRITE, 0, 0);
    for (int pass = 0; pass < STREAMER_PASS_COUNT; pass++)
    {
        if (DAT_118e99f8[index].pointCount < 3)
            return 0;
        Vec4 points[2];
        int segment = 0;
        Vec4 left;
        Vec4 corners[4];
        int i = (DAT_118e99f8[index].frame - DAT_1101dfdc[type].lightFrames) + DAT_118e99f8[index].pointCount;
        i = i > 0 ? i : 0;
        if (m_0050 != 0)
        {
            if (!(DAT_10f668a8.min.x - TRAIL_CULL_MARGIN < DAT_118e99f8[index].points[i].x && DAT_10f668a8.min.y - TRAIL_CULL_MARGIN < DAT_118e99f8[index].points[i].y &&
                  DAT_10f668a8.min.z - TRAIL_CULL_MARGIN_NEAR < DAT_118e99f8[index].points[i].z && DAT_118e99f8[index].points[i].x < DAT_10f668a8.max.x + TRAIL_CULL_MARGIN &&
                  DAT_118e99f8[index].points[i].y < DAT_10f668a8.max.y + TRAIL_CULL_MARGIN && DAT_118e99f8[index].points[i].z < DAT_10f668a8.max.z + TRAIL_CULL_MARGIN_FAR))
                return 0;
        }
        int first = i;
        points[0].Copy(DAT_118e99f8[index].points[i]);
        int vertexCount = 0;
        Vec4 scratch;
        Vec4 right;
        scratch.Set(1.0f, 1.0f, 1.0f, 1.0f);
        Matrix4 turn;
        if (pass == 0)
        {
            corners[0].Set(-STREAMER_CORNER, -STREAMER_CORNER, 0.0f, 0.0f);
            corners[1].Set(STREAMER_CORNER, STREAMER_CORNER, 0.0f, 0.0f);
        }
        else
        {
            corners[0].Set(STREAMER_CORNER, -STREAMER_CORNER, 0.0f, 0.0f);
            corners[1].Set(-STREAMER_CORNER, STREAMER_CORNER, 0.0f, 0.0f);
        }
        float twist = (float)((first - 1) * STREAMER_TWIST_PER_POINT);
        corners[0].RotateZ(twist);
        corners[1].RotateZ(twist);
        for (; i < DAT_118e99f8[index].pointCount - 1; i++)
        {
            corners[0].RotateZ(STREAMER_TWIST_STEP);
            corners[1].RotateZ(STREAMER_TWIST_STEP);
            points[(segment - 1) & 1].Copy(DAT_118e99f8[index].points[i + 1]);
            left.Copy(points[(segment - 1) & 1] - points[segment & 1]);
            turn = DirectionMatrix(left);
            left.Copy(Project(corners[0], turn));
            left.Copy(left * DAT_118e99f8[index].widths[i].x);
            left.Copy(points[segment & 1] + left);
            right.Copy(Project(corners[1], turn));
            right.Copy(right * DAT_118e99f8[index].widths[i].x);
            right.Copy(points[segment & 1] + right);
            scratch.Copy(right);
            left.Copy(Project(left, view));
            right.Copy(Project(right, view));
            ScreenVertex *vertex = DAT_110187c0 + vertexCount;
            float x = left.x * TRAIL_SCREEN_WIDTH / 2.0f + 320.0f;
            float y = 240.0f - left.y * TRAIL_SCREEN_HEIGHT / 2.0f;
            vertex->z = left.z;
            vertex->rhw = 1.0f;
            vertex->u = 0.0f;
            vertex->v = 0.5f;
            vertex->x = x;
            vertex->y = y;
            if (i == first)
                vertex->v = 0.0f;
            else if (i == DAT_118e99f8[index].pointCount - 2)
                vertex->v = 1.0f;
            float fade = 1.0f;
            if (i < first + 4)
                fade = (float)segment / STREAMER_FADE_IN_SEGMENTS;
            int remaining = (DAT_118e99f8[index].pointCount - i) - 2;
            if (remaining < STREAMER_FADE_OUT_START)
                fade = (float)remaining / STREAMER_FADE_OUT_SEGMENTS;
            float fadedRed = fade * DAT_118e99f8[index].colours[i].x;
            float red = fadedRed >= 1.0f ? 1.0f : fadedRed;
            float fadedGreen = fade * DAT_118e99f8[index].colours[i].y;
            float green = fadedGreen >= 1.0f ? 1.0f : fadedGreen;
            float fadedBlue = fade * DAT_118e99f8[index].colours[i].z;
            float blue = fadedBlue >= 1.0f ? 1.0f : fadedBlue;
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
            if (left.z < TRAIL_NEAR_DEPTH)
                vertex->alpha = 0;
            if (left.z > TRAIL_FAR_DEPTH || left.z < TRAIL_NEAR_DEPTH)
                vertex->alpha = 0;
            vertexCount++;
            ScreenVertex *other = DAT_110187c0 + vertexCount;
            other->z = right.z;
            other->rhw = 1.0f;
            other->u = 1.0f;
            other->v = 0.5f;
            float x2 = right.x * TRAIL_SCREEN_WIDTH / 2.0f + 320.0f;
            other->x = x2;
            float y2 = 240.0f - right.y * TRAIL_SCREEN_HEIGHT / 2.0f;
            other->y = y2;
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
            if (right.z < TRAIL_NEAR_DEPTH)
                other->alpha = 0;
            if (right.z > TRAIL_FAR_DEPTH || right.z < TRAIL_NEAR_DEPTH)
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
        if (vertexCount != 0)
            DAT_118d0ba8[DAT_1101dfdc[type].texture].DrawStrip(DAT_110187c0, vertexCount);
    }
    return 0;
}
