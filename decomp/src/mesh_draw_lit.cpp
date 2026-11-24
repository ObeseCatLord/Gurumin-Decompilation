// Drawing a mesh with the lit shaders (MeshData::DrawLit): one entry point for every drawing mode. The mode picks the
// vertex shader and the constants that go with it (matrices, colours, light set, fog, the tables of the shadow and
// rainbow passes) and a set of texture stage states; the packs are then drawn range by range, with the ranges of
// packs that share a material merged and the ranges that cannot be seen left out. The passes that draw a mesh twice
// (outline, overlay, rainbow) call DrawLit again after the main draw.

#include "d3d9_min.h"

#include "vec4.h"

#include "matrix4.h"

#include "mesh.h"

#include "bounds.h"

#include "render_vars.h"

#include "render_state.h"

#include "camera_vars.h"

#include "map_vars.h"

#include "map_setup.h"

#include "field_update.h"

#include "scene_draw_vars.h"

#include "anim_draw_vars.h"

#include "mesh_shader.h"

#include "frame_vars.h"

#include "debris.h"

#include "texture_object.h"

#include "mesh_draw_vars.h"

#include "debug_shape.h"

#include "render_overlay.h"

#include "render_reset_vars.h"

#include "mesh_lit_vars.h"

#define D3DPT_TRIANGLELIST_ 4
#define VERTEX_STRIDE 0x70
#define TRIANGLE_VERTICES 3
#define BOX_VECTOR_COUNT 9
#define BOX_TO_VIEW 7
#define BOX_DIRECTION 8
#define VIEW_AHEAD_DISTANCE 800.0f
#define FACING_LIMIT 0.9f
#define AREA_COLOUR_SCALE 0.00055
#define AREA_COLOUR_VALUE (*(const float *)&DAT_10f655c4)   // DAT_10f655c4 is read as a float by the shader constants
#define RAINBOW_WOBBLE_SCALE 0.3f
#define RAINBOW_WOBBLE_OFFSET 0.7f
#define FIRST_TEXTURE_OBJECT_VALUE 10   // DAT_10f6550c below this is none/unknown (TEXTURE_BINDING_UNKNOWN)
#define DEVICE_STATE_OVERHEAD_VARIANT 8   // FUN_1013ef30 state set to 2 while the packs draw and back to 3 afterwards

// Drawing modes that draw the mesh a second time.
#define MODE_OUTLINE_PASS 0x26
#define MODE_OUTLINE_PASS_ALT 0x28
#define MODE_OVERLAY 5
#define MODE_RAINBOW_BASE 0x22
#define MODE_RAINBOW_COPY 0x23
#define MODE_RAINBOW_WOBBLE_BASE 0x24
#define MODE_RAINBOW_WOBBLE_COPY 0x25

// Texture stage state types of FUN_1013ef90.
#define STAGE_COLOUR_OP 1
#define STAGE_COLOUR_ARG1 2
#define STAGE_COLOUR_ARG2 3
#define STAGE_ALPHA_OP 4
#define STAGE_ALPHA_ARG1 5
#define STAGE_ALPHA_ARG2 6

// Bits of a pack's edge word (MeshData::doubleSide).
#define EDGE_CULL_NONE 0x1
#define EDGE_ADDITIVE 0x2
#define EDGE_MULTIPLY 0x4
#define EDGE_BLEND_MASK 0x6
#define EDGE_EXTRA_BLEND_MASK 0x1c00
#define EDGE_BLEND_BIT_A 0x400
#define EDGE_BLEND_BIT_B 0x800
#define EDGE_BLEND_BIT_C 0x1000
#define EDGE_ALPHA_REFERENCE_HIGH 0x100
#define EDGE_POINT_FILTER 0x200
#define EDGE_ALPHA_TEST_OFF 0x2000

#define SET_VECTOR(reg, a, b, c, d)                                                       \
    {                                                                                     \
        float constant[4] = {a, b, c, d};                                                 \
        DAT_10f1fcf0->SetVertexShaderConstantF(reg, constant, 1);                         \
    }

// A colour constant: the given default, or the colour the caller passed when there is one.
#define SET_VECTOR_OR(reg, pointer, a, b, c, d)                                           \
    {                                                                                     \
        float constant[4] = {a, b, c, d};                                                 \
        if (pointer)                                                                      \
        {                                                                                 \
            constant[0] = (pointer)->x;                                                   \
            constant[1] = (pointer)->y;                                                   \
            constant[2] = (pointer)->z;                                                   \
            constant[3] = (pointer)->w;                                                   \
        }                                                                                 \
        DAT_10f1fcf0->SetVertexShaderConstantF(reg, constant, 1);                         \
    }

// A constant with defaults (a..d) that is replaced by (e..h) when `condition` holds.
#define SET_VECTOR_IF(reg, condition, a, b, c, d, e, f, g, h)                             \
    {                                                                                     \
        float constant[4] = {a, b, c, d};                                                 \
        if (condition)                                                                    \
        {                                                                                 \
            constant[0] = e;                                                              \
            constant[1] = f;                                                              \
            constant[2] = g;                                                              \
            constant[3] = h;                                                              \
        }                                                                                 \
        DAT_10f1fcf0->SetVertexShaderConstantF(reg, constant, 1);                         \
    }

#define SET_VECTORS(reg, source, count) \
    DAT_10f1fcf0->SetVertexShaderConstantF(reg, (const float *)(source), count)

#define SET_MATRIX(reg, rows, matrix)                                                     \
    transform = matrix;                                                                   \
    DAT_10f1fcf0->SetVertexShaderConstantF(reg, (const float *)&transform, rows)

// The four light boxes and the reciprocals of the squares of their counts.
#define SET_BOUNDS_CONSTANTS(bounds, counts)                                                              \
    SET_VECTOR(0x56, bounds[0].x, bounds[0].y, bounds[0].z, bounds[0].w);                                 \
    SET_VECTOR(0x57, bounds[1].x, bounds[1].y, bounds[1].z, bounds[1].w);                                 \
    SET_VECTOR(0x58, bounds[2].x, bounds[2].y, bounds[2].z, bounds[2].w);                                 \
    SET_VECTOR(0x59, bounds[3].x, bounds[3].y, bounds[3].z, bounds[3].w);                                 \
    SET_VECTOR(0x55, 1.0f / (counts[0] * counts[0]), 1.0f / (counts[1] * counts[1]),                      \
               1.0f / (counts[2] * counts[2]), 1.0f / (counts[3] * counts[3]))

// The scene's fog colour (components clamped to 1) as the render fog colour.
#define SET_SCENE_FOG()                                                                                   \
    {                                                                                                     \
        float red = DAT_10f668f8.x < 1.0f ? DAT_10f668f8.x : 1.0f;                                        \
        float green = DAT_10f668f8.y < 1.0f ? DAT_10f668f8.y : 1.0f;                                      \
        float blue = DAT_10f668f8.z < 1.0f ? DAT_10f668f8.z : 1.0f;                                       \
        FUN_1013f070(RENDER_FOG, 1,                                                                       \
                     (((0xffffff00 | (int)(red * 255.0f)) << 8 | (int)(green * 255.0f)) << 8) |           \
                         (int)(blue * 255.0f));                                                           \
    }

// Dot product of a box direction and the view direction, summed in the order the original does.
#define FACING_DOT(direction, view) ((direction).x * (view).x + (direction).z * (view).z + (view).y * (direction).y)

// Binds the textures of pack `pack` (sets of stage states per drawing mode). `merged` is 1 for a merged group (the
// first path of the pack loop) and 0 for a single pack; `unboundAlphaOp` is the alpha operation of the second
// stage when the pack has no second texture in the overlay mode.
#define BIND_PACK_TEXTURES(merged, unboundAlphaOp)                                                         \
    do                                                                                                    \
    {                                                                                                     \
        if (DAT_10f65580 != 0)                                                                            \
        {                                                                                                 \
            if (!(merged) && (doubleSide[pack] & EDGE_BLEND_MASK) != 0)                                   \
                goto nextPack;                                                                            \
            if (DAT_118d0ba8[materials[pack]].hasOwnAlpha == 0)                                           \
            {                                                                                             \
                FUN_10143b50();                                                                           \
                break;                                                                                    \
            }                                                                                             \
        }                                                                                                 \
        if (mode == 0xc)                                                                                  \
        {                                                                                                 \
            if (DAT_10203418 == 0)                                                                        \
            {                                                                                             \
                FUN_10143b50();                                                                           \
                break;                                                                                    \
            }                                                                                             \
            if (DAT_10f6548c != 0)                                                                        \
                goto nextPack;                                                                            \
            FUN_10161070(DAT_10f6559c);                                                                   \
            break;                                                                                        \
        }                                                                                                 \
        if (shaderId == 0x24 || shaderId == 0x28 || shaderId == 0x3c)                                     \
        {                                                                                                 \
            FUN_10143b50();                                                                               \
            if (DAT_10f6548c != 0)                                                                        \
                goto nextPack;                                                                            \
            FUN_10161070(DAT_10f655a0);                                                                   \
            if (DAT_10202b1c == 0)                                                                        \
                doubleSide[pack] |= EDGE_ADDITIVE;                                                        \
            break;                                                                                        \
        }                                                                                                 \
        if (mode == MODE_RAINBOW_WOBBLE_COPY || mode == MODE_RAINBOW_COPY)                                \
        {                                                                                                 \
            if (secondaryTextures[pack] >= 0)                                                             \
                FUN_10161070(secondaryTextures[pack]);                                                    \
            break;                                                                                        \
        }                                                                                                 \
        if (shaderId == 0x1d)                                                                             \
        {                                                                                                 \
            FUN_10161070(DAT_10f655ac);                                                                   \
            break;                                                                                        \
        }                                                                                                 \
        if (shaderId == 0x3d || shaderId == 0x1e || shaderId == 0x20 || shaderId == 0x2b)                 \
        {                                                                                                 \
            FUN_10161070(materials[pack]);                                                                \
            FUN_101610b0((mode == MODE_RAINBOW_BASE || mode == MODE_RAINBOW_WOBBLE_BASE) ? DAT_10f655b0   \
                                                                                           : DAT_10f655ac); \
            if (materials[pack] < 1)                                                                      \
            {                                                                                             \
                if (!(merged))                                                                            \
                    FUN_10143b50();                                                                       \
                FUN_1013ef90(0, STAGE_ALPHA_OP, 2);                                                       \
                FUN_1013ef90(0, STAGE_ALPHA_ARG1, 0);                                                     \
                FUN_1013ef90(0, STAGE_ALPHA_ARG2, 2);                                                     \
                FUN_1013ef90(0, STAGE_COLOUR_OP, 2);                                                      \
                FUN_1013ef90(0, STAGE_COLOUR_ARG1, 0);                                                    \
                FUN_1013ef90(0, STAGE_COLOUR_ARG2, 2);                                                    \
            }                                                                                             \
            else if (!(merged))                                                                           \
            {                                                                                             \
                FUN_1013ef90(0, STAGE_ALPHA_OP, 4);                                                       \
                FUN_1013ef90(0, STAGE_ALPHA_ARG1, 0);                                                     \
                FUN_1013ef90(0, STAGE_ALPHA_ARG2, 2);                                                     \
                FUN_1013ef90(0, STAGE_COLOUR_OP, 4);                                                      \
                FUN_1013ef90(0, STAGE_COLOUR_ARG1, 0);                                                    \
                FUN_1013ef90(0, STAGE_COLOUR_ARG2, 2);                                                    \
            }                                                                                             \
            break;                                                                                        \
        }                                                                                                 \
        if (mode == 6 || mode == 0x1c || mode == 0x1d)                                                    \
        {                                                                                                 \
            FUN_10161070(secondaryTextures[pack] >= 0 ? secondaryTextures[pack] : materials[pack]);       \
            if (secondaryTextures[pack] <= 0)                                                             \
            {                                                                                             \
                FUN_1013ef90(1, STAGE_COLOUR_OP, 1);                                                      \
                FUN_1013ef90(1, STAGE_ALPHA_OP, 1);                                                       \
            }                                                                                             \
            break;                                                                                        \
        }                                                                                                 \
        if (mode == 0x3b)                                                                                 \
        {                                                                                                 \
            FUN_10161070(materials[pack]);                                                                \
            FUN_1013ef90(0, STAGE_COLOUR_OP, 2);                                                          \
            FUN_1013ef90(0, STAGE_ALPHA_OP, 3);                                                           \
            FUN_1013ef90(0, STAGE_ALPHA_ARG1, 0);                                                         \
            FUN_1013ef90(0, STAGE_ALPHA_ARG2, 2);                                                         \
            FUN_1013ef90(0, STAGE_COLOUR_ARG1, 0);                                                        \
            FUN_1013ef90(0, STAGE_COLOUR_ARG2, 2);                                                        \
            break;                                                                                        \
        }                                                                                                 \
        if (!(mode == 0x10 && materials[pack] >= 0 && DAT_118d0ba8[materials[pack]].hasOwnAlpha == 0 &&   \
              (int)DAT_10f6550c >= FIRST_TEXTURE_OBJECT_VALUE && DAT_10f6550c->hasOwnAlpha == 0))         \
            FUN_10161070(materials[pack]);                                                                \
        FUN_101610b0(secondaryTextures[pack]);                                                            \
        if (mode == MODE_OVERLAY)                                                                         \
            FUN_1013ef90(0, STAGE_ALPHA_OP, secondaryTextures[pack] >= 0 ? 4 : (unboundAlphaOp));         \
        if (materials[pack] < 0)                                                                          \
        {                                                                                                 \
            FUN_10143b50();                                                                               \
            FUN_1013ef90(0, STAGE_COLOUR_OP, 1);                                                          \
        }                                                                                                 \
        else if (DAT_10f65580 == 0)                                                                       \
            FUN_1013ef90(0, STAGE_COLOUR_OP, 4);                                                          \
        if (secondaryTextures[pack] <= 0)                                                                 \
        {                                                                                                 \
            FUN_1013ef90(1, STAGE_COLOUR_OP, 1);                                                          \
            FUN_1013ef90(1, STAGE_ALPHA_OP, 1);                                                           \
        }                                                                                                 \
    } while (0)

// Draws pack `pack` (through `last` when merged) with the blend, cull, sampler and alpha states its edge word asks
// for, and restores them afterwards.
#define DRAW_RANGE(merged)                                                                                \
    {                                                                                                     \
        int vertexStart = vertexStarts[pack];                                                             \
        int vertexCount = vertexEnds[last] - vertexStart;                                                 \
        int indexStart = indexStarts[pack];                                                               \
        int triangleCount = (indexEnds[last] - indexStart) / TRIANGLE_VERTICES;                           \
        if (!(merged) || (vertexCount > 0 && triangleCount > 0))                                          \
            DAT_10f1fcf0->DrawIndexedPrimitive(D3DPT_TRIANGLELIST_, 0, vertexStart, vertexCount,          \
                                               indexStart, triangleCount);                                \
        DAT_10f1fcf4++;                                                                                   \
        DAT_10f623c8 += vertexEnds[last] - vertexStarts[pack];                                            \
    }

#define DRAW_PACK(merged)                                                                                 \
    do                                                                                                    \
    {                                                                                                     \
        if (mode == 0x3a)                                                                                 \
            previousBlend = FUN_1013f070(RENDER_BLEND_MODE, 2, 0);                                        \
        else if (mode == MODE_RAINBOW_WOBBLE_COPY || mode == MODE_RAINBOW_COPY)                           \
            previousBlend = FUN_1013f070(RENDER_BLEND_MODE, 1, 0);                                        \
        else if (DAT_10f65580 != 0)                                                                       \
            previousBlend = FUN_1013f070(RENDER_BLEND_MODE, 0, 0);                                        \
        else if (((merged) || noMerge == 0) && (mode == 4 || doubleSide[pack] == 0))                      \
        {                                                                                                 \
            FUN_1013f000(0, 1, 1);                                                                        \
            FUN_1013f000(0, 2, 1);                                                                        \
            DRAW_RANGE(merged);                                                                           \
            break;                                                                                        \
        }                                                                                                 \
        else                                                                                              \
        {                                                                                                 \
            int blend = 0;                                                                                \
            int edges = doubleSide[pack];                                                                 \
            int bitA = edges & EDGE_BLEND_BIT_A;                                                          \
            if ((edges & EDGE_ADDITIVE) && DAT_10202b14 != 5)                                             \
                blend = 1;                                                                                \
            else if ((edges & EDGE_MULTIPLY) && DAT_10202b14 != 5)                                        \
                blend = 2;                                                                                \
            else if (bitA != 0 && (edges & (EDGE_BLEND_BIT_B | EDGE_BLEND_BIT_C)) == 0 && DAT_10202b14 != 5) \
                blend = 10;                                                                               \
            else if (bitA == 0 && (edges & EDGE_BLEND_BIT_B) != 0 && (edges & EDGE_BLEND_BIT_C) == 0 &&   \
                     DAT_10202b14 != 5)                                                                   \
                blend = 11;                                                                               \
            else if (bitA == 0 && (edges & EDGE_BLEND_BIT_B) == 0 && (edges & EDGE_BLEND_BIT_C) != 0 &&   \
                     DAT_10202b14 != 5)                                                                   \
                blend = 12;                                                                               \
            else if (bitA != 0 && (edges & EDGE_BLEND_BIT_B) == 0 && (edges & EDGE_BLEND_BIT_C) != 0 &&   \
                     DAT_10202b14 != 5)                                                                   \
                blend = 13;                                                                               \
            if (blend != 0)                                                                               \
            {                                                                                             \
                previousBlend = FUN_1013f070(RENDER_BLEND_MODE, blend, 0);                                \
                FUN_1013f070(RENDER_DEPTH_WRITE, 0, 0);                                                   \
            }                                                                                             \
        }                                                                                                 \
        if ((doubleSide[pack] & EDGE_CULL_NONE) != 0 || (!(merged) && noMerge != 0))                      \
            FUN_1013f070(RENDER_CULL_MODE, 1, 0);                                                         \
        FUN_1013f000(0, 1, (doubleSide[pack] & 0x80) ? 2 : ((doubleSide[pack] & 0x40) ? 3 : 1));          \
        FUN_1013f000(0, 2, (doubleSide[pack] & 0x20) ? 2 : ((doubleSide[pack] & 0x10) ? 3 : 1));          \
        if ((doubleSide[pack] & EDGE_ALPHA_TEST_OFF) != 0)                                                \
            FUN_1013f070(RENDER_ALPHA_TEST_OFF, 1, 0);                                                    \
        if ((doubleSide[pack] & EDGE_ALPHA_REFERENCE_HIGH) != 0)                                          \
            FUN_1013f070(RENDER_ALPHA_TEST, 1, 0xf0);                                                     \
        if ((doubleSide[pack] & EDGE_POINT_FILTER) != 0)                                                  \
            FUN_1013f070(RENDER_TEXTURE_FILTER, 0, 0);                                                    \
        if (!(merged) && noMerge != 0)                                                                    \
            FUN_1013f070(RENDER_CULL_MODE, 1, 0);                                                         \
        DRAW_RANGE(merged);                                                                               \
        if (mode == MODE_RAINBOW_WOBBLE_COPY || mode == MODE_RAINBOW_COPY ||                              \
            (doubleSide[pack] & EDGE_CULL_NONE) != 0 || (!(merged) && noMerge != 0))                      \
            FUN_1013f070(RENDER_CULL_MODE, 0, 0);                                                         \
        if (DAT_10f65580 != 0)                                                                            \
            FUN_1013f070(RENDER_BLEND_MODE, previousBlend, 0);                                            \
        if ((doubleSide[pack] & EDGE_ALPHA_TEST_OFF) != 0)                                                \
            FUN_1013f070(RENDER_ALPHA_TEST_OFF, 0, 0);                                                    \
        if ((doubleSide[pack] & EDGE_ALPHA_REFERENCE_HIGH) != 0)                                          \
            FUN_1013f070(RENDER_ALPHA_TEST, 1, 0xf);                                                      \
        if ((doubleSide[pack] & EDGE_POINT_FILTER) != 0)                                                  \
            FUN_1013f070(RENDER_TEXTURE_FILTER, 1, 0);                                                    \
        if (((doubleSide[pack] & EDGE_BLEND_MASK) != 0 || (doubleSide[pack] & EDGE_EXTRA_BLEND_MASK) != 0) && \
            DAT_10202b14 != 5)                                                                            \
        {                                                                                                 \
            FUN_1013f070(RENDER_BLEND_MODE, previousBlend, 0);                                            \
            FUN_1013f070(RENDER_DEPTH_WRITE, 1, 0);                                                       \
        }                                                                                                 \
    } while (0)

// 0x10151ba0 (MeshData::DrawLit). Draws the mesh with the world matrix `world` and the view `view` in drawing mode
// `mode`, lit by the light set `lights` (a block of vertex shader constants: 6 vectors from register 0x5a, more for
// the rainbow and cel modes), tinted by `colour` and `addColour` (either may be null). The first half sets the
// vertex shader, its constants and the texture stage states of the mode (a goto chain over the modes, as in the
// original); the second binds the buffers and draws the packs of the range from m_003c, merging neighbours that
// share a texture and edge flags and leaving out the ranges that face away from the camera or lie outside the
// overhead area (counted in DAT_10f623bc). Some modes then draw the mesh again (outline 0x26/0x28 and overlay 5 with
// the plain modes 4 and 6, the rainbow bases 0x22/0x24 with their copies 0x23/0x25). Returns 0.
int MeshData::DrawLit(Matrix4 *world, const Matrix4 *view, int mode, const void *lights, Vec4 *colour,
                      Vec4 *addColour)
{
    Matrix4 transform;
    Vec4 *lightSet = (Vec4 *)lights;
    int shaderId;
    int noMerge = 0;

    FUN_1013f070(RENDER_TEXTURE_FILTER, 1, 0);
    FUN_1013f070(RENDER_ALPHA_TEST, 1, 0xf);
    if (textureSlot < 0)
        return 0;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 5, 0);
    FUN_1013ef90(0, 6, 2);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(0, 2, 0);
    FUN_1013ef90(0, 3, 2);
    FUN_1013ef90(1, 1, 1);
    FUN_1013ef90(1, 4, 1);
    DAT_10f1fcf0->SetPixelShader(0);
    if (mode != 0x1f4) goto L51e72;
    noMerge = 1;
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5e, DAT_10f0d5cc.x, DAT_10f0d5cc.y, DAT_10f0d5cc.z, 1.0f);
    shaderId = 0x18;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    FUN_1013ef90(0, 1, 3);
    FUN_1013ef90(0, 2, 2);
    FUN_1013ef90(0, 3, 0);
    goto drawPacks;
L51e72:
    if (mode != 0x2b) goto L5206d;
    noMerge = 1;
    SET_VECTOR(0x51, 0.5f, 0.5f, 0.8f, 1.0f);
    SET_VECTOR(0x50, 1.0f, 0, DAT_10f66910, 5e-06f);
    SET_VECTOR(0x52, 1.0f, 0, 0, 5e-05f);
    SET_VECTOR(0x53, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    shaderId = 0x26;
    FUN_1013ef90(0, 4, 3);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    FUN_1013ef90(0, 1, 3);
    FUN_1013ef90(0, 2, 2);
    FUN_1013ef90(0, 3, 0);
    goto drawPacks;
L5206d:
    if (mode == 0x2d) goto L520bc;
    if (mode == 0x12) goto L520bc;
    if (mode == 0x36) goto L520bc;
    if (mode == 0x25) goto L520bc;
    if (mode == 0x24) goto L520bc;
    if (mode == 0x23) goto L520bc;
    if (mode == 0x22) goto L520bc;
    if (mode == 0x21) goto L520bc;
    if (mode == 0x2f) goto L520bc;
    if (mode == 0x31) goto L520bc;
    if (mode == 0x20) goto L520bc;
    if (mode == 0x2c) goto L520bc;
    if (mode == 1) goto L520bc;
    if (mode == 2) goto L520bc;
    if (mode != 5) goto L522cb;
L520bc:
    if (DAT_10f65580 == 0) goto L52517;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0, 0, 0);
    shaderId = 0x17;
    FUN_1013ef90(0, 4, 2);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    FUN_1013ef90(0, 1, 3);
    FUN_1013ef90(0, 2, 2);
    FUN_1013ef90(0, 3, 0);
    SET_VECTOR(0x5a, DAT_10f65688.x, DAT_10f65688.y, DAT_10f65688.z, DAT_10f65688.Distance(DAT_10f66938));
    SET_VECTOR(0x5b, 0, 1e-06f, 1.0f, 2250000.0f);
    SET_VECTOR(0x5c, 0, 0, 0, 1.0f);
    goto drawPacks;
L522cb:
    if (DAT_10f65580 == 0) goto L52517;
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(0, 4, Transpose(*world * *view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(0x5a, DAT_10f65688.x, DAT_10f65688.y, DAT_10f65688.z, DAT_10f65688.Distance(DAT_10f66938));
    SET_VECTOR(0x5b, 0, 1e-06f, 1.0f, 2250000.0f);
    SET_VECTOR(0x5c, 0, 0, 0, 1.0f);
    FUN_1013ef90(0, 4, 2);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    FUN_1013ef90(0, 1, 3);
    FUN_1013ef90(0, 2, 2);
    FUN_1013ef90(0, 3, 0);
    shaderId = 0x16;
    goto drawPacks;
L52517:
    if (mode != 0xf) goto L526eb;
    SET_MATRIX(0, 4, Transpose(*world * *view));
    SET_VECTOR(7, 0, 0, 1.0f, 1.0f);
    SET_VECTOR(5, 0, 0, 0, 0);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 0);
    SET_VECTOR(6, 0, 0, 0, 0.1f);
    shaderId = 8;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L526eb:
    if (mode != 3) goto L528dc;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(7, 0, 0, 1.0f, 1.0f);
    SET_VECTOR(5, 0, 0, 0, 0);
    SET_VECTOR(6, 0, 0, 0, 0.1f);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 0);
    shaderId = 0x10;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L528dc:
    if (mode != 0x2e) goto L52b65;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(7, 0, 0, 1.0f, 1.0f);
    SET_VECTOR(5, 0, 0, 0, 0);
    SET_VECTOR(6, 0, 0, 0, 0.1f);
    SET_VECTOR_IF(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f, colour->x, colour->x, colour->x, 1.0f);
    SET_VECTOR_IF(0x5e, colour, 0, 0, 0, 0, 0, 0, colour->z, 0);
    SET_VECTOR_IF(0x5d, colour, 0, 0, 0, 0, colour->y, colour->y, colour->y, 0);
    SET_VECTOR_IF(0x5c, colour, 1.0f, 1.0f, 1.0f, 1.0f, colour->w, colour->w, colour->w, 1.0f);
    shaderId = 0x2a;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L52b65:
    if (mode != 0x1c) goto L52cdd;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(7, 0, 0, 0, 0);
    lightSet[0].w = 1.0f;
    SET_VECTORS(0x5a, lightSet, 6);
    shaderId = 0x19;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L52cdd:
    if (mode == 0x20) goto L52cf8;
    if (DAT_10203414 != 0) goto L52f82;
    if (mode != 2) goto L52f82;
L52cf8:
    if (lights == 0) goto L52f82;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(5, 0.01f, 0.99f, 0, 0);
    SET_VECTORS(7, lightSet + 16, 1);
    SET_VECTOR_OR(6, colour, 1.0f, 1.0f, 1.0f, 0);
    SET_VECTOR_OR(0x59, addColour, 0, 0, 0, 0);
    lightSet[0].w = 1.0f;
    SET_VECTORS(0x5a, lightSet, 6);
    shaderId = 0x1d;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    goto drawPacks;
L52f82:
    if (mode == 0x22) goto L52fa3;
    if (mode == 0x24) goto L52fa3;
    if (DAT_10203414 != 0) goto L5320a;
    if (mode != 2) goto L5320a;
L52fa3:
    if (lights == 0) goto L5320a;
    if (DAT_10f65520 == 0) goto L5320a;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 1.0f, 1.0f, 1.0f, DAT_10f65594);
    SET_VECTOR(5, 0.01f, 0.99f, 0, 0);
    SET_VECTOR_OR(6, colour, 1.0f, 1.0f, 1.0f, 0);
    SET_VECTORS(7, lightSet + 16, 1);
    lightSet[0].w = 1.0f;
    SET_VECTORS(0x5a, lightSet, 6);
    shaderId = 0x20;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    FUN_1013ef90(1, 1, 4);
    FUN_1013ef90(1, 2, 2);
    FUN_1013ef90(1, 3, 1);
    FUN_1013ef90(1, 4, 4);
    FUN_1013ef90(1, 5, 2);
    FUN_1013ef90(1, 6, 1);
    goto drawPacks;
L5320a:
    if (mode == 0x21) goto L53225;
    if (mode == 0x3c) goto L53225;
    if (DAT_10203414 != 0) goto L5348d;
    if (mode != 2) goto L5348d;
L53225:
    if (lights == 0) goto L5348d;
    if (DAT_10f65520 == 0) goto L5348d;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 1.0f, 1.0f, 1.0f, DAT_10f65594);
    SET_VECTOR(5, 0.01f, 0.99f, 0, 0);
    SET_VECTOR_OR(6, colour, 1.0f, 1.0f, 1.0f, 0);
    SET_VECTORS(7, lightSet + 16, 1);
    lightSet[0].w = 1.0f;
    SET_VECTORS(0x5a, lightSet, 6);
    shaderId = 0x20;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    FUN_1013ef90(1, 1, 4);
    FUN_1013ef90(1, 2, 2);
    FUN_1013ef90(1, 3, 1);
    FUN_1013ef90(1, 4, 4);
    FUN_1013ef90(1, 5, 2);
    FUN_1013ef90(1, 6, 1);
    goto drawPacks;
L5348d:
    if (mode == 0x3c) goto L534bc;
    if (mode == 0x24) goto L534bc;
    if (mode == 0x22) goto L534bc;
    if (mode == 0x21) goto L534bc;
    if (mode == 0x31) goto L534bc;
    if (mode == 0x2f) goto L534bc;
    if (DAT_10203414 != 0) goto L53899;
    if (mode != 2) goto L53899;
L534bc:
    if (lights == 0) goto L53893;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(5, 0.01f, 0.99f, 0, 0);
    SET_VECTOR_OR(6, colour, 1.0f, 1.0f, 1.0f, 0);
    SET_VECTORS(7, lightSet + 16, 1);
    lightSet[0].w = 1.0f;
    SET_VECTORS(0x5a, lightSet, 6);
    SET_VECTOR_OR(0x59, addColour, 0, 0, 0, 0);
    SET_VECTORS(0x58, &DAT_10f66948, 1);
    SET_VECTOR(0x57, 90000.0f, 0, (float)(AREA_COLOUR_VALUE * 0.00055), addColour->w);
    shaderId = 0x1e;
    if (mode != 0x3c) goto L53778;
    shaderId = 0x3d;
    goto L53818;
L53778:
    if (mode != 0x31) goto L537c9;
    SET_VECTOR(4, 1.0f, 1.0f, 1.0f, (DAT_10f65598 - 20.0f));
    shaderId = 0x20;
    goto L53818;
L537c9:
    if (mode != 0x2f) goto L53818;
    SET_VECTOR(4, 1.0f, 1.0f, 1.0f, (DAT_10f65598 + 20.0f));
    shaderId = 0x2b;
L53818:
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    FUN_1013ef90(1, 1, 4);
    FUN_1013ef90(1, 2, 2);
    FUN_1013ef90(1, 3, 1);
    FUN_1013ef90(1, 4, 4);
    FUN_1013ef90(1, 5, 2);
    FUN_1013ef90(1, 6, 1);
    goto drawPacks;
L53893:
L53899:
    if (mode == 1) goto L538af;
    if (DAT_10203414 != 0) goto L53a64;
    if (mode != 2) goto L53a64;
L538af:
    if (lights == 0) goto L53a6b;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(5, 0.01f, 0.99f, 0, 0);
    SET_VECTORS(7, lightSet + 16, 1);
    lightSet[0].w = 1.0f;
    SET_VECTORS(0x5a, lightSet, 6);
    shaderId = 2;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    goto drawPacks;
L53a64:
L53a6b:
    if (mode != 6) goto L53e0d;
    if (lights == 0) goto L55757;
    SET_BOUNDS_CONSTANTS(DAT_10f657e8, DAT_10f628d4);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(5, 0.075f, 0.075f, 0.075f, 0.5f);
    SET_VECTOR(7, 0.54f, 0.40499997f, 0.27f, 0);
    lightSet[0].w = 1.0f;
    SET_VECTORS(0x5a, lightSet, 6);
    SET_VECTORS(6, lightSet + 6, 1);
    shaderId = 0xf;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    goto drawPacks;
L53e0d:
    if (mode != 5) goto L5417f;
    if (lights == 0) goto L55757;
    {
        Vec4 position = world->GetTranslation();
        FUN_10147620(&position, DAT_10f657e8, (int *)DAT_10f628d4);
    }
    SET_BOUNDS_CONSTANTS(DAT_10f657e8, DAT_10f628d4);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(7, 0.54f, 0.40499997f, 0.27f, 0);
    lightSet[0].w = 1.0f;
    SET_VECTORS(0x5a, lightSet, 6);
    SET_VECTORS(6, lightSet + 6, 1);
    shaderId = 0xe;
    FUN_1013ef90(0, 4, 3);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(0, 2, 2);
    FUN_1013ef90(0, 3, 0);
    goto drawPacks;
L5417f:
    if (mode != 0x1b) goto L54592;
    if (lights == 0) goto L55757;
    {
        Vec4 bounds[4];
        float counts[4];
        Vec4 position = world->GetTranslation();
        FUN_10147620(&position, (Bounds *)bounds, (int *)counts);
        SET_BOUNDS_CONSTANTS(bounds, counts);
    }
    SET_VECTOR_IF(0x54, colour, 1.0f, 1.0f, 1.0f, 0, colour->x, colour->y, colour->z, colour->w);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(7, 0.54f, 0.40499997f, 0.27f, 0);
    lightSet[0].w = 1.0f;
    SET_VECTORS(0x5a, lightSet, 6);
    SET_VECTORS(6, lightSet + 6, 1);
    shaderId = 0xe;
    FUN_1013ef90(0, 4, 2);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L54592:
    if (mode == 2) goto L545a0;
    if (mode != 0x26) goto L5498a;
L545a0:
    if (lights == 0) goto L5498a;
    {
        Vec4 bounds[4];
        float counts[4];
        Vec4 position = world->GetTranslation();
        FUN_10147620(&position, (Bounds *)bounds, (int *)counts);
        SET_BOUNDS_CONSTANTS(bounds, counts);
    }
    SET_VECTOR_IF(0x54, colour, 1.0f, 1.0f, 1.0f, 0, colour->x, colour->y, colour->z, colour->w);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(7, 0.54f, 0.40499997f, 0.27f, 0);
    lightSet[0].w = 1.0f;
    SET_VECTORS(0x5a, lightSet, 6);
    SET_VECTORS(6, lightSet + 6, 1);
    shaderId = 0xe;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    goto drawPacks;
L5498a:
    if (mode == 0x27) goto L54998;
    if (mode != 0x28) goto L54d82;
L54998:
    if (lights == 0) goto L54d82;
    {
        Vec4 bounds[4];
        float counts[4];
        Vec4 position = world->GetTranslation();
        FUN_10147620(&position, (Bounds *)bounds, (int *)counts);
        SET_BOUNDS_CONSTANTS(bounds, counts);
    }
    SET_VECTOR_IF(0x54, colour, 1.0f, 1.0f, 1.0f, 0, colour->x, colour->y, colour->z, colour->w);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(7, 0.54f, 0.40499997f, 0.27f, 0);
    lightSet[0].w = 1.0f;
    SET_VECTORS(0x5a, lightSet, 6);
    SET_VECTORS(6, lightSet + 6, 1);
    shaderId = 0x23;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    goto drawPacks;
L54d82:
    if (mode != 4) goto L54f56;
    shaderId = mode;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(7, 0.3f, 0.03f, 0.04f, 1.0f);
    SET_VECTOR(5, 0, 0, 0, 1.0f);
    SET_VECTOR(6, 3.0f, 3.0f, 3.0f, 1.0f);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L54f56:
    if (mode != 0x35) goto L5530f;
    if (DAT_10202b10 != 0)
        SET_SCENE_FOG();
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(8, 4, Transpose(*world));
    SET_MATRIX(0, 4, Transpose(*view));
    SET_VECTOR(5, DAT_10f65848[0].x, DAT_10f65848[0].y, 300.0f, DAT_10f65848[0].w);
    SET_VECTOR(0x5f, DAT_10f65848[1].x, DAT_10f65848[1].y, 300.0f, DAT_10f65848[1].w);
    SET_VECTOR(6, 1.5e-06f, 0, 300.0f, DAT_10f65368);
    SET_VECTOR(7, 15.01f, 15.01f, -5.0f, DAT_10f65368);
    SET_VECTOR(0x5e, 0.08f, 0.08f, 0.08f, 1.0f);
    SET_VECTORS(0x50, &DAT_10f65688, 1);
    SET_VECTORS(0x51, &DAT_10f66968, 1);
    SET_VECTOR_OR(0x5d, colour, 1.0f, 1.0f, 1.0f, 0);
    shaderId = 0x31;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L5530f:
    if (mode != 0x16) goto L5556e;
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(8, 4, Transpose(*world));
    SET_MATRIX(0, 4, Transpose(*view));
    SET_VECTOR(5, DAT_10f65848[0].x, DAT_10f65848[0].y, 300.0f, DAT_10f65848[0].w);
    SET_VECTOR(0x5f, DAT_10f65848[1].x, DAT_10f65848[1].y, 300.0f, DAT_10f65848[1].w);
    SET_VECTOR(6, 1.5e-06f, 0, 300.0f, DAT_10f65368);
    SET_VECTOR(7, 15.01f, 15.01f, -5.0f, DAT_10f65368);
    SET_VECTOR(0x5e, 0.08f, 0.08f, 0.08f, 1.0f);
    shaderId = 0x12;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L5556e:
    if (mode != 0x1a) goto L55757;
    if (lights == 0) goto L55757;
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 4, Transpose(*world));
    SET_VECTOR_OR(0x53, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    lightSet[0].w = 1.0f;
    lightSet[18].Copy(DAT_10f66908);
    lightSet[19].x = 0.8f;
    lightSet[19].y = 1.0f;
    lightSet[19].z = 0.8f;
    lightSet[18].w = 100.0f;
    SET_VECTORS(0x54, lightSet + 8, 0xc);
    shaderId = 0x15;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L55757:
    if (DAT_10f65520 == 0) goto L55e45;
    if (mode != 0x37) goto L559c7;
    SET_VECTOR(9, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(8, 1.0f, 1.0f, (DAT_10f65594 - 20.0f), 1.0f);
    SET_MATRIX(0, 4, Transpose(*world * *view));
    SET_MATRIX(4, 4, Transpose(*world));
    SET_VECTOR_IF(0x5f, !(doubleSide[0] & 4) && colour, 1.0f, 1.0f, 1.0f, 1.0f, colour->x, colour->y, colour->z, colour->w);
    SET_VECTOR_OR(0x5e, addColour, 0, 0, 0, 0);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x36;
    goto drawPacks;
L559c7:
    if (mode == 8) goto L559d5;
    if (mode != 0xd) goto L55bdd;
L559d5:
    if (mode != 0xd) goto L559fd;
    FUN_1013ef30(8, 2);
L559fd:
    SET_VECTOR(9, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(8, 1.0f, 1.0f, (DAT_10f65594 - 20.0f), 1.0f);
    SET_MATRIX(0, 4, Transpose(*world * *view));
    SET_MATRIX(4, 4, Transpose(*world));
    SET_VECTOR_IF(0x5f, !(doubleSide[0] & 4) && colour, 1.0f, 1.0f, 1.0f, 1.0f, colour->x, colour->y, colour->z, colour->w);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x1f;
    goto drawPacks;
L55bdd:
    if (mode != 0x33) goto L55e45;
    SET_VECTOR(9, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(8, 1.0f, 1.0f, (DAT_10f65594 - 20.0f), 1.0f);
    SET_MATRIX(0, 4, Transpose(*world * *view));
    SET_MATRIX(4, 4, Transpose(*world));
    SET_VECTOR_IF(0x5f, !(doubleSide[0] & 4) && colour, 1.0f, 1.0f, 1.0f, 1.0f, colour->x, colour->y, colour->z, colour->w);
    SET_VECTOR_OR(0x5e, addColour, 0, 0, 0, 0);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x2e;
    goto drawPacks;
L55e45:
    if (mode == 8) goto L5aa74;
    if (mode == 0xd) goto L5aa74;
    if (mode != 0x37) goto L5603b;
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(0, 4, Transpose(*world * *view));
    SET_VECTOR_IF(0x5f, !(doubleSide[0] & 4) && colour, 1.0f, 1.0f, 1.0f, 1.0f, colour->x, colour->y, colour->z, colour->w);
    SET_VECTOR_OR(0x5e, addColour, 0, 0, 0, 0);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x35;
    goto drawPacks;
L5603b:
    if (mode != 0x33) goto L5621f;
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(0, 4, Transpose(*world * *view));
    SET_VECTOR_IF(0x5f, !(doubleSide[0] & 4) && colour, 1.0f, 1.0f, 1.0f, 1.0f, colour->x, colour->y, colour->z, colour->w);
    SET_VECTOR_OR(0x5e, addColour, 0, 0, 0, 0);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x2d;
    goto drawPacks;
L5621f:
    if (mode != 0x1f) goto L56459;
    if (DAT_10f65520 == 0) goto L5a7ca;
    SET_VECTOR(8, 1.0f, 1.0f, 1.0f, DAT_10f65594);
    SET_MATRIX(4, 4, Transpose(*world));
    SET_MATRIX(0, 4, Transpose(*view));
    SET_VECTOR(0x5e, 0, 0, -0.8f, 0);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTORS(0x58, &DAT_10f66948, 1);
    SET_VECTOR(0x57, 90000.0f, 0, (float)(AREA_COLOUR_VALUE * 0.00055), 1.0f);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x21;
    goto drawPacks;
L56459:
    if (mode == 0x30) goto L5a7ca;
    if (mode == 0x32) goto L5a7ca;
    if (DAT_10f65520 == 0) goto L567c2;
    if (mode != 0x1d) goto L56a99;
    if (lights == 0) goto L56a99;
    if (DAT_10202b10 != 0)
        SET_SCENE_FOG();
    SET_VECTOR(0x11, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x10, 1.0f, 1.0f, (DAT_10f65594 - 20.0f), 1.0f);
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 4, Transpose(*world));
    SET_VECTORS(0x50, &DAT_10f65688, 1);
    SET_VECTORS(0x51, &DAT_10f66968, 1);
    SET_VECTOR_OR(0x53, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    lightSet[0].w = 1.0f;
    lightSet[18].Copy(DAT_10f66908);
    lightSet[19].x = 0.8f;
    lightSet[19].y = 1.0f;
    lightSet[19].z = 0.8f;
    lightSet[18].w = 100.0f;
    SET_VECTORS(0x54, lightSet + 8, 0xc);
    shaderId = 0x32;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L567c2:
    if (mode != 0x1d) goto L57027;
    if (lights == 0) goto L5a4e7;
    if (DAT_10202b10 != 0)
        SET_SCENE_FOG();
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 4, Transpose(*world));
    SET_VECTORS(0x50, &DAT_10f65688, 1);
    SET_VECTORS(0x51, &DAT_10f66968, 1);
    SET_VECTOR_OR(0x53, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    lightSet[0].w = 1.0f;
    lightSet[18].Copy(DAT_10f66908);
    lightSet[19].x = 0.8f;
    lightSet[19].y = 1.0f;
    lightSet[19].z = 0.8f;
    lightSet[18].w = 100.0f;
    SET_VECTORS(0x54, lightSet + 8, 0xc);
    shaderId = 0x1b;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L56a99:
    if (mode != 0x1e) goto L56db1;
    if (DAT_10202b10 != 0)
        SET_SCENE_FOG();
    SET_VECTOR(0x11, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x10, 1.0f, 1.0f, (DAT_10f65594 - 20.0f), 1.0f);
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(8, 4, Transpose(*world));
    SET_MATRIX(0, 4, Transpose(*view));
    SET_VECTORS(0x50, &DAT_10f65688, 1);
    SET_VECTORS(0x51, &DAT_10f66968, 1);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5e, 0.001f, colour->y, colour->z, colour->w);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x25;
    goto drawPacks;
L56db1:
    if (mode != 0x38) goto L57225;
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(8, 4, Transpose(*world));
    SET_MATRIX(0, 4, Transpose(*view));
    SET_VECTORS(0x50, &DAT_10f65688, 1);
    SET_VECTOR(0x11, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x10, 1.0f, 1.0f, (DAT_10f65594 - 20.0f), 1.0f);
    SET_VECTORS(0x51, &DAT_10f66968, 1);
    SET_VECTOR(0x5f, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5e, 0.001f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5d, ((1.0f - DAT_10f66974) * 0.8f), ((1.0f - DAT_10f66974) * 0.8f), ((1.0f - DAT_10f66974) * 0.8f), ((1.0f - DAT_10f66974) * 0.8f));
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x39;
    goto drawPacks;
L57027:
    if (mode != 0x38) goto L57225;
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(8, 4, Transpose(*world));
    SET_MATRIX(0, 4, Transpose(*view));
    SET_VECTORS(0x50, &DAT_10f65688, 1);
    SET_VECTORS(0x51, &DAT_10f66968, 1);
    SET_VECTOR(0x5f, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5e, 0.001f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5d, ((1.0f - DAT_10f66974) * 0.8f), ((1.0f - DAT_10f66974) * 0.8f), ((1.0f - DAT_10f66974) * 0.8f), ((1.0f - DAT_10f66974) * 0.8f));
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x38;
    goto drawPacks;
L57225:
    if (mode != 0x1e) goto L574c5;
    if (DAT_10202b10 != 0)
        SET_SCENE_FOG();
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(8, 4, Transpose(*world));
    SET_MATRIX(0, 4, Transpose(*view));
    SET_VECTORS(0x50, &DAT_10f65688, 1);
    SET_VECTORS(0x51, &DAT_10f66968, 1);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5e, 0.001f, colour->y, colour->z, colour->w);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x1a;
    goto drawPacks;
L574c5:
    if (DAT_10f65520 == 0) goto L5784d;
    if (mode != 0x34) goto L57b55;
    if (DAT_10202b10 != 0)
        SET_SCENE_FOG();
    SET_VECTOR(0x11, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x10, 1.0f, 1.0f, (DAT_10f65594 - 20.0f), 1.0f);
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(8, 4, Transpose(*world));
    SET_MATRIX(0, 4, Transpose(*view));
    SET_VECTORS(0x50, &DAT_10f65688, 1);
    SET_VECTORS(0x51, &DAT_10f66968, 1);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5e, 0.001f, colour->y, colour->z, colour->w);
    SET_VECTOR_OR(0x5d, addColour, 0, 0, 0, 0);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x30;
    goto drawPacks;
L5784d:
    if (mode != 0x34) goto L57b55;
    if (DAT_10202b10 != 0)
        SET_SCENE_FOG();
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(8, 4, Transpose(*world));
    SET_MATRIX(0, 4, Transpose(*view));
    SET_VECTORS(0x50, &DAT_10f65688, 1);
    SET_VECTORS(0x51, &DAT_10f66968, 1);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5e, 0.001f, colour->y, colour->z, colour->w);
    SET_VECTOR_OR(0x5d, addColour, 0, 0, 0, 0);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x2f;
    goto drawPacks;
L57b55:
    if (mode == 9) goto L5a683;
    if (mode == 0xe) goto L5a683;
    if (mode != 0xa) goto L57d46;
    if (DAT_10202b10 != 0)
        SET_SCENE_FOG();
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTORS(0x51, &DAT_10f66968, 1);
    SET_MATRIX(0, 4, Transpose(*world * *view));
    shaderId = 0xa;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L57d46:
    if (mode != 0xb) goto L57e4e;
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(0, 4, Transpose(*world * *view));
    shaderId = 0xb;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L57e4e:
    if (mode != 0xc) goto L57fc2;
    SET_VECTOR(4, 0.5f, -0.5f, 1.0f, 1.0f);
    SET_VECTOR(5, 0.5f, 0.5f, 2.0f, 0);
    SET_MATRIX(0, 4, Transpose(*world * *view));
    if (DAT_10203418 == 0) goto L57f83;
    shaderId = 0xc;
    FUN_1013ef90(0, 4, 4);
    goto L57f97;
L57f83:
    shaderId = 0x27;
    FUN_1013ef90(0, 4, 2);
L57f97:
    FUN_1013ef90(0, 1, 2);
    FUN_1013f000(0, 1, 2);
    FUN_1013f000(0, 2, 2);
    goto drawPacks;
L57fc2:
    if (mode != 0x3a) goto L581be;
    SET_VECTOR(4, 0.5f, -0.5f, 1.0f, 1.0f);
    SET_VECTOR(5, 0.5019531f, 0.5019531f, 100.0f, 0);
    SET_VECTOR(6, 50.0f, 50.0f, 50.0f, 0);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR_IF(0x5f, colour, 0.5019531f, 0.5019531f, 100.0f, 0, colour->x, colour->x, colour->x, colour->w);
    shaderId = 0x3c;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013f000(0, 1, 2);
    FUN_1013f000(0, 2, 2);
    goto drawPacks;
L581be:
    if (mode != 0x2c) goto L583da;
    SET_VECTOR(4, 0.5f, -0.5f, 1.0f, 1.0f);
    SET_VECTOR(5, 0.5f, 0.5f, 2.0f, 0);
    SET_VECTOR(6, (colour->y * 0.009f), (colour->y * 0.009f), 2.0f, 0);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(0x5f, colour->x, colour->x, colour->x, colour->w);
    shaderId = 0x28;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013f000(0, 1, 2);
    FUN_1013f000(0, 2, 2);
    goto drawPacks;
L583da:
    if (mode != 0x2a) goto L585d9;
    SET_VECTOR(4, 0.5f, -0.5f, 1.0f, 1.0f);
    SET_VECTOR(5, 0.5f, 0.5f, 2.0f, 0);
    SET_VECTOR(6, (colour->y * 0.009f), (colour->y * 0.009f), 2.0f, 0);
    SET_MATRIX(0, 4, Transpose(*world * *view));
    SET_VECTOR(0x5f, colour->x, colour->x, colour->x, colour->w);
    shaderId = 0x24;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013f000(0, 1, 2);
    FUN_1013f000(0, 2, 2);
    goto drawPacks;
L585d9:
    if (mode != 0x10) goto L58753;
    FUN_1013ef90(0, 4, 3);
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR_IF(0x5f, colour, 0, 0, 0, 1.0f, colour->x, colour->x, colour->x, colour->w);
    shaderId = 0;
    goto drawPacks;
L58753:
    if (DAT_10f65520 == 0) goto L58a11;
    if (mode != 0x18) goto L58c4f;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(5, DAT_10f65688.x, DAT_10f65688.y, DAT_10f65688.z, 1.0f);
    SET_VECTOR(6, 0.5f, 0.5f, 0.5f, 0.99f);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5d, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5c, 1.0f, 1.0f, (DAT_10f65594 - 20.0f), 1.0f);
    shaderId = 0x3a;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(1, 4, 1);
    FUN_1013ef90(1, 1, 7);
    FUN_1013ef90(1, 2, 1);
    FUN_1013ef90(1, 3, 2);
    goto drawPacks;
L58a11:
    if (mode != 0x18) goto L58f77;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(5, DAT_10f65688.x, DAT_10f65688.y, DAT_10f65688.z, 1.0f);
    SET_VECTOR(6, 0.5f, 0.5f, 0.5f, 0.99f);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    shaderId = 0x37;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(1, 4, 1);
    FUN_1013ef90(1, 1, 7);
    FUN_1013ef90(1, 2, 1);
    FUN_1013ef90(1, 3, 2);
    goto drawPacks;
L58c4f:
    if (mode != 0x3d) goto L59227;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(5, DAT_10f65688.x, DAT_10f65688.y, DAT_10f65688.z, 1.0f);
    SET_VECTOR(6, 0.5f, 0.5f, 0.5f, 0.99f);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5d, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5c, 1.0f, 1.0f, (DAT_10f65594 - 20.0f), 1.0f);
    SET_VECTORS(0x58, &DAT_10f66948, 1);
    SET_VECTOR(0x57, 90000.0f, 0, (float)(AREA_COLOUR_VALUE * 0.00055), 1.0f);
    shaderId = 0x3f;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(1, 4, 1);
    FUN_1013ef90(1, 1, 7);
    FUN_1013ef90(1, 2, 1);
    FUN_1013ef90(1, 3, 2);
    goto drawPacks;
L58f77:
    if (mode != 0x3d) goto L59227;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(5, DAT_10f65688.x, DAT_10f65688.y, DAT_10f65688.z, 1.0f);
    SET_VECTOR(6, 0.5f, 0.5f, 0.5f, 0.99f);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTORS(0x58, &DAT_10f66948, 1);
    SET_VECTOR(0x57, 90000.0f, 0, (float)(AREA_COLOUR_VALUE * 0.00055), 1.0f);
    shaderId = 0x3e;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(1, 4, 1);
    FUN_1013ef90(1, 1, 7);
    FUN_1013ef90(1, 2, 1);
    FUN_1013ef90(1, 3, 2);
    goto drawPacks;
L59227:
    if (mode != 0x17) goto L59465;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(5, DAT_10f65688.x, DAT_10f65688.y, DAT_10f65688.z, 1.0f);
    SET_VECTOR(6, 0.5f, 0.5f, 0.5f, 0.99f);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    shaderId = 0x13;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(1, 4, 1);
    FUN_1013ef90(1, 1, 7);
    FUN_1013ef90(1, 2, 1);
    FUN_1013ef90(1, 3, 2);
    goto drawPacks;
L59465:
    if (mode != 0x2d) goto L59710;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(5, DAT_10f65688.x, DAT_10f65688.y, DAT_10f65688.z, 1.0f);
    SET_VECTOR(6, 0.5f, 0.5f, 0.5f, 0.99f);
    SET_VECTOR_IF(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f, colour->x, colour->x, colour->x, colour->w);
    SET_VECTOR_IF(0x5e, colour, 0, 0, 0, 0, 0, 0, colour->z, 0);
    SET_VECTOR_IF(0x5d, colour, 0, 0, 0, 0, colour->y, colour->y, colour->y, 0);
    shaderId = 0x29;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L59710:
    if (mode != 0x12) goto L5991f;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(5, DAT_10f65688.x, DAT_10f65688.y, DAT_10f65688.z, 1.0f);
    SET_VECTOR(6, 0.5f, 0.5f, 0.5f, 0.99f);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    shaderId = 0x11;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L5991f:
    if (mode != 0x25) goto L59be9;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTORS(0x59, &DAT_10f655c8[0], 1);
    SET_VECTORS(0x5a, &DAT_10f655c8[1], 1);
    SET_VECTORS(0x5b, &DAT_10f655c8[2], 1);
    SET_VECTORS(0x5c, &DAT_10f655c8[3], 1);
    SET_VECTORS(0x5d, &DAT_10f655c8[4], 1);
    SET_VECTORS(0x5e, &DAT_10f655c8[5], 1);
    SET_VECTORS(0x57, &DAT_10f655c8[6], 1);
    SET_VECTORS(0x58, &DAT_10f655c8[7], 1);
    SET_VECTORS(0x55, &DAT_10f655c8[8], 1);
    SET_VECTORS(0x56, &DAT_10f655c8[9], 1);
    SET_VECTORS(0x54, &DAT_10f66948, 1);
    SET_VECTOR(0x53, 90000.0f, 0, (float)(AREA_COLOUR_VALUE * 0.00055), 1.0f);
    shaderId = 0x22;
    SET_VECTOR_IF(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, colour->w);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L59be9:
    if (DAT_10f65520 == 0) goto L59ee4;
    if (mode != 0x36) goto L5a15f;
    if (DAT_10202b10 != 0)
        SET_SCENE_FOG();
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTORS(0x5d, &DAT_10f65688, 1);
    SET_VECTORS(0x5e, &DAT_10f66968, 1);
    SET_VECTOR(0x5b, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTOR(0x5c, 1.0f, 1.0f, (DAT_10f65594 - 20.0f), 1.0f);
    shaderId = 0x34;
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L59ee4:
    if (mode != 0x36) goto L5a15f;
    if (DAT_10202b10 != 0)
        SET_SCENE_FOG();
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTORS(0x5d, &DAT_10f65688, 1);
    SET_VECTORS(0x5e, &DAT_10f66968, 1);
    shaderId = 0x33;
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L5a15f:
    if (mode != 0x23) goto L5a35e;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTORS(0x58, &DAT_10f66948, 1);
    SET_VECTOR(0x57, 90000.0f, 0, (float)(AREA_COLOUR_VALUE * 0.00055), 1.0f);
    shaderId = 0x3b;
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L5a35e:
    if (mode != 0x3b) goto L5a4e7;
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    shaderId = 0;
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 2);
    goto drawPacks;
L5a4e7:
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(8, 3, Transpose(*world));
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    shaderId = 0;
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    if (mode != 0x1b) goto L5a677;
    FUN_1013ef90(0, 4, 3);
L5a677:
    goto drawPacks;
L5a683:
    if (mode != 0xe) goto L5a6ab;
    FUN_1013ef30(8, 2);
L5a6ab:
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_VECTOR(6, 0.5f, 0.5f, 0.5f, 1.0f);
    SET_MATRIX(0, 4, Transpose(*world * *view));
    shaderId = 9;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    goto drawPacks;
L5a7ca:
    SET_MATRIX(0, 4, Transpose(*view));
    SET_MATRIX(4, 3, Transpose(*world));
    SET_VECTOR(0x5e, 0, 0, (DAT_10203420 * -0.8f), 0);
    SET_VECTOR_OR(0x5f, colour, 1.0f, 1.0f, 1.0f, 1.0f);
    SET_VECTORS(0x58, &DAT_10f66948, 1);
    SET_VECTOR(0x57, 90000.0f, 0, (float)(AREA_COLOUR_VALUE * 0.00055), 1.0f);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 0x1c;
    if (mode != 0x32) goto L5aa19;
    SET_VECTOR(8, 1.0f, 1.0f, 1.0f, (DAT_10f65598 - 20.0f));
    shaderId = 0x21;
    goto drawPacks;
L5aa19:
    if (mode != 0x30) goto L5aa68;
    SET_VECTOR(8, 1.0f, 1.0f, 1.0f, (DAT_10f65598 + 20.0f));
    shaderId = 0x2c;
L5aa68:
    goto drawPacks;
L5aa74:
    if (mode != 0xd) goto L5aa9c;
    FUN_1013ef30(8, 2);
L5aa9c:
    SET_VECTOR(4, 0, 0.1f, 0.5f, 0.8f);
    SET_MATRIX(0, 4, Transpose(*world * *view));
    SET_VECTOR_IF(0x5f, !(doubleSide[0] & 4) && colour, 1.0f, 1.0f, 1.0f, 1.0f, colour->x, colour->y, colour->z, colour->w);
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
    shaderId = 5;
L5abf3:drawPacks:
    {
        IUnknownLike *vertexBuffer = 0;
        IUnknownLike *indexBuffer = 0;
        if (FUN_10147a20(textureSlot, &vertexBuffer, &indexBuffer) == 0)
            return 0;
        DAT_10f1fcf0->SetIndices(indexBuffer);
        DAT_10f1fcf0->SetVertexShader(DAT_10f0ee00[DAT_10f65544][shaderId]);
        DAT_10f1fcf0->SetVertexDeclaration(DAT_10f654ac);
        DAT_10f1fcf0->SetStreamSource(DAT_10f65544, vertexBuffer, 0, VERTEX_STRIDE);
        DAT_10f65544++;
        if (DAT_10f65544 >= 1)
            DAT_10f65544 = 0;

        Vec4 worldPosition;
        if (DAT_10f655a4 != 0)
            FUN_1013ef30(DEVICE_STATE_OVERHEAD_VARIANT, 2);
        transform = *world * *view;
        worldPosition.Copy(world->GetTranslation());
        Vec4 viewDirection;
        Vec4 viewPoint;
        viewDirection.Copy(DAT_10f668e8 - DAT_10f65688);
        viewDirection.Normalized3();
        float ahead = VIEW_AHEAD_DISTANCE;
        viewPoint.Copy(DAT_10f65688 - worldPosition - viewDirection * ahead);
        int skipped = 0;
        int previousBlend = 0;
        Vec4 boxVectors[BOX_VECTOR_COUNT];

        for (int pack = m_003c + DAT_10f655b8; pack < m_0044 && pack < packCount; pack++)
        {
            int last = pack;
            int packsLeft = packCount - m_003c > m_0044 ? m_0044 : packCount - m_003c;
            int lastPack = m_003c + packsLeft - 1;
            if (vertexEnds[pack] - vertexStarts[pack] <= 0)
            {
                pack = last + skipped;
                skipped = 0;
                continue;
            }
            if (noMerge == 0 && pointFlags[pack] != 0)
            {
                skipped = 0;
                for (;;)
                {
                    if (last == m_003c + DAT_10f655b8)
                    {
                        DAT_10f65898.Copy(boxes[last * 2]);
                        DAT_10f66958.Copy(boxes[last * 2 + 1]);
                    }
                    DAT_10f623c4++;
                    int culled = 0;
                    if (points[last].w > 0.0f)
                    {
                        boxVectors[BOX_TO_VIEW].Copy((boxes[last * 2] + boxes[last * 2 + 1]) * 0.5f - viewPoint);
                        boxVectors[BOX_DIRECTION].Copy(Vec3_Normalized(&boxVectors[BOX_TO_VIEW]));
                        if (FACING_DOT(boxVectors[BOX_DIRECTION], viewDirection) < FACING_LIMIT)
                        {
                            boxVectors[BOX_TO_VIEW].Copy((boxes[last * 2] + boxes[last * 2 + 1]) * 0.5f - viewPoint);
                            boxVectors[BOX_TO_VIEW].z =
                                boxVectors[BOX_TO_VIEW].z - (boxes[last * 2 + 1].z - boxes[last * 2].z) * 0.5f;
                            boxVectors[BOX_DIRECTION].Copy(Vec3_Normalized(&boxVectors[BOX_TO_VIEW]));
                            if (FACING_DOT(boxVectors[BOX_DIRECTION], viewDirection) < FACING_LIMIT)
                            {
                                boxVectors[BOX_TO_VIEW].Copy((boxes[last * 2] + boxes[last * 2 + 1]) * 0.5f - viewPoint);
                                boxVectors[BOX_TO_VIEW].z =
                                    (boxes[last * 2 + 1].z - boxes[last * 2].z) * 0.5f + boxVectors[BOX_TO_VIEW].z;
                                boxVectors[BOX_DIRECTION].Copy(Vec3_Normalized(&boxVectors[BOX_TO_VIEW]));
                                if (FACING_DOT(boxVectors[BOX_DIRECTION], viewDirection) < FACING_LIMIT)
                                    culled = 1;
                            }
                        }
                        if (culled == 0 && DAT_10f655bc != 0)
                        {
                            if (boxes[last * 2].x > DAT_10f66928.x || boxes[last * 2].y > DAT_10f66928.y ||
                                boxes[last * 2].z > DAT_10f66928.z || boxes[last * 2 + 1].x < DAT_10f66918.x ||
                                boxes[last * 2 + 1].y < DAT_10f66918.y || boxes[last * 2 + 1].z < DAT_10f66918.z)
                                culled = 1;
                        }
                        if (culled == 0 && DAT_10f65580 != 0 && (doubleSide[last] & EDGE_BLEND_MASK) != 0)
                            culled = 1;
                    }
                    if (culled)
                    {
                        DAT_10f623bc++;
                        if (last == pack)
                            goto nextPack;
                        last--;
                        skipped = 1;
                        break;
                    }
                    if (last + 1 > lastPack)
                        break;
                    if (DAT_10f65580 != 0 && DAT_118d0ba8[materials[last]].hasOwnAlpha == 0 &&
                        DAT_118d0ba8[materials[last + 1]].hasOwnAlpha == 0)
                    {
                        last++;
                        continue;
                    }
                    if (materials[last] != materials[last + 1] || doubleSide[last] != doubleSide[last + 1])
                        break;
                    last++;
                }
                BIND_PACK_TEXTURES(1, 1);
                if (DAT_10f655b4 != 0)
                    FUN_10161070(DAT_10f655b4);
                DRAW_PACK(1);
            }
            else
            {
                BIND_PACK_TEXTURES(0, 3);
                if (DAT_10f655b4 != 0)
                    FUN_10161070(DAT_10f655b4);
                DRAW_PACK(0);
            }
            pack = last + skipped;
            skipped = 0;
        nextPack:;
        }
    }
    if (DAT_10f655a4 != 0)
        FUN_1013ef30(DEVICE_STATE_OVERHEAD_VARIANT, 3);

    if (DAT_10f65580 == 0)
    {
        if (mode == MODE_OUTLINE_PASS || mode == MODE_OUTLINE_PASS_ALT)
        {
            if (lights != 0)
            {
                FUN_1013f070(RENDER_BLEND_MODE, 1, 0);
                DrawLit(world, view, 4, lights, colour, addColour);
                goto blendRestored;
            }
        }
        else if (mode == MODE_OVERLAY)
        {
            if (lights != 0)
            {
                FUN_1013ef90(0, STAGE_ALPHA_OP, 2);
                FUN_1013ef90(0, STAGE_COLOUR_OP, 4);
                DrawLit(world, view, 6, lights, colour, addColour);
                FUN_1013ef90(0, STAGE_ALPHA_OP, 4);
                FUN_1013ef90(0, STAGE_COLOUR_OP, 4);
            }
        }
        else if (mode == MODE_RAINBOW_COPY || mode == MODE_RAINBOW_WOBBLE_COPY)
        {
            if (lights != 0)
                goto blendRestored;
        }
        goto rainbowPass;
    blendRestored:
        FUN_1013f070(RENDER_BLEND_MODE, 0, 0);
    rainbowPass:
        if (DAT_10f6558c != 0)
        {
            if (mode == MODE_RAINBOW_BASE && lights != 0 && DAT_10f65520 == 0)
            {
                Vec4 tint = *colour;
                tint.x = 0.0f;
                tint.y = 0.0f;
                tint.z = 0.0f;
                Vec4 tinted = tint * DAT_10f65888;
                DrawLit(world, view, MODE_RAINBOW_COPY, lights, &tinted, addColour);
            }
            if (DAT_10f6558c != 0 && mode == MODE_RAINBOW_WOBBLE_BASE)
            {
                if (lights != 0 && DAT_10f65520 == 0)
                {
                    Vec4 wobble;
                    wobble.Copy(DAT_10f65888);
                    wobble.w = wobble.w * RAINBOW_WOBBLE_SCALE + RAINBOW_WOBBLE_OFFSET;
                    Vec4 tint = *colour;
                    tint.x = 0.0f;
                    tint.y = 0.0f;
                    tint.z = 0.0f;
                    Vec4 tinted = tint * wobble;
                    DrawLit(world, view, MODE_RAINBOW_WOBBLE_COPY, lights, &tinted, addColour);
                }
                goto finished;
            }
        }
    }
    if (mode == 0xd || mode == 0xe)
        FUN_1013ef30(DEVICE_STATE_OVERHEAD_VARIANT, 3);
    if (mode == 0x18 || mode == 0x3d || mode == 0x12 || mode == 0x13 || mode == 0x14 || mode == 0x15 || mode == 0x17)
    {
        FUN_1013ef90(1, STAGE_ALPHA_OP, 4);
        FUN_1013ef90(1, STAGE_ALPHA_ARG1, 0);
        FUN_1013ef90(1, STAGE_ALPHA_ARG2, 2);
        FUN_1013ef90(1, STAGE_COLOUR_OP, 4);
        FUN_1013ef90(1, STAGE_COLOUR_ARG1, 0);
        FUN_1013ef90(1, STAGE_COLOUR_ARG2, 2);
        FUN_1013ef90(1, STAGE_COLOUR_OP, 1);
        FUN_1013ef90(1, STAGE_ALPHA_OP, 1);
        FUN_10143b80();
    }
finished:
    if (DAT_10f65580 != 0)
        FUN_1013f070(RENDER_BLEND_MODE, 0, 0);
    if (DAT_10202b10 != 0)
        FUN_1013f070(RENDER_FOG, 0, 0);
    return 0;
}
