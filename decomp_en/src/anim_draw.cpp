// Drawing a model (EN build). Ported from the matched JP source decomp/src/anim_model_draw.cpp.
#include "anim_model.h"
#include "frame_pacer.h"
#include "map_view.h"
#include "node.h"
#include "plane.h"
#include "render_command.h"

// One channel of the wobble: the ramp entry at `index` (wrapping), scaled to 0..1.
static inline float WobbleValue(const float *ramp, unsigned int index)
{
    return ramp[index % WOBBLE_RAMP_LENGTH] / WOBBLE_RAMP_PEAK;
}

// AnimModel::Draw: queues the model's node tree under `viewProjection` with the light set `lights`, coloured by the
// model's scale, shadow opacity and alpha. `flags` = DRAW_NORMAL draws it as it stands (and, for the wobbling root,
// first copies the colours the wobble animates), DRAW_OVERHEAD draws it flipped about z below the floor, any other
// value is a pointer to the Matrix4 of a mirror whose plane (through the matrix's position, facing along its z axis)
// the model is reflected in. Returns 1 when the model is hidden or queued, 0 when its shadow opacity or alpha is zero
// (nothing is drawn). Called for every character slot by Characters_Draw.
int AnimModel::Draw(Matrix4 *viewProjection, const void *lights, int flags)
{
    if (loadedRoot == 0 || shadowHidden != 0)
        return 1;
    if (shadowOpacity != 0)
    {
        Matrix4 flip;
        Matrix4 flipView;
        flip = IdentityMatrix();
        if (alpha <= 0.0f)
            return 0;
        Vec4 colour;
        float opacity = shadowOpacity;
        colour.w = opacity / SHADOW_OPACITY_FULL * alpha;
        colour.x = scale.x;
        colour.y = scale.y;
        colour.z = scale.z;
        if (flags == DRAW_NORMAL)
        {
            g_wobbleAmbient.Copy(m_024c);
            if (g_wobbleOn != 0 && loadedRoot != 0 && loadedRoot->drawMode == WOBBLE_DRAW_MODE)
            {
                float ramp[WOBBLE_RAMP_LENGTH] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f,
                                                  9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f,
                                                  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
                float one = 1.0f;
                g_wobbleAmbient.Fill(one);
                for (int i = 0; i < WOBBLE_COLOUR_COUNT; i++)
                    g_wobbleColours[i].Copy(m_00b4[i]);
                g_wobbleColours[0].Copy(g_wobbleColours[0] + m_0174[0]);
                g_wobbleColours[2].Copy(g_wobbleColours[2] + m_0174[1]);
                g_wobbleColours[4].Copy(g_wobbleColours[4] + m_0174[2]);
                g_wobbleColours[6].Copy(g_wobbleColours[6] + m_0174[3]);
                g_wobbleColours[8].Copy(g_wobbleColours[8] + m_0174[4]);
                g_wobbleColours[1].x = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_X);
                g_wobbleColours[1].y = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_Y);
                g_wobbleColours[1].z = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_Z);
                g_wobbleColours[3].x = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_X + WOBBLE_NEXT_COLOUR);
                g_wobbleColours[3].y = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_Y + WOBBLE_NEXT_COLOUR);
                g_wobbleColours[3].z = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_Z + WOBBLE_NEXT_COLOUR);
                g_wobbleColours[5].x = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_X + 2 * WOBBLE_NEXT_COLOUR);
                g_wobbleColours[5].y = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_Y + 2 * WOBBLE_NEXT_COLOUR);
                g_wobbleColours[5].z = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_Z + 2 * WOBBLE_NEXT_COLOUR);
                g_wobbleColours[7].x = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_X + 3 * WOBBLE_NEXT_COLOUR);
                g_wobbleColours[7].y = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_Y + 3 * WOBBLE_NEXT_COLOUR);
                g_wobbleColours[7].z = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_Z + 3 * WOBBLE_NEXT_COLOUR);
                g_wobbleColours[9].x = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_X + 4 * WOBBLE_NEXT_COLOUR);
                g_wobbleColours[9].y = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_Y + 4 * WOBBLE_NEXT_COLOUR);
                g_wobbleColours[9].z = WobbleValue(ramp, m_025c + ((unsigned int)g_engineTickCount >> 1) + WOBBLE_PHASE_Z + 4 * WOBBLE_NEXT_COLOUR);
            }
            Render_Command(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            root->QueueDraw(&matrix, viewProjection, lights, -1, &colour, &m_022c);
        }
        flip.m[MATRIX_SCALE_Z] = -1.0f;
        if (flags == DRAW_OVERHEAD)
        {
            g_mapPassRunning = 1;
            flip.m[MATRIX_TRANSLATION_Z] = shadowMatrix.m[MATRIX_TRANSLATION_Z] * OVERHEAD_FLIP_SCALE;
            flipView = flip * *viewProjection;
            Render_Command(RENDER_CULL_MODE, CULL_DEFAULT, 0);
            root->QueueDraw(&matrix, &flipView, lights, -1, 0, 0);
            g_mapPassRunning = 0;
            Render_Command(RENDER_CULL_MODE, CULL_DEFAULT, 0);
        }
        else if (flags != DRAW_NORMAL)
        {
            g_mapPassRunning = 1;
            Matrix4 mirrorInverse;
            Matrix4 mirrorTurn;
            Matrix4 toMirror;
            mirrorInverse = *(const Matrix4 *)flags;
            Vec4 normal;
            Vec4 mirrorPosition;
            Vec4 shift;
            mirrorPosition.Copy(mirrorInverse.GetTranslation());
            normal.Set(0.0f, 0.0f, 0.0f, 0.0f);
            mirrorInverse.SetTranslation(normal);
            normal.Set(0.0f, 0.0f, 1.0f, 0.0f);
            normal.Copy(Project(normal, mirrorInverse));
            Plane plane;
            plane.a = normal.x;
            plane.b = normal.y;
            plane.c = normal.z;
            plane.d = -(mirrorPosition.z * normal.z + mirrorPosition.y * normal.y + mirrorPosition.x * normal.x);
            Vec4 position;
            position.Copy(matrix.GetTranslation());
            plane.IntersectRay(&normal, &position, &normal);
            float offsetScale = MIRROR_OFFSET_SCALE;
            shift.Copy((normal - matrix.GetTranslation()) * offsetScale);
            mirrorTurn = mirrorInverse;
            mirrorInverse = InverseMatrix(mirrorInverse);
            Matrix4 reflect;
            Matrix4 lift;
            lift.SetIdentity();
            lift.SetTranslation(shift);
            reflect = lift * matrix * mirrorInverse * flip * mirrorTurn;
            lift.SetIdentity();
            toMirror.SetIdentity();
            toMirror.m[MATRIX_TRANSLATION_Z] = shadowMatrix.m[MATRIX_TRANSLATION_Z] * MIRROR_SHADOW_SCALE;
            lift.m[MATRIX_TRANSLATION_Z] = shadowMatrix.m[MATRIX_TRANSLATION_Z];
            flipView = toMirror * reflect * lift * *viewProjection;
            Render_Command(RENDER_CULL_MODE, CULL_DEFAULT, 0);
            root->QueueDraw(&matrix, &flipView, lights, -1, &colour, &m_022c);
            g_mapPassRunning = 0;
            Render_Command(RENDER_CULL_MODE, CULL_DEFAULT, 0);
        }
        return 1;
    }
    return 0;
}
