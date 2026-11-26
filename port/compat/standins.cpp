// Port stand-ins: hand-written native versions of functions that the compiled decomp files call but that the JP
// decompilation does not define yet. They are NOT decompiled code and are not checked against APP.dll; each one is
// written from the declaration's comment in decomp/include/ (and, for the matrix product, from the EN build's
// reconstruction in decomp_en/src/vec4.cpp). Delete a stand-in as soon as the decomp defines the function.
//
// They are weak symbols on gcc/clang, so the day a decomp file that defines one joins gurumin_core, the decomp's
// definition wins instead of the link failing with a duplicate.
#include "vec4.h"
#include "matrix4.h"
#include "key_track.h"

#if defined(__GNUC__) || defined(__clang__)
#define PORT_STANDIN __attribute__((weak))
#else
#define PORT_STANDIN
#endif

// 0x1013eda0 in APP.dll (an empty destructor shared with other types). Nothing to release.
PORT_STANDIN Matrix4::~Matrix4()
{
}

// 0x101b0670: matrix product this * other for row vectors (element (row, column) is row `row` of this dotted with
// column `column` of other). The EN build's matched reconstruction computes the same sums.
PORT_STANDIN Matrix4 Matrix4::operator*(const Matrix4 &other)
{
    Matrix4 result;
    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            result.m[row * 4 + column] = m[row * 4 + 0] * other.m[0 * 4 + column] +
                                         m[row * 4 + 1] * other.m[1 * 4 + column] +
                                         m[row * 4 + 2] * other.m[2 * 4 + column] +
                                         m[row * 4 + 3] * other.m[3 * 4 + column];
        }
    }
    return result;
}

// "Multiplies by `matrix` in place" (vec4.h). Provisional: the vector is treated as the row vector (x, y, z, w)
// times the 4x4 matrix. Whether the original keeps w or forces it to 1 is not yet known.
PORT_STANDIN void Vec4::Transform(const Matrix4 &matrix)
{
    const float *m = matrix.m;
    float nx = x * m[0] + y * m[4] + z * m[8] + w * m[12];
    float ny = x * m[1] + y * m[5] + z * m[9] + w * m[13];
    float nz = x * m[2] + y * m[6] + z * m[10] + w * m[14];
    float nw = x * m[3] + y * m[7] + z * m[11] + w * m[15];
    x = nx;
    y = ny;
    z = nz;
    w = nw;
}

// "Appends a key (used by CopyFrom)" (key_track.h). Writes the key after the last one with flag 0 and keeps the
// same 16-key headroom that KeyTrack::Insert keeps. Returns the new key's index.
PORT_STANDIN int KeyTrack::AddKey(int frame, float x, float y, float z, float w)
{
    if (count + 16 > capacity)
        Reserve(16);
    int index = count;
    frames[index] = frame;
    values[index].x = x;
    values[index].y = y;
    values[index].z = z;
    values[index].w = w;
    flags[index] = 0;
    count++;
    return index;
}
