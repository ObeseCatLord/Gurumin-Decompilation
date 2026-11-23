// Texture objects: drawing an indexed mesh through a vertex shader.
#include "d3d9_min.h"

#include "texture_object.h"

#include "matrix4.h"
#include "vec4.h"
#include "camera_vars.h"
#include "render_vars.h"
#include "render_state.h"
#include "mesh_shader.h"

extern "C" void FUN_1013ef30(int state, int value);                   // Render_SetRenderState (cached)
extern "C" void FUN_1013ef90(int stage, int type, int value);         // Render_SetTextureStageState (cached)
extern "C" void FUN_10143b80(void);                                   // Render_UnbindTexture1

#define MESH_VERTEX_STRIDE 0x70
#define MESH_INDEX_FORMAT_16 0x65         // D3DFMT_INDEX16
#define MESH_TRIANGLE_LIST 4
#define MESH_REGISTER_MATRIX 0            // first vertex-shader register of the view-projection matrix
#define MESH_REGISTER_PARAMS 4
#define MESH_REGISTER_WORLD 8
#define MESH_REGISTER_CAMERA 0x50
#define MESH_REGISTER_FOG 0x51
#define MESH_REGISTER_EXTRA 0x5e
#define MESH_REGISTER_TINT 0x5f
#define MESH_OUTLINE_WIDTH 0.001f
#define MESH_COLOUR_SCALE 255.0f

// Colour stages back to texture times diffuse after a shader pass.
static inline void ResetStage0Modulate()
{
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 1, 4);
}

// 0x1014ddf0 (TextureObject::DrawMesh). Binds this texture to stage 0 (stage 1 and up off) and draws the indexed
// triangle list (`vertexCount` vertices of 0x70 bytes, `indexCount` 16-bit indices) with the vertex shader
// `shader` (a MeshShaderMode). Each mode first loads its constants: the outline pass uploads the world and
// view-projection matrices, the camera position, fog data and, when DAT_10202b10 is set, the fog colour as render
// state; `colour` and `extra` are optional tints (default white and black). The toon mode is skipped when
// DAT_10f6548c is set. Returns 0 when skipped, else 1.
int TextureObject::DrawMesh(Matrix4 *world, Matrix4 *viewProjection, void *vertices, int vertexCount,
                            void *indices, int indexCount, int shader, const Vec4 *colour, const Vec4 *extra)
{
    int previousBlend = -1;
    BindStage0();
    FUN_10143b80();
    if (shader == MESH_SHADER_TOON && DAT_10f6548c != 0)
        return 0;
    FUN_1013ef90(0, 4, 4);
    FUN_1013ef90(0, 5, 2);
    FUN_1013ef90(0, 6, 0);
    FUN_1013ef90(0, 1, 4);
    FUN_1013ef90(0, 2, 2);
    FUN_1013ef90(0, 3, 0);
    Matrix4 toonMatrix;
    FUN_10143b80();
    DAT_10f1fcf0->SetTexture(2, 0);
    DAT_10f1fcf0->SetTexture(3, 0);
    if (shader == MESH_SHADER_OUTLINE)
    {
        if (DAT_10202b10 != 0)
        {
            float red;
            if (1.0f > DAT_10f668f8.x)
                red = DAT_10f668f8.x;
            else
                red = 1.0f;
            float green;
            if (1.0f > DAT_10f668f8.y)
                green = DAT_10f668f8.y;
            else
                green = 1.0f;
            float blue;
            if (1.0f > DAT_10f668f8.z)
                blue = DAT_10f668f8.z;
            else
                blue = 1.0f;
            FUN_1013f070(RENDER_FOG, 1,
                         ((((int)(red * MESH_COLOUR_SCALE) | 0xffffff00) << 8 | (int)(green * MESH_COLOUR_SCALE)) << 8) |
                             (int)(blue * MESH_COLOUR_SCALE));
        }
        float tint[4];
        Matrix4 matrix;
        float params[4] = {0.0f, 0.1f, 0.5f, 0.8f};
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_PARAMS, params, 1);
        matrix = Transpose(*world);
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_WORLD, matrix.m, 4);
        matrix = Transpose(*viewProjection);
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_MATRIX, matrix.m, 4);
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_CAMERA, &DAT_10f65688.x, 1);
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_FOG, &DAT_10f66968, 1);
        tint[0] = tint[1] = tint[2] = tint[3] = 1.0f;
        if (colour != 0)
        {
            tint[0] = colour->x;
            tint[1] = colour->y;
            tint[2] = colour->z;
            tint[3] = colour->w;
        }
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_TINT, tint, 1);
        tint[0] = MESH_OUTLINE_WIDTH;
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_EXTRA, tint, 1);
        ResetStage0Modulate();
    }
    else if (shader == MESH_SHADER_SHADED)
    {
        Matrix4 matrix;
        float params[4] = {0.0f, 0.1f, 0.5f, 0.8f};
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_PARAMS, params, 1);
        matrix = Transpose(*world * *viewProjection);
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_MATRIX, matrix.m, 4);
        float side[4];
        float tint[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        if (colour != 0)
        {
            tint[0] = colour->x;
            tint[1] = colour->y;
            tint[2] = colour->z;
            tint[3] = colour->w;
        }
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_TINT, tint, 1);
        side[0] = side[1] = side[2] = side[3] = 0.0f;
        if (extra != 0)
        {
            side[0] = extra->x;
            side[1] = extra->y;
            side[2] = extra->z;
            side[3] = extra->w;
        }
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_EXTRA, side, 1);
        ResetStage0Modulate();
    }
    else if (shader == MESH_SHADER_TEXTURED)
    {
        Matrix4 matrix;
        float params[4] = {0.0f, 0.1f, 0.5f, 0.8f};
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_PARAMS, params, 1);
        matrix = Transpose(*world * *viewProjection);
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_MATRIX, matrix.m, 4);
        float tint[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        if (colour != 0)
        {
            tint[0] = colour->x;
            tint[1] = colour->y;
            tint[2] = colour->z;
            tint[3] = colour->w;
        }
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_TINT, tint, 1);
        ResetStage0Modulate();
    }
    else if (shader == MESH_SHADER_TOON)
    {
        float params[4] = {0.5f, -0.5f, 1.0f, 1.0f};
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_PARAMS, params, 1);
        float scale[4] = {0.5f, 0.5f, 2.0f, 0.0f};
        DAT_10f1fcf0->SetVertexShaderConstantF(5, scale, 1);
        float bias[4] = {0.00923f, 0.00923f, 2.0f, 0.0f};
        DAT_10f1fcf0->SetVertexShaderConstantF(6, bias, 1);
        toonMatrix = Transpose(*world * *viewProjection);
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_MATRIX, toonMatrix.m, 4);
        float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        DAT_10f1fcf0->SetVertexShaderConstantF(MESH_REGISTER_TINT, white, 1);
        if (DAT_10202b1c == 0)
            previousBlend = FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
    }
    DAT_10f1fcf0->SetVertexDeclaration(DAT_10f654ac);
    FUN_1013ef30(0x8d, 1);
    DAT_10f1fcf0->SetVertexShader(DAT_10f0ee00[0][shader]);
    FUN_1013ef30(0x91, 1);
    DAT_10f0d5c8++;
    DAT_10f1fcf0->DrawIndexedPrimitiveUP(MESH_TRIANGLE_LIST, 0, vertexCount, indexCount / 3, indices,
                                         MESH_INDEX_FORMAT_16, vertices, MESH_VERTEX_STRIDE);
    if (previousBlend != -1)
        FUN_1013f070(RENDER_BLEND_MODE, previousBlend, 0);
    return 1;
}
