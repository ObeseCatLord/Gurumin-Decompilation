// Camera push offset (EN build): the player's push expressed in the camera's frame.
#include <math.h>

#include "camera.h"

// Factor between the player's push and the camera's push offset.
const float PUSH_SCALE = 2.0f;

// Camera_SetPushOffset: turns the ground-plane push (x, y) by the heading of the camera copy (the direction from its
// position to its target) and stores twice that in g_cameraPushOffset (z = 0). Called by the world update (0x60cb10)
// for the player's push this frame; returns 0.
extern "C" int Camera_SetPushOffset(float x, float y)
{
    float heading = (float)atan2(g_cameraTargetCopy.x - g_cameraPositionCopy.x, g_cameraTargetCopy.y - g_cameraPositionCopy.y);
    float cosine = (float)cos(heading);
    float sine = (float)sin(heading);
    float pushY = (cosine * y + sine * x) * PUSH_SCALE;
    float pushX = (cosine * x - sine * y) * PUSH_SCALE;
    g_cameraPushOffset.z = 0;
    g_cameraPushOffset.y = pushY;
    g_cameraPushOffset.x = pushX;
    return 0;
}
