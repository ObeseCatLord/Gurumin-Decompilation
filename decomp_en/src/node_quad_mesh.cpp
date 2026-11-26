// Four-vertex mesh of a node (EN build, EN-only: no JP twin found).
#include "containers.h"
#include "node.h"

// Vertices and indices of the mesh BuildQuadMesh makes.
const int QUAD_VERTEX_COUNT = 4;
const int QUAD_INDEX_COUNT = 12;

// Node::BuildQuadMesh: copies the four vertices and gives the node a mesh built from them, using the index list
// 0 1 2 3, 1 2 3 0, 2 3 0 1 (each run of four indices starts one vertex later). No direct caller recovered.
int Node::BuildQuadMesh(const Vertex *first, const Vertex *second, const Vertex *third, const Vertex *fourth)
{
    Vertex quad[QUAD_VERTEX_COUNT];
    int indices[QUAD_INDEX_COUNT] = {0, 1, 2, 3, 1, 2, 3, 0, 2, 3, 0, 1};
    quad[0] = *first;
    quad[1] = *second;
    quad[2] = *third;
    quad[3] = *fourth;
    return BuildMeshFromVertices(quad, indices, QUAD_VERTEX_COUNT, QUAD_INDEX_COUNT);
}
