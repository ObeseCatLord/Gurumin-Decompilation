// The engine's 3D vector (12 bytes): three floats for positions, directions, rotation limits and the like, where no
// w component is stored. Unlike Vec4 it has no constructor or methods. (The Vec3_Dot/Vec3_Length helpers despite
// their names take Vec4 pointers and read only x/y/z; they are declared in vec4.h.)
// This is the one declaration of Vec3: include this header, never redeclare it.
#ifndef GURUMIN_VEC3_H
#define GURUMIN_VEC3_H

struct Vec3
{
    float x;
    float y;
    float z;
};

#endif
