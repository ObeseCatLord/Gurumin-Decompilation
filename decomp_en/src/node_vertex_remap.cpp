// Remapping the per-vertex codes of a node's packs (EN build, EN-only: no JP twin found).
#include "containers.h"
#include "node.h"

// A non-zero vertex code v stands for the number k = (v - base) / -step (a whole number held as a float); the remapped
// code is base + step * map[-k].
const float VERTEX_CODE_BASE = 3.0f;
const float VERTEX_CODE_NEGATIVE_STEP = -3.0f;
const int VERTEX_CODE_STEP = 3;

// Node::RemapVertexCodes: replaces the first two not recovered values (m_0050[0] and m_0050[1]) of every vertex of every
// pack of the node that are not 0 by their remapped code through `codeMap`, which is indexed by the negated number the
// old code stands for. Returns 0. No direct caller recovered.
int Node::RemapVertexCodes(const int *codeMap)
{
    for (int pack = 0; pack < packCount; pack++)
    {
        for (int vertex = 0; vertex < packs[pack].vertexCount; vertex++)
        {
            float first = packs[pack].vertices[vertex].m_0050[0];
            if (first != 0)
                packs[pack].vertices[vertex].m_0050[0] =
                    (float)(codeMap[-(int)((first - VERTEX_CODE_BASE) / VERTEX_CODE_NEGATIVE_STEP)] * VERTEX_CODE_STEP) +
                    VERTEX_CODE_BASE;
            float second = packs[pack].vertices[vertex].m_0050[1];
            if (second != 0)
                packs[pack].vertices[vertex].m_0050[1] =
                    (float)(codeMap[-(int)((second - VERTEX_CODE_BASE) / VERTEX_CODE_NEGATIVE_STEP)] * VERTEX_CODE_STEP) +
                    VERTEX_CODE_BASE;
        }
    }
    return 0;
}
