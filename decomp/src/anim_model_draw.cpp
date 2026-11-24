// AnimModel::Draw: queues a model's node tree for drawing, optionally mirrored (0x10163150).

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "plane.h"
#include "anim_model.h"
#include "anim_draw_vars.h"
#include "render_state.h"
#include "render_vars.h"

Matrix4 IdentityMatrix();                    // 0x101b0ba0
#include "matrix_transform.h"
Vec4 Project(const Vec4 &point, const Matrix4 &matrix);  // 0x101ae060

#define DRAW_NORMAL 0                  // Draw's `flags`: the plain draw
#define DRAW_OVERHEAD 1                // the model drawn flipped about z below the floor (the overhead pass)
                                       // any other value is a pointer to the matrix of a mirror
#define SHADOW_OPACITY_FULL 15.0f      // shadowOpacity of a placed model
#define WOBBLE_DRAW_MODE 0x24          // the root's drawMode that turns the wobble on
#define WOBBLE_RAMP_LENGTH 30          // entries of the triangle ramp the wobble steps through
#define WOBBLE_RAMP_PEAK 9.0f          // its highest value: dividing by it scales the ramp to 0..1
#define WOBBLE_COLOUR_COUNT 10         // m_00b4 colours copied to DAT_10f655c8
#define WOBBLE_PHASE_X 0               // phase shifts of the three channels of the first wobbling colour
#define WOBBLE_PHASE_Y 10
#define WOBBLE_PHASE_Z 20
#define WOBBLE_NEXT_COLOUR 6           // shift added for each following wobbling colour
#define OVERHEAD_SHIFT_SCALE (-2.0f)   // z shift of the shadow matrix in the mirror's frame
#define MIRROR_OFFSET_SCALE (-2.0f)    // how far the mirrored model moves from the foot of its perpendicular

// One channel of the wobble: the ramp entry at `index` (wrapping), scaled to 0..1.
static inline float WobbleValue(const float *ramp, unsigned int index)
{
    return ramp[index % WOBBLE_RAMP_LENGTH] / WOBBLE_RAMP_PEAK;
}

// 0x10163150 (AnimModel::Draw). Queues the model's node tree under `viewProjection` with the light set `lights`,
// coloured by the model's scale, shadow opacity and alpha. `flags` = DRAW_NORMAL draws it as it stands (and, for
// the wobbling root, first copies the colours the wobble animates), DRAW_OVERHEAD draws it flipped about z below
// the floor, any other value is a Matrix4 describing a mirror whose plane (through the matrix's position, facing
// along its z axis) the model is reflected in. Returns 1 when the
// model is hidden or queued, 0 when its shadow opacity or alpha is zero (nothing is drawn).
int AnimModel::Draw(const Matrix4 *viewProjection, const Vec4 *lights, int flags)
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
        colour.x = scale.x;
        colour.y = scale.y;
        colour.z = scale.z;
        colour.w = opacity / SHADOW_OPACITY_FULL * alpha;
        if (flags == DRAW_NORMAL)
        {
            DAT_10f65888.Copy(m_024c);
            if (DAT_10f6558c != 0 && loadedRoot != 0 && loadedRoot->drawMode == WOBBLE_DRAW_MODE)
            {
                float ramp[WOBBLE_RAMP_LENGTH] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f,
                                                  9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f,
                                                  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
                float one = 1.0f;
                DAT_10f65888.Fill(one);
                for (int i = 0; i < WOBBLE_COLOUR_COUNT; i++)
                    DAT_10f655c8[i].Copy(m_00b4[i]);
                DAT_10f655c8[0].Copy(DAT_10f655c8[0] + m_0174[0]);
                DAT_10f655c8[2].Copy(DAT_10f655c8[2] + m_0174[1]);
                DAT_10f655c8[4].Copy(DAT_10f655c8[4] + m_0174[2]);
                DAT_10f655c8[6].Copy(DAT_10f655c8[6] + m_0174[3]);
                DAT_10f655c8[8].Copy(DAT_10f655c8[8] + m_0174[4]);
                DAT_10f655c8[1].x = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_X);
                DAT_10f655c8[1].y = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_Y);
                DAT_10f655c8[1].z = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_Z);
                DAT_10f655c8[3].x = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_X + WOBBLE_NEXT_COLOUR);
                DAT_10f655c8[3].y = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_Y + WOBBLE_NEXT_COLOUR);
                DAT_10f655c8[3].z = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_Z + WOBBLE_NEXT_COLOUR);
                DAT_10f655c8[5].x = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_X + 2 * WOBBLE_NEXT_COLOUR);
                DAT_10f655c8[5].y = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_Y + 2 * WOBBLE_NEXT_COLOUR);
                DAT_10f655c8[5].z = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_Z + 2 * WOBBLE_NEXT_COLOUR);
                DAT_10f655c8[7].x = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_X + 3 * WOBBLE_NEXT_COLOUR);
                DAT_10f655c8[7].y = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_Y + 3 * WOBBLE_NEXT_COLOUR);
                DAT_10f655c8[7].z = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_Z + 3 * WOBBLE_NEXT_COLOUR);
                DAT_10f655c8[9].x = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_X + 4 * WOBBLE_NEXT_COLOUR);
                DAT_10f655c8[9].y = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_Y + 4 * WOBBLE_NEXT_COLOUR);
                DAT_10f655c8[9].z = WobbleValue(ramp, m_025c + ((unsigned int)DAT_10f65504 >> 1) + WOBBLE_PHASE_Z + 4 * WOBBLE_NEXT_COLOUR);
            }
            FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            root->QueueDraw(&matrix, viewProjection, lights, 0xffffffff, &colour, &m_022c);
        }
        flip._33 = -1.0f;
        if (flags == DRAW_OVERHEAD)
        {
            DAT_10f65520 = 1;
            flip._43 = shadowMatrix._43 + shadowMatrix._43;
            flipView = flip * *viewProjection;
            FUN_1013f070(RENDER_CULL_MODE, CULL_DEFAULT, 0);
            root->QueueDraw(&matrix, &flipView, lights, 0xffffffff, 0, 0);
            DAT_10f65520 = 0;
            FUN_1013f070(RENDER_CULL_MODE, CULL_DEFAULT, 0);
        }
        else if (flags != DRAW_NORMAL)
        {
            DAT_10f65520 = 1;
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
            toMirror._43 = shadowMatrix._43 * OVERHEAD_SHIFT_SCALE;
            lift._43 = shadowMatrix._43;
            flipView = toMirror * reflect * lift * *viewProjection;
            FUN_1013f070(RENDER_CULL_MODE, CULL_DEFAULT, 0);
            root->QueueDraw(&matrix, &flipView, lights, 0xffffffff, &colour, &m_022c);
            DAT_10f65520 = 0;
            FUN_1013f070(RENDER_CULL_MODE, CULL_DEFAULT, 0);
        }
        return 1;
    }
    return 0;
}
