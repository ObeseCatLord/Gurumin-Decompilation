// Camera orbit (JP twin: camera_orbit.cpp).

#include "camera.h"
#include "vec4.h"

// Places the camera at heading `angle` around the target, keeping its ground distance and its height above the
// target. Called by the orbit camera shots every frame. Returns 0.
extern "C" int Camera_PlaceAtHeading(float angle)
{
    Vec4 offset;
    float zero = 0;
    offset.Fill(zero);
    offset.z = g_cameraPositionCopy.z - g_cameraTargetCopy.z;
    offset.y = (g_cameraPositionCopy - g_cameraTargetCopy).Length2D();
    offset.RotateZ(angle);
    g_cameraPositionCopy.Copy(g_cameraTargetCopy + offset);
    return 0;
}
