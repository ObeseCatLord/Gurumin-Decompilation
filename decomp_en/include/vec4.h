// EN declaration of the engine's 4D vector (16 bytes) and 4x4 matrix (64 bytes). Only the members EN code has used so
// far are declared; the JP declarations are decomp/include/vec4.h and matrix4.h (same layouts, different compiler).
// This is the one declaration of Vec4 and Matrix4 in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_VEC4_H
#define GURUMIN_EN_VEC4_H

struct Box2;
struct Matrix4;

struct Vec4
{
    float x;  // x component
    float y;  // y component
    float z;  // z component
    float w;  // w component: 1 for points, 0 for directions; ignored by most helpers

    Vec4();                                          // EN 0x76f0c0: zero (JP 0x101ac990)
    Vec4(float x, float y, float z, float w);        // EN 0x76f090 (JP 0x101ac9a0)
    void Set(float x, float y, float z, float w);    // EN 0x779af0 (JP 0x101ac9c0)
    void Copy(const Vec4 &source);                   // EN 0x76f1d0 (JP 0x101acc20)
    void Fill(const float &value);                   // EN 0x76f1b0 (JP 0x101acc40): sets all four components
    float LengthSquared3();                          // EN 0x773490 (JP 0x101ac6a0): squared length of x/y/z
    float Length();                                  // EN 0x7732d0 (JP 0x101ac4e0): length of x/y/z
    float Length2D();                                // EN 0x773660 (JP 0x101ac500): length of x/y
    float Distance2D(Vec4 other);                    // EN 0x773610 (JP x/y distance)
    float DistanceSquared(const Vec4 &other);        // EN 0x773440 (JP 0x101ac650)
    float Distance(const Vec4 &other);               // EN 0x773270 (JP 0x101ac590)
    float DistanceSquaredToSegment(const Vec4 &a, const Vec4 &b);  // EN 0x772cc0 (JP 0x101acd90): squared distance to the segment a-b
    float DistanceToSegment(const Vec4 &a, const Vec4 &b, Vec4 &closest);  // EN 0x772ea0 (JP 0x101acdf0): distance to the segment a-b, closest point in `closest`
    float DistanceSquaredToSegment2D(float x1, float y1, float x2, float y2);   // EN 0x7734d0 (JP 0x101acc60): squared x/y distance to the segment (x1, y1)-(x2, y2)
    float DistanceZWeighted(const Vec4 &other);      // EN 0x7736f0 (JP 0x101ac5f0)
    int InRect(const Box2 *box);                     // EN 0x7749d0 (JP 0x101ac7c0): inside the box's x/y rectangle
    int InBox(const Box2 *box);                      // EN 0x774a10 (JP 0x101ac6e0)
    void RotateZ(float angle);                       // EN 0x779400 (JP 0x101ac300)
    void RotateX(float angle);                       // EN 0x779200 (JP 0x101ac440): rotates the y/z part about the x axis
    void RotateY(float angle);                       // EN 0x779300 (JP 0x101ac3a0): rotates the x/z part about the y axis
    int InBoxMargin(const Box2 *box, float margin);  // EN 0x774b10 (JP 0x101ac740): inside the box grown by `margin`

    Vec4 operator+(const Vec4 &other) const;         // EN 0x76f720 (JP 0x101ac9e0)
    Vec4 operator-(const Vec4 &other) const;         // EN 0x76f6d0 (JP 0x101aca40)
    Vec4 operator*(const Vec4 &other) const;         // EN 0x76f680: component-wise
    Vec4 operator*(const float &scale) const;        // EN 0x76f630: x/y/z times `scale`, w = 0
    Vec4 Normalized3();                              // EN 0x777320 (JP 0x101af080): x/y/z scaled to length 1 in place and returned (w = 0); a zero vector stays and zero is returned
    Vec4 Divided(const float &divisor);              // EN 0x76f7c0 (JP 0x101acb60)
};

// Dot product of the x/y/z parts of two vectors (EN 0x773750, JP 0x101ac200).
extern "C" float Vec3_Dot(const Vec4 *a, const Vec4 *b);

// Cross product a x b (EN 0x772250); the result's w stays 0.
Vec4 Cross(const Vec4 &a, const Vec4 &b);

// Axis-aligned box given by its minimum and maximum corners.
struct Box2
{
    Vec4 min;  // corner with the smallest x/y/z
    Vec4 max;  // corner with the largest x/y/z
};

// The matrix with rows and columns swapped (EN 0x776c40, JP 0x101b1090).
struct Matrix4 Transpose(const struct Matrix4 &matrix);

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
    float m[MATRIX_ELEMENT_COUNT];  // row-major, row vectors on the left; m[12..14] = translation

    Matrix4();                                      // EN 0x76efc0 (JP 0x101ae770): zeroes most elements
    ~Matrix4();                                     // EN 0x76f0e0 (JP 0x1013eda0): empty, out of line; gives constructors of owners an unwind frame
    Vec4 GetTranslation();                          // EN 0x773ef0 (JP 0x101ade80)
    void SetIdentity();                             // EN 0x7748e0 (JP 0x101ae7a0): the identity matrix
    void SetTranslation(Vec4 position);             // EN 0x77a170 (JP 0x101adef0)
    void Translate(float x, float y, float z);      // EN 0x776ed0 (JP 0x101adec0)
    Matrix4 operator*(const Matrix4 &other) const;  // EN 0x76f230 (JP 0x101b0670): this transformed by `other` (row vectors)
    void SetRotationZ(float angle);               // EN 0x77a1a0 (JP 0x101ae710): rotation about the z axis in the upper-left 3x3
};

// Rotation matrix looking from `eye` to `target` with `up` and roll `roll` (EN 0x77aea0, JP 0x101b1c20).
Matrix4 LookAt(Vec4 eye, Vec4 target, Vec4 *up, float roll);
// `a` transformed by `b` (EN 0x7768e0, JP 0x101b10f0).
Matrix4 Multiply(const Matrix4 &a, const Matrix4 &b);
// Inverse of `matrix` (EN 0x774f00, JP 0x101b0d40).
Matrix4 InverseMatrix(const Matrix4 &matrix);
// The identity matrix by value (EN 0x774950, JP 0x101b0ba0).
Matrix4 RotationY(float angle);   // EN 0x779290 (JP 0x101b1bb0): turn of `angle` radians about y
Matrix4 IdentityMatrix();   // EN 0x774950 (JP 0x101b0ba0)
Matrix4 ZeroMatrix();   // EN 0x77ba80 (JP 0x101b0cd0): every element 0
Matrix4 RotationX(float angle);   // EN 0x779190 (JP 0x101b1ad0): turn of `angle` radians about x
Matrix4 RotationZ(float angle);   // EN 0x779390 (JP 0x101b1b40): turn of `angle` radians about z

// Number of matrix products taken (incremented by Matrix4::operator*, reset by the frame code at EN 0x60c750).
extern "C" int g_matrixProductCount;   // EN 0x0218ee80

// Scale matrix (EN 0x779480, JP 0x101b13c0), perspective projection from the near and far planes and the field of
// view in radians and an aspect factor, 1 for none (EN 0x777770, JP 0x101b0c10), and `v` scaled to length 1 by x/y/z (EN 0x777410, JP 0x101b1180).
Matrix4 ScaleMatrix(float x, float y, float z);
Matrix4 PerspectiveMatrix(float nearPlane, float farPlane, float fov, float aspect);
Vec4 Vec3_Normalized(Vec4 *v);
// Translation matrix (EN 0x777220, JP 0x101b1360).
Matrix4 TranslationMatrix(float x, float y, float z);

// Rotation matrix of the quaternion `quaternion` (x, y, z = vector part, w = scalar part) (EN 0x777a20).
Matrix4 RotationMatrix(const Vec4 &quaternion);

// Transforms `point` by `matrix` and divides x/y/z by the resulting w (perspective divide); the result's w is that w
// (EN 0x77a4a0, JP 0x101ae060).
Vec4 Project(const Vec4 &point, const Matrix4 &matrix);

// Transforms the direction `v` by the rotation part of `matrix` (no translation); the result's w is 0
// (EN 0x77a2a0, JP 0x101ae130).
Vec4 TransformNormal(const Vec4 &v, const Matrix4 &matrix);

// 1 when `point` transformed by the view-projection `matrix` lies inside the view volume in x and y, with the volume
// widened by `margin` (EN 0x775320, JP 0x101ae4d0).
extern "C" int View_ContainsPoint(const Vec4 *point, const Matrix4 *matrix, float margin);

// Rotation quaternion (x, y, z = vector part, w = scalar part) of the rotation part of `matrix`
// (EN 0x776990, JP 0x101ae2a0).
Vec4 QuatFromMatrix(const Matrix4 &matrix);

// Spherical interpolation between the rotation quaternions `a` and `b` by `t` (EN 0x777d20, JP 0x101aeae0).
Vec4 Slerp(const Vec4 &a, const Vec4 &b, float t);

// Snaps an angle in radians to the nearest multiple of `step` degrees, returned in radians (EN 0x7476a0, JP 0x1016c340).
float SnapAngle(float radians, int step);

#endif
