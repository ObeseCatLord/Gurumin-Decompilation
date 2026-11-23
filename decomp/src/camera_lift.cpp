// Camera lift over uneven ground: casts five rays down around a point and lifts the camera target by the best result.

#include "vec4.h"
#include "node.h"
#include "camera_vars.h"

#define LIFT_PROBE_COUNT 5
#define LIFT_PROBE_SPREAD 280.0f   // distance of the four outer probes from the point
#define LIFT_RAY_HALF_LENGTH 300.0f
#define LIFT_ON_HIT 120.0f

#define LIFT_MAX(a, b) ((b) < (a) ? (a) : (b))
#define LIFT_MIN(a, b) ((a) > (b) ? (b) : (a))

// 0x1016be80 (Camera_LiftOverGround). Probes five points around `point` (the point and the four at +/-280 on x and
// y): from 300 above to 300 below each, a ray is cast down against the camera collision, and a probe that hits
// counts 120. The largest of the five, at most 120, becomes the lift DAT_1101dde4, which is added to the height
// of `target`. Returns 1 at once when the camera has no collision, else 0.
extern "C" int FUN_1016be80(Vec4 *target, Vec4 *point)
{
    CameraTarget *camera = DAT_1101d884;
    if (camera->collision == 0)
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
        if (camera->collision->RayCast(DAT_101e8d50, 0, 0, &below, &probe[i], &hit, 0) != 0)
            lift[i] = LIFT_ON_HIT;
        else
            lift[i] = 0;
    }
    DAT_1101dde4 = LIFT_MIN(LIFT_ON_HIT,
        LIFT_MAX(LIFT_MAX(LIFT_MAX(LIFT_MAX(lift[3], lift[4]), lift[2]), lift[1]), lift[0]));
    target->z += DAT_1101dde4;
    return 0;
}
