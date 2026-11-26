// Debug view of a mesh's bounding box (EN build): the box as a solid, then the three coordinate axes with arrow
// heads at the box's position. Ported from the matched JP source decomp/src/mesh_debug_box.cpp.
#include "slot_table.h"
#include "render.h"
#include "containers.h"

// Vertex shader constant registers the debug drawing fills.
const int SHADER_CONSTANT_WORLD = 0;   // world matrix (first three rows) and, for the axes, world x view (four rows)
const int SHADER_CONSTANT_TINT = 4;    // colour tint of the debug shapes
const int SHADER_CONSTANT_VIEW = 8;    // view matrix (four rows)
const int WORLD_ROWS = 3;              // rows of the world matrix the shader reads
const int VIEW_ROWS = 4;               // rows of the view matrix (and of the combined matrix of the axes)
const int TINT_REGISTERS = 1;          // registers the tint takes

// Render state number that is not in the D3D9 headers; the debug drawing clears it first.
const unsigned long RENDER_STATE_DEBUG_UNKNOWN = 0x89;

// Vertices and triangles of the box, and the size of the vertex array shared by box and axes.
const int BOX_VERTEX_COUNT = 8;
const int BOX_TRIANGLE_COUNT = 12;
const int AXES_VERTEX_COUNT = 18;
const int DEBUG_VERTEX_CAPACITY = 24;
// Line segments of the three axes (one shaft and four arrow-head lines each).
const int AXES_LINE_COUNT = 15;
// Axis geometry: shaft length, where the arrow head starts, and how far its lines spread.
const float AXIS_LENGTH = 50.0f;
const float ARROW_START = 45.0f;
const float ARROW_SPREAD = 5.0f;

// Index of the vertex shader handle in g_drawListBuffers[0] (EN 0x021e3634).
const int DEBUG_SHADER_BUFFER = 5;

// Tint of the debug shapes (written to SHADER_CONSTANT_TINT).
static const float DEBUG_TINT[4] = {0.0f, 0.1f, 0.5f, 0.8f};

// Draws the box `box` (smallest corner, largest corner) as 12 triangles with the world matrix `world`, then three
// 50-unit axis lines (red x, green y, blue z) with arrow heads at the world position, through the view matrix. Does
// nothing without a texture slot or when the scene cannot be drawn. Returns 1 when it drew, otherwise 0. Called by
// the node debug drawing.
int SlotTable::DrawDebugBox(Matrix4 *world, const Matrix4 *view, const Vec4 *box)
{
    int drawn = 0;
    if (textureSlot >= 0 && g_sceneCanDraw != 0)
    {
        Stage0_Unbind();
        Matrix4 transform;
        float tint[4] = {DEBUG_TINT[0], DEBUG_TINT[1], DEBUG_TINT[2], DEBUG_TINT[3]};
        g_d3dDevice->SetVertexShaderConstantF(SHADER_CONSTANT_TINT, tint, TINT_REGISTERS);
        transform = Transpose(*world);
        g_d3dDevice->SetVertexShaderConstantF(SHADER_CONSTANT_WORLD, (const float *)&transform, WORLD_ROWS);
        transform = Transpose(*view);
        g_d3dDevice->SetVertexShaderConstantF(SHADER_CONSTANT_VIEW, (const float *)&transform, VIEW_ROWS);
        g_d3dDevice->SetVertexShader((IDirect3DVertexShader9 *)g_drawListBuffers[0][DEBUG_SHADER_BUFFER]);

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
        if (g_sceneCanDraw != 0)
            g_d3dDevice->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, BOX_VERTEX_COUNT, BOX_TRIANGLE_COUNT, indices,
                                                D3DFMT_INDEX16, vertices, sizeof(Vertex));

        transform = IdentityMatrix();
        transform.SetTranslation(world->GetTranslation());
        transform = Transpose(transform * *view);
        g_d3dDevice->SetVertexShaderConstantF(SHADER_CONSTANT_WORLD, (const float *)&transform, VIEW_ROWS);
        Render_SetRenderState(RENDER_STATE_DEBUG_UNKNOWN, 0);
        Render_SetRenderState(D3DRS_ZENABLE, 0);

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
        if (g_sceneCanDraw != 0)
            g_d3dDevice->DrawIndexedPrimitiveUP(D3DPT_LINELIST, 0, AXES_VERTEX_COUNT, AXES_LINE_COUNT, indices, D3DFMT_INDEX16,
                                                vertices, sizeof(Vertex));
        Render_SetRenderState(D3DRS_ZENABLE, 1);
        drawn = 1;
    }
    return drawn;
}
