// Drawing a mesh with the shadow shader: the shader constants are set from the matrices and the shadow parameters,
// the buffers bound, and the packs drawn range by range with the packs that face away from the viewer left out.

#include "d3d9_min.h"

#include "vec4.h"

#include "matrix4.h"

#include "mesh.h"

#include "render_vars.h"

#include "render_state.h"

#include "camera_vars.h"

#include "frame_vars.h"

#include "mesh_draw_vars.h"

extern "C" int FUN_10147a20(int slot, IUnknownLike **vertexBuffer, IUnknownLike **indexBuffer);   // GetMeshBuffers (mesh_buffers.cpp)

#define D3DPT_TRIANGLELIST_ 4
#define ALPHA_TEST_REFERENCE 15
#define SHADER_CONSTANT_VIEW 0
#define SHADER_CONSTANT_ONE 4
#define SHADER_CONSTANT_SIZE 5
#define SHADER_CONSTANT_SHADOW_POSITION 6
#define SHADER_CONSTANT_LIGHT_DIRECTION 7
#define SHADER_CONSTANT_WORLD 8
#define SHADER_CONSTANT_WORLD_VIEW 12
#define SHADER_CONSTANT_SHADOW_MATRIX 0x5c
#define SHADER_CONSTANT_FADE 0x5b
#define SHADOW_SIZE_UNIT 200.0f
#define SHADOW_HEIGHT_OFFSET 50.0f
#define VIEW_AHEAD_DISTANCE 300.0f
#define FACING_LIMIT 0.85f
#define VERTEX_STRIDE 0x70
#define TRIANGLE_VERTICES 3
#define BOX_VECTOR_COUNT 9
#define BOX_SIZE 5
#define BOX_CENTRE 6
#define BOX_TO_VIEW 7
#define BOX_DIRECTION 8

// 0x10151150 (MeshData::Draw). Sets the render states and the shader constants of the shadow shader (view, world,
// world * view and shadow matrices, the light direction, the shadow position raised by 50, the shadow size and the
// fade 1 - alpha), binds the mesh buffers and the shadow texture, and draws the packs from the first one of the
// mesh's range. A pack with a bounding sphere is merged with the packs after it that share its materials and edges,
// and a group whose three sample points on the box all face away from the viewer (dot product below 0.85, or the
// sphere too far from the shadow position) is left out and counted in DAT_10f623bc. Returns 0.
int MeshData::Draw(Matrix4 *worldMatrix, const Matrix4 *view, const Matrix4 *shadowMatrix, int drawMode, float shadowX,
                   float shadowY, float shadowZ, int shadowTexture, float shadowScale, int shadowSize,
                   const Vec4 *lightDirection, float modelSize, float alpha)
{
    FUN_1013f070(RENDER_ALPHA_TEST, 1, ALPHA_TEST_REFERENCE);
    if (textureSlot >= 0)
    {
        FUN_1013ef90(0, 4, 2);
        FUN_1013ef90(0, 5, 2);
        FUN_1013ef90(0, 6, 0);
        FUN_1013ef90(0, 1, 2);
        FUN_1013ef90(0, 2, 2);
        FUN_1013ef90(0, 3, 0);
        FUN_1013ef90(1, 4, 1);
        FUN_1013ef90(1, 1, 1);

        Matrix4 transform;
        transform = Transpose(*shadowMatrix);
        DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_SHADOW_MATRIX, (const float *)&transform, 4);
        transform = Transpose(*view);
        DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_VIEW, (const float *)&transform, 4);
        transform = Transpose(*worldMatrix);
        DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_WORLD, (const float *)&transform, 3);
        transform = Transpose(*worldMatrix * *view);
        DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_WORLD_VIEW, (const float *)&transform, 4);

        {
            float one[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_ONE, one, 1);
        }
        {
            float fade = 1.0f - alpha;
            float fadeColour[4] = {fade, fade, fade, 0.0f};
            DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_FADE, fadeColour, 1);
        }
        float sizeValue = (float)shadowSize;
        float size = sizeValue / SHADOW_SIZE_UNIT;
        size = size < 1.0f ? 1.0f : size;
        float sizeConstant[4];
        sizeConstant[0] = size * 0.1f;
        sizeConstant[2] = -0.1f;
        sizeConstant[0] = sizeConstant[0] + 0.43f;
        sizeConstant[3] = 0.0f;
        sizeConstant[1] = shadowScale * 0.000004f;
        DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_SIZE, sizeConstant, 1);
        {
            float light[4] = {lightDirection->x, lightDirection->y, lightDirection->z, 1.0f};
            DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_LIGHT_DIRECTION, light, 1);
        }
        {
            float shadowPosition[4] = {shadowX, shadowY, shadowZ + SHADOW_HEIGHT_OFFSET, 0.0f};
            DAT_10f1fcf0->SetVertexShaderConstantF(SHADER_CONSTANT_SHADOW_POSITION, shadowPosition, 1);
        }

        IUnknownLike *vertexBuffer = 0;
        IUnknownLike *indexBuffer = 0;
        if (FUN_10147a20(textureSlot, &vertexBuffer, &indexBuffer) != 0)
        {
            DAT_10f1fcf0->SetIndices(indexBuffer);
            DAT_10f1fcf0->SetVertexShader(DAT_10f0ee1c[DAT_10f65544].shader);
            DAT_10f1fcf0->SetVertexDeclaration(DAT_10f654ac);
            DAT_10f1fcf0->SetStreamSource(DAT_10f65544, vertexBuffer, 0, VERTEX_STRIDE);
            DAT_10f65544++;
            if (DAT_10f65544 >= 1)
                DAT_10f65544 = 0;
            FUN_10161070(shadowTexture);

            Vec4 shadowPoint;
            Vec4 worldPosition;
            shadowPoint.x = shadowX;
            shadowPoint.y = shadowY;
            shadowPoint.z = shadowZ;
            Vec4 boxVectors[BOX_VECTOR_COUNT];
            worldPosition.Copy(worldMatrix->GetTranslation());
            Vec4 viewDirection;
            Vec4 viewPoint;
            viewDirection.Copy(DAT_10f668e8 - DAT_10f65688);
            viewDirection.Normalized3();
            float ahead = VIEW_AHEAD_DISTANCE;
            viewPoint.Copy(DAT_10f65688 - worldPosition - viewDirection * ahead);

            int skipped = 0;
            for (int pack = m_003c + DAT_10f655b8; pack < m_0044 && pack < packCount; pack++)
            {
                int lastPack = m_0044 - 1;
                int vertexStart = vertexStarts[pack];
                int vertexTotal = vertexEnds[pack] - vertexStart;
                int last = pack;
                if (vertexTotal <= 0)
                {
                    pack = last + skipped;
                    continue;
                }
                if (pointFlags[pack] == 0)
                {
                    int indexStart = indexStarts[pack];
                    DAT_10f1fcf0->DrawIndexedPrimitive(D3DPT_TRIANGLELIST_, 0, vertexStart, vertexTotal, indexStart,
                                                       (indexEnds[pack] - indexStart) / TRIANGLE_VERTICES);
                    DAT_10f623c8 += vertexEnds[pack] - vertexStarts[pack];
                    pack = last + skipped;
                    continue;
                }
                skipped = 0;
                for (;;)
                {
                    int culled = 0;
                    if (points[last].w > 0.0f)
                    {
                        boxVectors[BOX_CENTRE].Copy((boxes[last * 2] + boxes[last * 2 + 1]) * 0.5f + worldPosition);
                        boxVectors[BOX_SIZE].Copy(boxes[last * 2 + 1] - boxes[last * 2]);
                        double nearness = boxVectors[BOX_CENTRE].DistanceSquared(shadowPoint) * shadowScale;
                        if (nearness > boxVectors[BOX_SIZE].LengthSquared3() * 0.5)
                            culled = 1;
                        else
                        {
                            boxVectors[BOX_TO_VIEW].Copy((boxes[last * 2] + boxes[last * 2 + 1]) * 0.5f - viewPoint);
                            boxVectors[BOX_DIRECTION].Copy(Vec3_Normalized(&boxVectors[BOX_TO_VIEW]));
                            if (boxVectors[BOX_DIRECTION].x * viewDirection.x + boxVectors[BOX_DIRECTION].y * viewDirection.y +
                                    boxVectors[BOX_DIRECTION].z * viewDirection.z < FACING_LIMIT)
                            {
                                boxVectors[BOX_TO_VIEW].Copy((boxes[last * 2] + boxes[last * 2 + 1]) * 0.5f - viewPoint);
                                boxVectors[BOX_TO_VIEW].z = boxVectors[BOX_TO_VIEW].z - (boxes[last * 2 + 1].z - boxes[last * 2].z) * 0.5f;
                                boxVectors[BOX_DIRECTION].Copy(Vec3_Normalized(&boxVectors[BOX_TO_VIEW]));
                                if (boxVectors[BOX_DIRECTION].x * viewDirection.x + boxVectors[BOX_DIRECTION].y * viewDirection.y +
                                        boxVectors[BOX_DIRECTION].z * viewDirection.z < FACING_LIMIT)
                                {
                                    boxVectors[BOX_TO_VIEW].Copy((boxes[last * 2] + boxes[last * 2 + 1]) * 0.5f - viewPoint);
                                    boxVectors[BOX_TO_VIEW].z = (boxes[last * 2 + 1].z - boxes[last * 2].z) * 0.5f + boxVectors[BOX_TO_VIEW].z;
                                    boxVectors[BOX_DIRECTION].Copy(Vec3_Normalized(&boxVectors[BOX_TO_VIEW]));
                                    if (boxVectors[BOX_DIRECTION].x * viewDirection.x + boxVectors[BOX_DIRECTION].y * viewDirection.y +
                                            boxVectors[BOX_DIRECTION].z * viewDirection.z < FACING_LIMIT)
                                        culled = 1;
                                }
                            }
                        }
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
                    if (last + 1 > lastPack || materials[last] != materials[last + 1] ||
                        doubleSide[last] != doubleSide[last + 1])
                        break;
                    last++;
                }
                {
                    int indexStart = indexStarts[pack];
                    DAT_10f1fcf0->DrawIndexedPrimitive(D3DPT_TRIANGLELIST_, 0, vertexStarts[pack],
                                                       vertexEnds[last] - vertexStarts[pack], indexStart,
                                                       (indexEnds[last] - indexStart) / TRIANGLE_VERTICES);
                    DAT_10f623c8 += vertexEnds[pack] - vertexStarts[pack];
                    pack = last + skipped;
                }
            nextPack:;
            }
            FUN_1013ef90(0, 4, 4);
        }
    }
    return 0;
}
