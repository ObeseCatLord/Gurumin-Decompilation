// Segment against the boxes of a node tree: each node's mesh box, as twelve triangles in world space.

#include "vec4.h"
#include "matrix4.h"
#include "mesh.h"
#include "node.h"

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);   // 0x101ae060 (vec4.cpp): transformed, divided by w

// Corners of a box built from Node::box: bit 0 picks the x, bit 1 the y, bit 2 the z of the upper corner (with the
// x bit mirrored for corners 2, 3, 6 and 7 so that the corners run round each face).
enum BoxCorner
{
    BOX_CORNER_COUNT = 8,
    BOX_TRIANGLE_COUNT = 12,    // two per face
    BOX_VERTEX_ROOM = 24        // the corner array has room for four vertices per face; only 8 are used
};

// 0x101ab3e0 (Node::SegmentHitsBox). Called by ModelObject::QueryCollider (0x10164090). Tests the segment from
// `from` to `to` against the box of every node in this tree that has index lists and a box-collision flag
// (NODE_FLAG_SHAPE_BOX or NODE_FLAG_SHAPE_SPHERE): the box's twelve triangles, moved by the node's world matrix,
// are hit along the ray from `from` towards `to` (IntersectTriangle); the hit counts when it lies closer than the
// segment's length to both ends. Returns 1 on the first hit, else 0.
int Node::SegmentHitsBox(Vec4 *from, Vec4 *to)
{
    Vec4 backwards;   // from - to
    Vec4 origin;
    float length = from->Distance(*to);
    Vec4 direction;
    Vec4 cornerA;
    Vec4 cornerB;
    Vec4 cornerC;
    Vec4 hit;         // corner A in world space, then the hit point
    Vec4 worldB;
    Vec4 worldC;
    Vec4 uvA;
    Vec4 uvB;
    Vec4 uvC;

    if (indexLists != 0 && (flags & (NODE_FLAG_SHAPE_BOX | NODE_FLAG_SHAPE_SPHERE))) {
        backwards.Copy(*from - *to);
        origin.Copy(*from);
        direction.Copy(backwards * -1.0f);

        Vertex corners[BOX_VERTEX_ROOM];
        corners[0].SetPosition(box[NODE_BOX_MIN].x, box[NODE_BOX_MIN].y, box[NODE_BOX_MIN].z);
        corners[1].SetPosition(box[NODE_BOX_MAX].x, box[NODE_BOX_MIN].y, box[NODE_BOX_MIN].z);
        corners[2].SetPosition(box[NODE_BOX_MAX].x, box[NODE_BOX_MAX].y, box[NODE_BOX_MIN].z);
        corners[3].SetPosition(box[NODE_BOX_MIN].x, box[NODE_BOX_MAX].y, box[NODE_BOX_MIN].z);
        corners[4].SetPosition(box[NODE_BOX_MIN].x, box[NODE_BOX_MIN].y, box[NODE_BOX_MAX].z);
        corners[5].SetPosition(box[NODE_BOX_MAX].x, box[NODE_BOX_MIN].y, box[NODE_BOX_MAX].z);
        corners[6].SetPosition(box[NODE_BOX_MAX].x, box[NODE_BOX_MAX].y, box[NODE_BOX_MAX].z);
        corners[7].SetPosition(box[NODE_BOX_MIN].x, box[NODE_BOX_MAX].y, box[NODE_BOX_MAX].z);
        // The corners of each triangle: bottom (z min), the four sides, top.
        unsigned short triangles[BOX_TRIANGLE_COUNT * 3] = {
            0, 1, 2,  0, 2, 3,
            3, 2, 6,  3, 6, 7,
            7, 6, 5,  7, 5, 4,
            4, 5, 1,  4, 1, 0,
            0, 3, 7,  0, 7, 4,
            2, 1, 5,  2, 5, 6
        };
        for (int i = 0; i < BOX_TRIANGLE_COUNT; i++) {
            cornerA.x = corners[triangles[i * 3]].position[0];
            cornerA.y = corners[triangles[i * 3]].position[1];
            cornerA.z = corners[triangles[i * 3]].position[2];
            uvA.x = corners[triangles[i * 3]].uv[0];
            uvA.y = corners[triangles[i * 3]].uv[1];
            cornerB.x = corners[triangles[i * 3 + 1]].position[0];
            cornerB.y = corners[triangles[i * 3 + 1]].position[1];
            cornerB.z = corners[triangles[i * 3 + 1]].position[2];
            uvB.x = corners[triangles[i * 3 + 1]].uv[0];
            uvB.y = corners[triangles[i * 3 + 1]].uv[1];
            cornerC.x = corners[triangles[i * 3 + 2]].position[0];
            cornerC.y = corners[triangles[i * 3 + 2]].position[1];
            cornerC.z = corners[triangles[i * 3 + 2]].position[2];
            uvC.x = corners[triangles[i * 3 + 2]].uv[0];
            uvC.y = corners[triangles[i * 3 + 2]].uv[1];
            hit.Copy(Project(cornerA, worldMatrix));
            worldB.Copy(Project(cornerB, worldMatrix));
            worldC.Copy(Project(cornerC, worldMatrix));
            float hitT;
            float texU;
            float texV;
            if (IntersectTriangle(&origin, &direction, &hit, &worldB, &worldC, &hitT, &texU, &texV, &uvA, &uvB,
                                  &uvC) &&
                hit.Distance(*from) < length && hit.Distance(*to) < length)
                return 1;
        }
    }
    for (int i = 0; i < childCount; i++) {
        int result = children[i]->SegmentHitsBox(from, to);
        if (result != 0)
            return result;
    }
    return 0;
}
