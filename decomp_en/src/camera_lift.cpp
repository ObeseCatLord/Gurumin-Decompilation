// Camera height lift over the ground (EN build). Ported from the matched JP source decomp/src/camera_lift.cpp.
#include "camera.h"
#include "small_b11.h"

// Points probed around the camera target (the point itself and four around it).
const int LIFT_PROBE_COUNT = 5;
// Distance of the four outer probes from the point.
const float LIFT_PROBE_SPREAD = 280.0f;
// Each probe ray starts this far above the probe and ends this far below it.
const float LIFT_RAY_HALF_LENGTH = 300.0f;
// Lift a probe that hits ground counts.
const float LIFT_ON_HIT = 120.0f;
// The most the target is lifted.
const float LIFT_MAX_TOTAL = 120.0f;
// Collision tag the camera rays look for.
const char *const CAMERA_COLLISION_TAG = "CT";

// The larger / smaller of two floats, written the way the original macros compared them.
#define LIFT_MAX(a, b) ((b) < (a) ? (a) : (b))
#define LIFT_MIN(a, b) ((a) > (b) ? (b) : (a))

// Camera_LiftOverGround: probes five points around `point` (the point and the four at +/-280 on x and y): from 300 above
// to 300 below each, a ray is cast down against the camera collision, and a probe that hits counts 120. The largest of
// the five, at most 120, becomes the lift g_cameraTargetLift, which is added to the height of `target`. Returns 1 at once
// when the camera has no collision, else 0. Called by the follow camera.
extern "C" int Camera_LiftOverGround(Vec4 *target, Vec4 *point)
{
    AnimModel *camera = g_boundModels;
    if (camera->dropCollider == 0)
        return 1;
    Vec4 probe[LIFT_PROBE_COUNT];
    probe[0].Copy(*point);
    probe[1].Copy(*point);
    probe[2].Copy(*point);
    probe[3].Copy(*point);
    probe[4].Copy(*point);
    probe[0].x += LIFT_PROBE_SPREAD;
    probe[1].y += LIFT_PROBE_SPREAD;
    probe[2].x -= LIFT_PROBE_SPREAD;
    probe[3].y -= LIFT_PROBE_SPREAD;
    Vec4 ray;
    Vec4 below;
    Vec4 hit;
    float lift[LIFT_PROBE_COUNT];
    ray.Copy(*target - *point);
    ray.Length2D();
    for (int i = 0; i < LIFT_PROBE_COUNT; i++)
    {
        probe[i].z += LIFT_RAY_HALF_LENGTH;
        below.Copy(probe[i]);
        below.z -= 2 * LIFT_RAY_HALF_LENGTH;
        if (camera->dropCollider->CastRay(CAMERA_COLLISION_TAG, 0, 0, &below, &probe[i], &hit, 0) != 0)
            lift[i] = LIFT_ON_HIT;
        else
            lift[i] = 0;
    }
    g_cameraTargetLift = LIFT_MIN(LIFT_MAX_TOTAL,
        LIFT_MAX(LIFT_MAX(LIFT_MAX(LIFT_MAX(lift[3], lift[4]), lift[2]), lift[1]), lift[0]));
    target->z += g_cameraTargetLift;
    return 0;
}
