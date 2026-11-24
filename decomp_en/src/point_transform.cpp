// Transforming a point by a matrix (EN build). Ported from the matched JP source decomp/src/point_transform.cpp.
#include "point_transform.h"

// Point_Transform: transforms `point` by `matrix`: the point is rotated by the matrix's rotation part (as the translation
// of a matrix multiplied by it) and moved by the matrix's translation. Returns 0.
int Point_Transform(Vec4 *point, Matrix4 *matrix)
{
    Matrix4 placed;
    Matrix4 rotation;
    placed.SetTranslation(*point);
    rotation = *matrix;
    rotation.SetTranslation(Vec4(0, 0, 0, 0));
    placed = placed * rotation;
    point->Copy(placed.GetTranslation() + matrix->GetTranslation());
    return 0;
}
