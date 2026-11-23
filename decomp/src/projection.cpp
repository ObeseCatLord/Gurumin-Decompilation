// Projection parameters.

#include "camera_vars.h"

// Sets the fog: colour (red, green, blue), density and the distances it starts and is full at, keeping
// 1 / (far - near) for the depth scaling. The "fogset" node of a map does the same in Node::QueueDraw.
extern "C" int FUN_10199dc0(float red, float green, float blue, float density, float nearZ, float farZ)
{
    DAT_10f668f8.x = red;
    DAT_10f668f8.y = green;
    DAT_10f668f8.z = blue;
    DAT_10f66968 = nearZ;
    DAT_10f6696c = 1.0f / (farZ - nearZ);
    DAT_10f66970 = 0;
    DAT_10f66974 = density;
    return 1;
}
