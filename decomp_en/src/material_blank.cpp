// Blank-material setters (EN build): variants of Material::SetBlank (scene_node.cpp) that differ in which edge flags
// they touch. No caller is known for either.
#include "node.h"

// Material::SetBlankFirstEdge: makes the material a blank one (no name or textures, colour (r, g, b), alpha 0) and sets
// the first edge flag to `edgeFlag`; the other three edge flags keep their values.
void Material::SetBlankFirstEdge(float r, float g, float b, char edgeFlag)
{
    colour[0] = r;
    colour[1] = g;
    m_0174 = MATERIAL_BLANK_VALUE;
    colour[2] = b;
    textures[3][0] = 0;
    textures[2][0] = 0;
    textures[1][0] = 0;
    textures[0][0] = 0;
    name[0] = 0;
    m_0170 = 0;
    m_016c = 0;
    m_0168 = 0;
    m_0164 = 0;
    m_0160 = 0;
    m_015c = 0;
    m_0154 = 0;
    m_0150 = 0;
    m_014c = 0;
    alpha = MATERIAL_BLANK_ALPHA;
    edge[0] = edgeFlag;
}

// Material::SetBlankNoEdges: makes the material a blank one (no name or textures, colour (r, g, b), alpha 0) with all
// four edge flags cleared.
void Material::SetBlankNoEdges(float r, float g, float b)
{
    colour[0] = r;
    colour[1] = g;
    m_0174 = MATERIAL_BLANK_VALUE;
    colour[2] = b;
    textures[3][0] = 0;
    textures[2][0] = 0;
    textures[1][0] = 0;
    textures[0][0] = 0;
    name[0] = 0;
    edge[0] = 0;
    edge[1] = 0;
    edge[2] = 0;
    edge[3] = 0;
    m_0170 = 0;
    m_016c = 0;
    m_0168 = 0;
    m_0164 = 0;
    m_0160 = 0;
    m_015c = 0;
    m_0154 = 0;
    m_0150 = 0;
    m_014c = 0;
    alpha = MATERIAL_BLANK_ALPHA;
}
