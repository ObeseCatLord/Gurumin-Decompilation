// BoneRows: declared here once (first used by src/mesh_release.cpp).
#ifndef GURUMIN_BONE_ROWS_H
#define GURUMIN_BONE_ROWS_H

#include "matrix4.h"

// The first three rows of a transposed bone matrix: one vertex-shader constant each.
struct BoneRows
{
    Vec4 row[3];
};

#endif
