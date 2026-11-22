// The engine's 4D vector (16 bytes). Defined in vec4.cpp (constructors, setters, operators, segment helpers) and
// vec_math.cpp (lengths, distances, boxes, rotations, normalising). w is usually 1 for points and 0 for directions
// and is ignored by most helpers. This is the one declaration of Vec4: include this header, never redeclare it.
#ifndef GURUMIN_VEC4_H
#define GURUMIN_VEC4_H

struct Box2;
struct Matrix4;

struct Vec4
{
    float x;
    float y;
    float z;
    float w;

    Vec4();                                                    // 0x101ac990: zero
    Vec4(float x, float y, float z, float w);                  // 0x101ac9a0
    void Set(float x, float y, float z, float w);              // 0x101ac9c0
    void Copy(const Vec4 &source);                             // 0x101acc20
    void Fill(const float &value);                             // 0x101acc40: sets all four components

    float Length();                                            // 0x101ac4e0: length of x/y/z
    float Length2D();                                          // 0x101ac500: length of x/y
    float Distance2D(Vec4 other);                              // x/y distance
    float LengthSquared3();                                    // 0x101ac6a0: squared length of x/y/z
    float DistanceSquared(const Vec4 &other);                  // 0x101ac650
    float Distance(const Vec4 &other);                         // 0x101ac590
    float DistanceZWeighted(const Vec4 &other);                // 0x101ac5f0
    int InRect(const Box2 *box);                               // 0x101ac7c0: inside the box's x/y rectangle
    int InBox(const Box2 *box);                                // 0x101ac6e0
    int InBoxMargin(const Box2 *box, float margin);            // 0x101ac740

    void RotateX(float angle);                                 // 0x101ac440
    void RotateY(float angle);                                 // 0x101ac3a0
    void RotateZ(float angle);                                 // 0x101ac300
    void RotateAxis(const Vec4 &axis, float angle);            // 0x101b1290: turns about `axis`
    void Transform(const Matrix4 &matrix);                     // multiplies by `matrix` in place
    void Mirror(const Vec4 &normal);                           // reflects about the plane with `normal`
    void Normalize();                                          // normalises x/y/z in place
    Vec4 Normalized3();                                        // 0x101af080: normalises x/y/z in place, returns a copy

    Vec4 operator+(const Vec4 &other) const;                   // 0x101ac9e0
    Vec4 operator-(const Vec4 &other) const;                   // 0x101aca40
    Vec4 operator*(const Vec4 &other) const;                   // component-wise
    Vec4 operator*(const float &scale) const;                  // 0x101acb00: x/y/z scaled, w kept
    Vec4 Divided(const float &divisor);                        // 0x101acb60
    Vec4 operator/(const int &divisor) const;

    float DistanceSquaredToSegment2D(float x1, float y1, float x2, float y2);
    float DistanceSquaredToSegment(const Vec4 &a, const Vec4 &b);
    float DistanceToSegment(const Vec4 &a, const Vec4 &b, Vec4 &closest);
    float DistanceToLine(const Vec4 &a, const Vec4 &b);
    float DistanceSquaredToLine(const Vec4 &a, const Vec4 &b);
    int InTriangle(const Vec4 &a, const Vec4 &b, const Vec4 &c);
};

#endif
