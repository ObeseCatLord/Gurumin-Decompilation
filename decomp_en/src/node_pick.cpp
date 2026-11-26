// Picking nodes with a screen point (EN build, EN-only: the JP twins only have the segment test). The point is turned into a
// ray through the camera and tested against the boxes of the nodes of a tree.
#include "containers.h"
#include "node.h"
#include "plane.h"
#include "render.h"
#include "vec4.h"

// Triangles of a node's box (two per face) and the vertices the corner array has room for.
const int PICK_BOX_TRIANGLE_COUNT = 12;
const int PICK_BOX_VERTEX_ROOM = 24;
// The ray of the screen point leaves the camera along the view axis: normalised z of the point.
const float PICK_NEAR_DEPTH = 1.0f;
// Normalised device coordinates run from -1 to 1 across the screen: 2 * (position / size) - 1.
const float PICK_NDC_SCALE = 2.0f;
const float PICK_NDC_ORIGIN = 1.0f;

// Node::PickBox: finds the node of this tree whose box the screen point (screenX, screenY) lies over. The point is turned
// into a ray with the projection's x and y scale (`projection`) and the inverse of the camera matrix (`camera`); then each
// of the node's box triangles (the corners moved by the world matrix) is tested against the ray, front faces only, and the
// first hit point goes to *hitX, *hitY and *hitZ. Without a hit the children are searched in turn. Returns the node that
// was hit, or 0. Only the first call (`firstCall` = 1) takes the ray from the camera position; the calls for the children
// use the camera's axes. `unusedA` and `unusedB` are only passed on to the children.
Node *Node::PickBox(float *hitX, float *hitY, float *hitZ, float screenX, float screenY, const Matrix4 *projection,
                    const Matrix4 *camera, int unusedA, int unusedB, int firstCall)
{
    Vec4 rayDirection;
    Vec4 rayOrigin;
    Vec4 ndcPoint;
    ndcPoint.z = PICK_NEAR_DEPTH;
    ndcPoint.x = ((screenX * PICK_NDC_SCALE) / (float)g_deviceBackBufferWidth - PICK_NDC_ORIGIN) / projection->m[0];
    ndcPoint.y = -(((screenY * PICK_NDC_SCALE) / (float)g_deviceBackBufferHeight - PICK_NDC_ORIGIN) / projection->m[5]);
    Matrix4 inverse;
    inverse = InverseMatrix(*camera);
    if (firstCall == 1)
    {
        rayDirection.x = inverse.m[MATRIX_TRANSLATION_X];
        rayDirection.y = inverse.m[MATRIX_TRANSLATION_Y];
        rayDirection.z = inverse.m[MATRIX_TRANSLATION_Z];
        rayOrigin.Copy(Project(ndcPoint, inverse));
    }
    else
    {
        rayDirection.x = inverse.m[4] * ndcPoint.y + inverse.m[0] * ndcPoint.x + inverse.m[8] * ndcPoint.z;
        rayDirection.y = inverse.m[5] * ndcPoint.y + inverse.m[1] * ndcPoint.x + inverse.m[9] * ndcPoint.z;
        rayDirection.z = inverse.m[6] * ndcPoint.y + inverse.m[2] * ndcPoint.x + inverse.m[10] * ndcPoint.z;
        rayOrigin.x = inverse.m[MATRIX_TRANSLATION_X];
        rayOrigin.y = inverse.m[MATRIX_TRANSLATION_Y];
        rayOrigin.z = inverse.m[MATRIX_TRANSLATION_Z];
    }
    Vec4 cornerA;
    Vec4 cornerB;
    Vec4 cornerC;
    Vec4 worldA;
    Vec4 worldB;
    Vec4 worldC;
    Vec4 uvA;
    Vec4 uvB;
    Vec4 uvC;
    float hitT;
    float texU;
    float texV;
    Vertex corners[PICK_BOX_VERTEX_ROOM];
    corners[0].SetPosition(box[NODE_BOX_MIN].x, box[NODE_BOX_MIN].y, box[NODE_BOX_MIN].z);
    corners[1].SetPosition(box[NODE_BOX_MAX].x, box[NODE_BOX_MIN].y, box[NODE_BOX_MIN].z);
    corners[2].SetPosition(box[NODE_BOX_MAX].x, box[NODE_BOX_MAX].y, box[NODE_BOX_MIN].z);
    corners[3].SetPosition(box[NODE_BOX_MIN].x, box[NODE_BOX_MAX].y, box[NODE_BOX_MIN].z);
    corners[4].SetPosition(box[NODE_BOX_MIN].x, box[NODE_BOX_MIN].y, box[NODE_BOX_MAX].z);
    corners[5].SetPosition(box[NODE_BOX_MAX].x, box[NODE_BOX_MIN].y, box[NODE_BOX_MAX].z);
    corners[6].SetPosition(box[NODE_BOX_MAX].x, box[NODE_BOX_MAX].y, box[NODE_BOX_MAX].z);
    corners[7].SetPosition(box[NODE_BOX_MIN].x, box[NODE_BOX_MAX].y, box[NODE_BOX_MAX].z);
    // The corners of each triangle: bottom (z min), the four sides, top.
    unsigned short triangles[PICK_BOX_TRIANGLE_COUNT * 3] = {
        0, 1, 2,  0, 2, 3,
        3, 2, 6,  3, 6, 7,
        7, 6, 5,  7, 5, 4,
        4, 5, 1,  4, 1, 0,
        0, 3, 7,  0, 7, 4,
        2, 1, 5,  2, 5, 6
    };
    for (int i = 0; i < PICK_BOX_TRIANGLE_COUNT; i++)
    {
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
        worldA.Copy(Project(cornerA, worldMatrix));
        worldB.Copy(Project(cornerB, worldMatrix));
        worldC.Copy(Project(cornerC, worldMatrix));
        float facing = Vec3_Dot(&TriangleNormal(worldA, worldB, worldC), &rayDirection);
        if (facing >= 0.0f &&
            IntersectTriangle(&rayOrigin, &rayDirection, &worldA, &worldB, &worldC, &hitT, &texU, &texV, &uvA, &uvB, &uvC))
        {
            *hitX = worldA.x;
            *hitY = worldA.y;
            *hitZ = worldA.z;
            return this;
        }
    }
    for (int child = 0; child < childCount; child++)
    {
        Node *picked = children[child]->PickBox(hitX, hitY, hitZ, screenX, screenY, projection, camera, unusedA, unusedB, 0);
        if (picked != 0)
            return picked;
    }
    return 0;
}
