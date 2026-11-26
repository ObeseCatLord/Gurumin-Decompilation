// Rotation part (upper-left 3x3) helpers of Matrix4 (EN build, EN-only: no JP twin found).
#include "vec4.h"

// Matrix4::SetRotationIdentity: sets the diagonal of the upper-left 3x3 to 1 and the other eight elements of it to 0; the
// translation and the fourth column and row are left alone. Called by the node update at EN 0x763e70.
void Matrix4::SetRotationIdentity()
{
    m[10] = 1.0f;
    m[5] = 1.0f;
    m[0] = 1.0f;
    m[9] = 0;
    m[8] = 0;
    m[6] = 0;
    m[4] = 0;
    m[2] = 0;
    m[1] = 0;
}

// Matrix4::SetRotationTransposed: copies the upper-left 3x3 of `source` into this matrix with rows and columns swapped;
// the other elements are left alone. Called by the node update at EN 0x763e70.
void Matrix4::SetRotationTransposed(const Matrix4 *source)
{
    m[0] = source->m[0];
    m[4] = source->m[1];
    m[8] = source->m[2];
    m[1] = source->m[4];
    m[5] = source->m[5];
    m[9] = source->m[6];
    m[2] = source->m[8];
    m[6] = source->m[9];
    m[10] = source->m[10];
}
