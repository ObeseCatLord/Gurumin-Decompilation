// The out-of-line float math wrappers (math_float.cpp): the compiler did not inline acosf/atan2f at some call sites, so
// the game calls these standalone copies.
#ifndef GURUMIN_MATH_FLOAT_H
#define GURUMIN_MATH_FLOAT_H

extern "C" float FUN_10001f60(float x);            // 0x10001f60 (math_float.cpp): acosf
extern "C" float FUN_10001f80(float y, float x);   // 0x10001f80 (math_float.cpp): atan2f

#endif
