// Planar texture coordinates of a node's mesh (EN build; JP Node::ProjectPlanarUVs 0x1019bdd0, itself an open match).
#include <math.h>
#include "containers.h"
#include "index_list.h"
#include "node.h"

// Scale from model units to texture coordinates (one texture repeat per 33 units).
const float PLANAR_UV_SCALE = 0.03f;

// Index into Vertex::position of each axis.
enum VertexAxis
{
    AXIS_X = 0,
    AXIS_Y = 1,
    AXIS_Z = 2
};

// Node::ProjectPlanarUVs: gives every triangle of the packs that use material `material` planar texture coordinates.
// The sum of the triangle's three vertex normals picks the axis it faces most; the vertex positions on the other two
// axes, times PLANAR_UV_SCALE, become u and v (facing x: z and y, facing y: x and z, facing z: x and y). Returns 1.
// Called by the model loader (chunk parser) for materials flagged for planar mapping.
int Node::ProjectPlanarUVs(int material)
{
    Vec4 a;
    Vec4 b;
    Vec4 c;
    Vec4 normal;
    Vec4 axisX;
    Vec4 axisY;
    Vec4 axisZ;
    axisX.Set(1.0f, 0, 0, 0);
    axisY.Set(0, 1.0f, 0, 0);
    axisZ.Set(0, 0, 1.0f, 0);
    for (int p = 0; p < packCount; p++)
    {
        if (packs[p].material != material)
            continue;
        for (int i = 0; i < indexLists[p].count; i += 3)
        {
            int i0 = indexLists[p].indices[i];
            int i1 = indexLists[p].indices[i + 1];
            int i2 = indexLists[p].indices[i + 2];
            a.x = packs[p].vertices[i0].normal[0];
            a.y = packs[p].vertices[i0].normal[1];
            a.z = packs[p].vertices[i0].normal[2];
            b.x = packs[p].vertices[i1].normal[0];
            b.y = packs[p].vertices[i1].normal[1];
            b.z = packs[p].vertices[i1].normal[2];
            c.x = packs[p].vertices[i2].normal[0];
            c.y = packs[p].vertices[i2].normal[1];
            c.z = packs[p].vertices[i2].normal[2];
            normal.Copy(a + b + c);
            // `largest` starts as how much the triangle faces x.
            float largest = (float)fabs(Vec3_Dot(&normal, &axisX));
            float facingY = (float)fabs(Vec3_Dot(&normal, &axisY));
            float facingZ = (float)fabs(Vec3_Dot(&normal, &axisZ));
            int axis = AXIS_X;
            if (largest < facingY)
            {
                largest = facingY;
                axis = AXIS_Y;
            }
            if (largest < facingZ)
                axis = AXIS_Z;
            if (axis == AXIS_X)
            {
                packs[p].vertices[i0].uv[0] = packs[p].vertices[i0].position[2] * PLANAR_UV_SCALE;
                packs[p].vertices[i0].uv[1] = packs[p].vertices[i0].position[1] * PLANAR_UV_SCALE;
                packs[p].vertices[i1].uv[0] = packs[p].vertices[i1].position[2] * PLANAR_UV_SCALE;
                packs[p].vertices[i1].uv[1] = packs[p].vertices[i1].position[1] * PLANAR_UV_SCALE;
                packs[p].vertices[i2].uv[0] = packs[p].vertices[i2].position[2] * PLANAR_UV_SCALE;
                packs[p].vertices[i2].uv[1] = packs[p].vertices[i2].position[1] * PLANAR_UV_SCALE;
            }
            else if (axis == AXIS_Y)
            {
                packs[p].vertices[i0].uv[0] = packs[p].vertices[i0].position[0] * PLANAR_UV_SCALE;
                packs[p].vertices[i0].uv[1] = packs[p].vertices[i0].position[2] * PLANAR_UV_SCALE;
                packs[p].vertices[i1].uv[0] = packs[p].vertices[i1].position[0] * PLANAR_UV_SCALE;
                packs[p].vertices[i1].uv[1] = packs[p].vertices[i1].position[2] * PLANAR_UV_SCALE;
                packs[p].vertices[i2].uv[0] = packs[p].vertices[i2].position[0] * PLANAR_UV_SCALE;
                packs[p].vertices[i2].uv[1] = packs[p].vertices[i2].position[2] * PLANAR_UV_SCALE;
            }
            else
            {
                packs[p].vertices[i0].uv[0] = packs[p].vertices[i0].position[0] * PLANAR_UV_SCALE;
                packs[p].vertices[i0].uv[1] = packs[p].vertices[i0].position[1] * PLANAR_UV_SCALE;
                packs[p].vertices[i1].uv[0] = packs[p].vertices[i1].position[0] * PLANAR_UV_SCALE;
                packs[p].vertices[i1].uv[1] = packs[p].vertices[i1].position[1] * PLANAR_UV_SCALE;
                packs[p].vertices[i2].uv[0] = packs[p].vertices[i2].position[0] * PLANAR_UV_SCALE;
                packs[p].vertices[i2].uv[1] = packs[p].vertices[i2].position[1] * PLANAR_UV_SCALE;
            }
        }
    }
    return 1;
}
