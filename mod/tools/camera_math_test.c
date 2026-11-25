// Host-side checks of the pure camera math in mod/src/camera_math.h:
//   cc -O2 -o /tmp/camera_math_test mod/tools/camera_math_test.c -lm && /tmp/camera_math_test
#include <stdio.h>
#include <math.h>
#include "../src/camera_math.h"

static int failures;

static void check(int condition, const char *what)
{
    if (!condition)
    {
        printf("FAIL %s\n", what);
        failures++;
    }
}

static float distance3(const float *a, const float *b)
{
    float x = a[0] - b[0], y = a[1] - b[1], z = a[2] - b[2];
    return sqrtf(x * x + y * y + z * z);
}

int main(void)
{
    float target[3] = {100, 200, 70};

    // Yaw keeps height and distance; a quarter turn moves +y offset to -x.
    float eye[3] = {100, 1250, 300};
    float before = distance3(eye, target);
    check(camera_orbit_yaw(eye, target, 1.5707963f), "yaw ok");
    check(fabsf(eye[2] - 300) < 1e-3f, "yaw keeps height");
    check(fabsf(distance3(eye, target) - before) < 1e-2f, "yaw keeps distance");
    check(fabsf(eye[0] - (100 - 1050)) < 1e-2f && fabsf(eye[1] - 200) < 1e-2f, "yaw direction (counter-clockwise)");

    // Pitch keeps distance and heading and stops at 80 degrees.
    float tilted[3] = {100, 1250, 70};
    float accepted = 0;
    check(camera_orbit_pitch(tilted, target, 0.5f, &accepted), "pitch ok");
    check(fabsf(accepted - 0.5f) < 1e-4f, "pitch accepted");
    check(fabsf(distance3(tilted, target) - 1050) < 1e-2f, "pitch keeps distance");
    check(fabsf(tilted[0] - 100) < 1e-3f && tilted[1] > 200, "pitch keeps heading");
    check(fabsf(atan2f(tilted[2] - 70, tilted[1] - 200) - 0.5f) < 1e-4f, "pitch elevation");
    float steep[3] = {100, 1250, 70};
    check(camera_orbit_pitch(steep, target, 3.0f, &accepted), "steep pitch ok");
    check(fabsf(accepted - CAMERA_PITCH_LIMIT) < 1e-4f, "pitch clamped to 80 degrees");
    float above[3] = {100, 200, 500};
    check(!camera_orbit_pitch(above, target, 0.1f, &accepted), "no heading straight above");
    float bad[3] = {NAN, 0, 0};
    check(!camera_orbit_yaw(bad, target, 0.1f) && !camera_zoom(bad, target, 2), "NaN rejected");

    // Zoom moves along the line.
    float far[3] = {100, 1250, 300};
    check(camera_zoom(far, target, 1.5f), "zoom ok");
    check(fabsf(far[1] - (200 + 1050 * 1.5f)) < 1e-2f && fabsf(far[2] - (70 + 230 * 1.5f)) < 1e-2f, "zoom scale");
    check(!camera_zoom(far, target, 0), "zoom 0 rejected");

    // Zoom distance clamps and eases (a quarter of the gap per 1/30 s).
    CameraZoom zoom;
    camera_zoom_reset(&zoom, 500);
    check(zoom.target == CAMERA_MAX_DISTANCE && zoom.current == CAMERA_MAX_DISTANCE, "zoom reset clamps");
    camera_zoom_reset(&zoom, 100);
    camera_zoom_step(&zoom, -100);
    check(zoom.target == CAMERA_MIN_DISTANCE, "zoom step clamps");
    camera_zoom_ease(&zoom, 1.0f / 30);
    check(fabsf(zoom.current - 90) < 1e-3f, "zoom ease one tick");
    for (int i = 0; i < 100; i++)
        camera_zoom_ease(&zoom, 1.0f / 30);
    check(zoom.current == CAMERA_MIN_DISTANCE, "zoom ease settles");

    // Radial stick: dead zone and full deflection.
    CameraStick stick = camera_radial_stick(5000, 0, 8689);
    check(stick.x == 0 && stick.y == 0, "stick dead zone");
    stick = camera_radial_stick(32767, 0, 8689);
    check(fabsf(stick.x - 1) < 1e-4f && stick.y == 0, "stick full");
    stick = camera_radial_stick(-32768, 0, 8689);
    check(fabsf(stick.x + 1) < 1e-4f, "stick full negative");

    printf(failures ? "%d failure(s)\n" : "all camera math checks passed\n", failures);
    return failures != 0;
}
