// Out-of-line copies of math.h's inline float functions. The game calls acosf/atan2f; at some call site the
// compiler did not inline them, so APP.dll contains standalone copies (0x10001f60 acosf, 0x10001f80 atan2f).
// Taking their addresses here makes the compiler emit the same copies.

#include <math.h>

float (*DAT_acosf)(float) = acosf;
float (*DAT_atan2f)(float, float) = atan2f;
