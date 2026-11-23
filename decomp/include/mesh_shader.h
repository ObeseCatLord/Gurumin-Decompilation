// Vertex-shader pass modes of TextureObject::DrawMesh (src/texture_mesh.cpp) and the two switches it reads.
// This is the one declaration of these names: include this header, never redeclare them.
#ifndef GURUMIN_MESH_SHADER_H
#define GURUMIN_MESH_SHADER_H

// `shader` values DrawMesh sets constants for; the value also indexes the vertex shader table DAT_10f0ee00[0].
enum MeshShaderMode
{
    MESH_SHADER_TEXTURED = 5,    // view-projection matrix, tint in register 0x5f
    MESH_SHADER_OUTLINE = 0x1a,  // world and view-projection matrices, camera position, fog; outline width 0.001
    MESH_SHADER_TOON = 0x24,     // view-projection matrix and fixed lighting constants; additive blend
    MESH_SHADER_SHADED = 0x2d    // view-projection matrix, tint in register 0x5f and extra in register 0x5e
};

extern "C" int DAT_10202b10;   // non-zero: the outline pass sets the fog colour from the scene fog (DAT_10f668f8)
extern "C" int DAT_10202b1c;   // zero: the toon pass switches to additive blending while it draws

#endif
