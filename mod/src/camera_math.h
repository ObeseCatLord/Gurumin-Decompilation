// Pure camera math for camera.c (no Windows or game dependencies, so mod/tools/camera_math_test.c can run it on
// the host). Ported from Gurumin Enhanced Rendering (MIT, ObeseCatLord), src/settings.hpp: radialStick, orbitYaw,
// orbitFromAnchor (pitch part), zoomCamera and CameraZoom. Positions are x, y on the ground and z up, as in the
// game's camera code (decomp/src/camera_update.cpp lifts the target with z).
#ifndef GURUMIN_CAMERA_MATH_H
#define GURUMIN_CAMERA_MATH_H

#include <math.h>

#define CAMERA_PITCH_LIMIT 1.3962634f   // 80 degrees: the eye never reaches straight above or below the target
#define CAMERA_MIN_DISTANCE 60          // zoom range in percent of the game's own distance
#define CAMERA_MAX_DISTANCE 250

typedef struct
{
    float x, y;
} CameraStick;

static float camera_clampf(float value, float low, float high)
{
    return value < low ? low : value > high ? high : value;
}

static int camera_finite3(const float *v)
{
    return isfinite(v[0]) && isfinite(v[1]) && isfinite(v[2]);
}

// XInput thumb values (-32768..32767) to -1..1 with a radial dead zone (also in XInput units); the response starts
// at 0 at the edge of the dead zone.
static CameraStick camera_radial_stick(int x, int y, int deadzone)
{
    CameraStick stick = {0, 0};
    float fx = camera_clampf(x / 32767.0f, -1.0f, 1.0f);
    float fy = camera_clampf(y / 32767.0f, -1.0f, 1.0f);
    float length = sqrtf(fx * fx + fy * fy);
    float dz = camera_clampf(deadzone / 32767.0f, 0.0f, 0.9f);
    if (length <= dz)
        return stick;
    float gain = ((length < 1.0f ? length : 1.0f) - dz) / ((1.0f - dz) * length);
    stick.x = fx * gain;
    stick.y = fy * gain;
    return stick;
}

// Turns `eye` about `target` by `radians` on the ground plane (x, y); height and distance stay. 0 on bad input.
static int camera_orbit_yaw(float *eye, const float *target, float radians)
{
    if (!camera_finite3(eye) || !camera_finite3(target) || !isfinite(radians))
        return 0;
    float x = eye[0] - target[0], y = eye[1] - target[1];
    float c = cosf(radians), s = sinf(radians);
    eye[0] = target[0] + x * c - y * s;
    eye[1] = target[1] + x * s + y * c;
    return 1;
}

// Raises (pitch > 0) or lowers `eye` about `target` along its own heading, keeping the distance; the elevation stays
// within +-CAMERA_PITCH_LIMIT. Stores the pitch that was actually applied in *accepted. 0 on bad input.
static int camera_orbit_pitch(float *eye, const float *target, float pitch, float *accepted)
{
    if (!camera_finite3(eye) || !camera_finite3(target) || !isfinite(pitch))
        return 0;
    double dx = (double)eye[0] - target[0], dy = (double)eye[1] - target[1], dz = (double)eye[2] - target[2];
    double ground = sqrt(dx * dx + dy * dy), radius = sqrt(ground * ground + dz * dz);
    if (radius < 0.001 || ground < 0.001)
        return 0;
    double base = atan2(dz, ground);
    double elevation = base + pitch;
    if (elevation > CAMERA_PITCH_LIMIT)
        elevation = CAMERA_PITCH_LIMIT;
    if (elevation < -CAMERA_PITCH_LIMIT)
        elevation = -CAMERA_PITCH_LIMIT;
    double newGround = radius * cos(elevation);
    eye[0] = (float)(target[0] + dx / ground * newGround);
    eye[1] = (float)(target[1] + dy / ground * newGround);
    eye[2] = (float)(target[2] + radius * sin(elevation));
    if (accepted)
        *accepted = (float)(elevation - base);
    return 1;
}

// Moves `eye` along the target-to-eye line to `scale` times its distance. 0 on bad input.
static int camera_zoom(float *eye, const float *target, float scale)
{
    if (!camera_finite3(eye) || !camera_finite3(target) || !isfinite(scale) || scale <= 0)
        return 0;
    for (int i = 0; i < 3; i++)
        eye[i] = target[i] + (eye[i] - target[i]) * scale;
    return 1;
}

// Zoom distance in percent: requests move `target`, the drawn distance `current` eases towards it.
typedef struct
{
    float target, current;
} CameraZoom;

static void camera_zoom_reset(CameraZoom *zoom, int percent)
{
    zoom->target = zoom->current = camera_clampf((float)percent, CAMERA_MIN_DISTANCE, CAMERA_MAX_DISTANCE);
}

static void camera_zoom_step(CameraZoom *zoom, float percent)
{
    if (isfinite(percent))
        zoom->target = camera_clampf(zoom->target + percent, CAMERA_MIN_DISTANCE, CAMERA_MAX_DISTANCE);
}

// Eases `current` towards `target` over `seconds`: a quarter of the gap per 1/30 s, as the EN mod does per tick.
static void camera_zoom_ease(CameraZoom *zoom, float seconds)
{
    if (!(seconds > 0))
        return;
    float keep = powf(0.75f, camera_clampf(seconds, 0, 1.0f) * 30.0f);
    zoom->current = zoom->target + (zoom->current - zoom->target) * keep;
    if (fabsf(zoom->target - zoom->current) < 0.05f)
        zoom->current = zoom->target;
}

#endif
