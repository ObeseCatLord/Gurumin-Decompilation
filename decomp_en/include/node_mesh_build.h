// Globals of Node::BuildMesh (EN build; JP: DAT_11b04500 in decomp/include/render_vars.h). Declared once here.
#ifndef GURUMIN_EN_NODE_MESH_BUILD_H
#define GURUMIN_EN_NODE_MESH_BUILD_H

// Non-zero = Node::BuildMesh splits each new mesh into cells (Node::SplitIntoCells). EN 0x0218be68 (JP DAT_11b04500).
extern "C" int g_splitMeshIntoCells;

#endif
