// Debug view of a mesh's bounding box: the box as a solid, then the three coordinate axes with arrow heads at the
// box's position.

#include "d3d9_min.h"

#include "vec4.h"

#include "matrix4.h"

#include "mesh.h"

#include "render_vars.h"

#include "debug_draw_vars.h"

Matrix4 Transpose(const Matrix4 &matrix);   // 0x101b1090
Matrix4 IdentityMatrix();                   // 0x101b0ba0

extern "C" void FUN_10143b50(void);                   // Render_UnbindTexture0
extern "C" void FUN_1013ef30(int state, int value);   // Render_SetRenderState (cached)

#define D3DPT_LINELIST_ 2
#define D3DPT_TRIANGLELIST_ 4
#define D3DFMT_INDEX16_ 101
#define D3DRS_ZENABLE_ 7
#define D3DRS_UNKNOWN_89 0x89
#define BOX_VERTEX_COUNT 8
#define BOX_TRIANGLE_COUNT 12
#define AXES_VERTEX_COUNT 18
#define DEBUG_VERTEX_CAPACITY 24
#define AXES_LINE_COUNT 15
#define AXIS_LENGTH 50.0f
#define ARROW_START 45.0f
#define ARROW_SPREAD 5.0f
#define SHADER_CONSTANT_TINT 4
#define SHADER_CONSTANT_WORLD 0
#define SHADER_CONSTANT_VIEW 8

// 0x10146db0 (MeshData::DrawDebugBox). Draws the box `box` (smallest corner, largest corner) as 12 triangles with
// the world matrix `world`, then three 50-unit axis lines (red x, green y, blue z) with arrow heads at the world
// position, through the view matrix. Does nothing without a texture slot or when the scene is not ready. Returns 1
// when it drew, otherwise 0.
int MeshData::DrawDebugBox(Matrix4 *world, const Matrix4 *view, const Vec4 *box)
{
    int drawn;
    int cannotDraw = textureSlot < 0 || DAT_10f65584 == 0;
    if (cannotDraw)
    {
        drawn = 0;
    }
    else
    {
        FUN_10143b50();
        Matrix4 transform;
        float tint[4] = {0.0f, 0.1f, 0.5f, 0.8f};
        DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_TINT, tint, 1);
        transform = Transpose(*world);
        DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_WORLD, (const float *)&transform, 3);
        transform = Transpose(*view);
        DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_VIEW, (const float *)&transform, 4);
        DAT_10f1fcf0->SetVertexShader(DAT_10f0ee14);

        Vertex vertices[DEBUG_VERTEX_CAPACITY];
        vertices[0].SetPosition(box[0].x, box[0].y, box[0].z);
        vertices[1].SetPosition(box[1].x, box[0].y, box[0].z);
        vertices[2].SetPosition(box[1].x, box[1].y, box[0].z);
        vertices[3].SetPosition(box[0].x, box[1].y, box[0].z);
        vertices[4].SetPosition(box[0].x, box[0].y, box[1].z);
        vertices[5].SetPosition(box[1].x, box[0].y, box[1].z);
        vertices[6].SetPosition(box[1].x, box[1].y, box[1].z);
        vertices[7].SetPosition(box[0].x, box[1].y, box[1].z);

        unsigned short indices[BOX_TRIANGLE_COUNT * 3];
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
        indices[3] = 0;
        indices[4] = 2;
        indices[5] = 3;
        indices[6] = 3;
        indices[7] = 2;
        indices[8] = 6;
        indices[9] = 3;
        indices[10] = 6;
        indices[11] = 7;
        indices[12] = 7;
        indices[13] = 6;
        indices[14] = 5;
        indices[15] = 7;
        indices[16] = 5;
        indices[17] = 4;
        indices[18] = 4;
        indices[19] = 5;
        indices[20] = 1;
        indices[21] = 4;
        indices[22] = 1;
        indices[23] = 0;
        indices[24] = 0;
        indices[25] = 3;
        indices[26] = 7;
        indices[27] = 0;
        indices[28] = 7;
        indices[29] = 4;
        indices[30] = 2;
        indices[31] = 1;
        indices[32] = 5;
        indices[33] = 2;
        indices[34] = 5;
        indices[35] = 6;
        if (DAT_10f65584 != 0)
            DAT_10f1fcf0->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST_, 0, BOX_VERTEX_COUNT, BOX_TRIANGLE_COUNT, indices,
                                                 D3DFMT_INDEX16_, vertices, sizeof(Vertex));

        transform = IdentityMatrix();
        transform.SetTranslation(world->GetTranslation());
        transform = Transpose(transform * *view);
        DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_WORLD, (const float *)&transform, 4);
        FUN_1013ef30(D3DRS_UNKNOWN_89, 0);
        FUN_1013ef30(D3DRS_ZENABLE_, 0);

        vertices[0].Set(0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1);
        vertices[1].Set(AXIS_LENGTH, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1);
        vertices[2].Set(ARROW_START, ARROW_SPREAD, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1);
        vertices[3].Set(ARROW_START, -ARROW_SPREAD, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1);
        vertices[4].Set(ARROW_START, 0, ARROW_SPREAD, 0, 0, 0, 0, 0, 1, 0, 0, 1);
        vertices[5].Set(ARROW_START, 0, -ARROW_SPREAD, 0, 0, 0, 0, 0, 1, 0, 0, 1);
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 2;
        indices[3] = 1;
        indices[4] = 3;
        indices[5] = 1;
        indices[6] = 4;
        indices[7] = 1;
        indices[8] = 5;
        indices[9] = 1;
        vertices[6].Set(0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1);
        vertices[7].Set(0, AXIS_LENGTH, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1);
        vertices[8].Set(ARROW_SPREAD, ARROW_START, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1);
        vertices[9].Set(-ARROW_SPREAD, ARROW_START, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1);
        vertices[10].Set(0, ARROW_START, ARROW_SPREAD, 0, 0, 0, 0, 0, 0, 1, 0, 1);
        vertices[11].Set(0, ARROW_START, -ARROW_SPREAD, 0, 0, 0, 0, 0, 0, 1, 0, 1);
        indices[10] = 6;
        indices[11] = 7;
        indices[12] = 8;
        indices[13] = 7;
        indices[14] = 9;
        indices[15] = 7;
        indices[16] = 10;
        indices[17] = 7;
        indices[18] = 11;
        indices[19] = 7;
        vertices[12].Set(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1);
        vertices[13].Set(0, 0, AXIS_LENGTH, 0, 0, 0, 0, 0, 0, 0, 1, 1);
        vertices[14].Set(ARROW_SPREAD, 0, ARROW_START, 0, 0, 0, 0, 0, 0, 0, 1, 1);
        vertices[15].Set(-ARROW_SPREAD, 0, ARROW_START, 0, 0, 0, 0, 0, 0, 0, 1, 1);
        vertices[16].Set(0, ARROW_SPREAD, ARROW_START, 0, 0, 0, 0, 0, 0, 0, 1, 1);
        vertices[17].Set(0, -ARROW_SPREAD, ARROW_START, 0, 0, 0, 0, 0, 0, 0, 1, 1);
        indices[20] = 12;
        indices[21] = 13;
        indices[22] = 14;
        indices[23] = 13;
        indices[24] = 15;
        indices[25] = 13;
        indices[26] = 16;
        indices[27] = 13;
        indices[28] = 17;
        indices[29] = 13;
        if (DAT_10f65584 != 0)
            DAT_10f1fcf0->DrawIndexedPrimitiveUP(D3DPT_LINELIST_, 0, AXES_VERTEX_COUNT, AXES_LINE_COUNT, indices,
                                                 D3DFMT_INDEX16_, vertices, sizeof(Vertex));
        FUN_1013ef30(D3DRS_ZENABLE_, 1);
        drawn = 1;
    }
    return drawn;
}
