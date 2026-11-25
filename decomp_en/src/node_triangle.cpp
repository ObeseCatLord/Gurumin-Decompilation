// Ray-triangle test of the scene nodes (EN build). Ported from the matched JP source decomp/src/node_triangle.cpp.
#include "node.h"
#include "plane.h"

// A determinant below this means the ray runs parallel to the triangle (or hits its back).
const float PARALLEL_EPSILON = 0.0001f;

// Node::IntersectTriangle: ray-triangle test used by the segment and ray tests: the ray starts at `origin` and runs along
// `direction`; the triangle is a-b-c (front face only). On a hit, *hitT is the distance along the ray in units of
// `direction`, the hit point is written into `a` (x, y, z), and *texU / *texV are the texture coordinates at the hit
// point, interpolated from the corners' (x, y of uvA..uvC); returns 1. Returns 0 on a miss (the outputs may then hold
// the barycentric coordinates reached so far).
int Node::IntersectTriangle(const Vec4 *origin, const Vec4 *direction, Vec4 *a, Vec4 *b, const Vec4 *c, float *hitT,
                            float *texU, float *texV, const Vec4 *uvA, const Vec4 *uvB, const Vec4 *uvC)
{
    Vec4 edgeAB;
    edgeAB.x = b->x - a->x;
    edgeAB.y = b->y - a->y;
    edgeAB.z = b->z - a->z;
    Vec4 edgeAC;
    edgeAC.x = c->x - a->x;
    edgeAC.y = c->y - a->y;
    edgeAC.z = c->z - a->z;
    Vec4 directionCrossAC;
    directionCrossAC.Copy(Cross(*direction, edgeAC));
    float determinant = Vec3_Dot(&edgeAB, &directionCrossAC);
    if (determinant < PARALLEL_EPSILON)
        return 0;

    // Barycentric coordinates (scaled by the determinant until the end). The tests are written as "not outside"
    // (a NaN coordinate passes), as the original compares.
    Vec4 aToOrigin;
    aToOrigin.x = origin->x - a->x;
    aToOrigin.y = origin->y - a->y;
    aToOrigin.z = origin->z - a->z;
    *texU = Vec3_Dot(&aToOrigin, &directionCrossAC);
    if (!(*texU < 0.0f) && !(*texU > determinant))
    {
        Vec4 originCrossAB;
        originCrossAB.Copy(Cross(aToOrigin, edgeAB));
        *texV = Vec3_Dot(direction, &originCrossAB);
        if (!(*texV < 0.0f) && !(*texV + *texU > determinant))
        {
            float inverse = 1.0f / determinant;
            *hitT = Vec3_Dot(&edgeAC, &originCrossAB) * inverse;
            *texU = inverse * *texU;
            *texV = inverse * *texV;

            // Texture coordinates: the line from c through the hit point meets edge a-b at `edgePoint`; interpolate
            // along a-b by how far edgePoint lies from a, then from c towards edgePoint by how far the hit point lies
            // from c.
            Vec4 hit;
            float uA = uvA->x;
            float vA = uvA->y;
            float uB = uvB->x;
            float vB = uvB->y;
            float uC = uvC->x;
            float vC = uvC->y;
            Vec4 edgePoint;
            Vec4 normalTip;  // a + the triangle's normal: with a and b it spans the plane through a-b upright on it
            Plane plane = PlaneFromTriangle(*a, *b, *c);
            plane.IntersectRay(&hit, origin, direction);
            Vec4 hitToC;
            hitToC.x = c->x - hit.x;
            hitToC.y = c->y - hit.y;
            hitToC.z = c->z - hit.z;
            normalTip.Copy(TriangleNormal(*a, *b, *c));
            normalTip.x += a->x;
            normalTip.y += a->y;
            normalTip.z += a->z;
            plane = PlaneFromTriangle(*a, *b, normalTip);
            plane.IntersectRay(&edgePoint, c, &hitToC);
            float alongAB = edgePoint.Distance(*a) / b->Distance(*a);
            float alongCToEdge = hit.Distance(*c) / edgePoint.Distance(*c);
            float edgeU = (uB - uA) * alongAB + uA;
            float edgeV = (vB - vA) * alongAB + vA;
            float hitU = (edgeU - uC) * alongCToEdge + uC;
            float hitV = (edgeV - vC) * alongCToEdge + vC;
            a->x = hit.x;
            a->y = hit.y;
            a->z = hit.z;
            *texU = hitU;
            *texV = hitV;
            return 1;
        }
    }
    return 0;
}
