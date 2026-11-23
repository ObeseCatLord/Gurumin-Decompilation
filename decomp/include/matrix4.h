// The engine's 4x4 matrix (64 bytes). Row-major with row vectors multiplied on the left (the Direct3D convention):
// m[0..2], m[4..6] and m[8..10] are the rotation/scale rows, m[12], m[13] and m[14] the translation. Defined in
// vec_math.cpp (constructor, Translate, SetRotationZ), vec4.cpp (GetTranslation) and containers.cpp (SetTranslation,
// ResetRotation, SetIdentity, SetRotationTransposed). This is the one declaration of Matrix4: include this header,
// never redeclare it. Character records keep their world matrix in one (see world.h).
#ifndef GURUMIN_MATRIX4_H
#define GURUMIN_MATRIX4_H

#include "vec3.h"
#include "vec4.h"

// Indices into Matrix4::m of the translation row.
enum Matrix4Element
{
    MATRIX_TRANSLATION_X = 12,  // m[12]: x of the translation row
    MATRIX_TRANSLATION_Y = 13,  // m[13]: y of the translation row
    MATRIX_TRANSLATION_Z = 14,  // m[14]: z of the translation row
    MATRIX_ELEMENT_COUNT = 16   // 4 rows of 4 floats
};

struct Matrix4
{
    union
    {
        float m[16];  // row-major; m[12], m[13], m[14] = translation
        struct        // the same elements by row and column (Direct3D's D3DMATRIX names)
        {
            float _11, _12, _13, _14;  // row 1: x axis
            float _21, _22, _23, _24;  // row 2: y axis
            float _31, _32, _33, _34;  // row 3: z axis
            float _41, _42, _43, _44;  // row 4: translation x, y, z and 1
        };
        struct        // the same rows as x/y/z vectors (the fourth column after each)
        {
            Vec3 axisX;          // row 1 x/y/z
            float axisXW;        // _14
            Vec3 axisY;          // row 2 x/y/z
            float axisYW;        // _24
            Vec3 axisZ;          // row 3 x/y/z
            float axisZW;        // _34
            Vec3 translation;    // row 4 x/y/z: the position (_41, _42, _43)
            float translationW;  // _44
        };
    };

    Matrix4();                                         // 0x101ae770: zeroes most elements (no identity)
    ~Matrix4();                                        // 0x1013eda0: empty (shared with other empty destructors)

    void SetIdentity();                                // 0x101ae7a0: identity matrix
    void ResetRotation();                              // 0x101adf10: rotation block to identity, translation kept
    void SetRotationZ(float angle);                    // 0x101ae710: rotation block = turn of `angle` about z
    void SetRotationTransposed(const Matrix4 &source); // 0x101ae7e0: rotation block = transpose of source's

    Vec4 GetTranslation();                             // 0x101ade80: translation row (w = 0)
    void SetTranslation(Vec4 position);                // 0x101adef0: translation row = position.x/y/z
    void Translate(float x, float y, float z);         // 0x101adec0: adds (x, y, z) to the translation row

    Matrix4 operator*(const Matrix4 &other);           // 0x101b0670: matrix product
};

Matrix4 RotationZ(float angle);   // 0x101b1b40: turn of `angle` about z
Matrix4 RotationX(float angle);   // 0x101b1ad0: turn of `angle` about x
Matrix4 Transpose(const Matrix4 &matrix);   // 0x101b1090: rows and columns swapped

Vec4 Project(const Vec4 &point, const Matrix4 &matrix);   // 0x101ae060: to -1..1 screen space

#endif
