// Transforming a point by a matrix through matrix products (the engine's way, with matrix temporaries).

#include "vec4.h"

#include "matrix4.h"

// 0x1001c510. Transforms `point` by `matrix`: the point is rotated by the matrix's rotation part
// (as the translation of a matrix multiplied by it) and moved by the matrix's translation. Returns 0.
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
