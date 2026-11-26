// Orbit camera and zoom for Gurumin (JP PC, bin\APP.dll). UNTESTED on the Japanese game: off by default.
//
// Ported from Gurumin Enhanced Rendering (MIT, ObeseCatLord), src/camera.hpp, for the English Steam game.exe. The EN
// mod hooks six functions; most of them have no evidenced JP counterpart, so this port does the same job through
// one call that the decompilation shows byte for byte (see docs/CAMERA_SHADOWS_JP.md for the evidence):
//
//   Camera_Refresh 0x10015960 (decomp/src/camera_refresh.cpp, matched) calls Camera_Update 0x1016c3e0 once a frame
//   to get the eye and the target, then builds the view from them and publishes them. That one call is redirected
//   to camera_update_hook below.
//
// What the hook does, only while the game's free follow camera runs (the same test as FUN_1016c300, which the game
// uses before its own camera turns) and the player has control:
//   - yaw (right stick x, or the mouse while `mouse_button` is held): turns the camera position DAT_11af3e28 about
//     its target DAT_118e6c78 before Camera_Update runs and sets DAT_1101de30 = 2, exactly what the game's own mouse
//     wheel and camera keys do (decomp/src/game_frame.cpp). The follow camera, the movement direction
//     (Camera_TurnInput reads the same two vectors) and the wall search all continue from the new heading;
//   - pitch (right stick y, or the mouse) and zoom (PageUp/PageDown, the wheel, optionally the stick): applied to
//     the eye Camera_Update returns, so they change only what is drawn (and the published eye), never the follow
//     camera's own state. A raised, lowered or more distant eye is then kept in front of walls the way the game's
//     item-stand camera does it (Camera_PullToItemStand 0x10018870, matched): cast against the map's "CA" (mask 4)
//     and "CS" shapes and stop 25 units short of a hit.
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "display.h"
#include "camera.h"
#include "camera_math.h"

// Call site (decomp/src/camera_refresh.cpp, matched 100 %, 2968 bytes) and callee.
#define VA_CAMERA_REFRESH 0x10015960u
#define SIZE_CAMERA_REFRESH 2968u
#define VA_CAMERA_UPDATE 0x1016c3e0u
// Node::CastSegment (decomp/include/node.h); called the same way by Camera_PullToItemStand (matched).
#define VA_CAST_SEGMENT 0x101aaa20u

// Globals (decomp/include/*.h; see docs/CAMERA_SHADOWS_JP.md).
#define VA_CAMERA_POSITION 0x11af3e28u   // Vec4: camera position (goal of the follow camera)
#define VA_CAMERA_TARGET 0x118e6c78u     // Vec4: camera target
#define VA_SCRIPT_CAMERA 0x1101ddf4u     // non-zero: a scripted camera runs
#define VA_CAMERA_MODE 0x1101de24u       // camera mode: 0 or 5 = follow the player
#define VA_EFFECT_CAMERA 0x10ff1df0u     // 1 = the effect camera runs (mode 0)
#define VA_FOLLOW_CAMERA 0x1101ddf0u     // 1 = follow camera
#define VA_PATH_CAMERA 0x1101d890u       // 1 while a path camera runs
#define VA_MANUAL_TURN 0x1101de30u       // 2 = the camera was turned by hand this frame (skips the wall peek)
#define VA_SCRIPTS_IDLE 0x1032d69cu      // 0 while a queued script runs (game camera keys are ignored then)
#define VA_DEMO 0x1065cb90u              // non-zero while a demo records or plays
#define VA_WHEEL_ROTATES 0x104c010fu     // char: config.ini WROT, the game turns the camera with the wheel
#define VA_MAP_ROOT 0x10255f08u          // Node *: the map's root node (world collision)

#define WALL_CLEARANCE 25.0f      // Camera_PullToItemStand: stop this far in front of a wall
#define WALL_NEAR_DISTANCE 50.0f  // nearer walls: stop at 3/4 of the distance instead
#define WALL_NEAR_SHRINK 0.75f
#define CAMERA_MASK 4             // "CA" shapes the camera collides with

typedef struct
{
    float x, y, z, w;
} Vec4;

typedef int(__cdecl *camera_update_fn)(Vec4 *eye, Vec4 *target);
typedef int(__thiscall *cast_segment_fn)(void *node, const char *name, unsigned mask, Vec4 *from, Vec4 *to, Vec4 *hit,
                                         float *t);

static void (*log_line)(const char *format, ...);
static char ini_path[MAX_PATH];
static int active;
static camera_update_fn game_camera_update;
static cast_segment_fn cast_segment;

static struct
{
    int enabled, yaw_speed, pitch_speed, deadzone, invert_x, invert_y, distance, stick_zoom, wheel_zoom;
    int zoom_in_key, zoom_out_key, mouse_button;
    float mouse_sensitivity;
} settings;

static CameraZoom zoom;
static float pitch;              // radians added to the eye's elevation
static LARGE_INTEGER last_time, frequency;
static int wheel_notches;        // WM_MOUSEWHEEL delta since the last frame (taken only when zooming)
static int zoom_dirty;
static double zoom_changed;
static POINT mouse_last;
static int mouse_held;
static HWND game_window;
static WNDPROC game_window_proc;
static unsigned long frames;

#define GLOBAL(type, va) (*(type *)app_address(va))

// ---- input ---------------------------------------------------------------------------------------------------

typedef struct
{
    WORD buttons;
    BYTE left_trigger, right_trigger;
    SHORT thumb_lx, thumb_ly, thumb_rx, thumb_ry;
} PadState;
typedef struct
{
    DWORD packet;
    PadState pad;
} XInputState;
typedef DWORD(WINAPI *xinput_get_state_fn)(DWORD index, XInputState *state);
static xinput_get_state_fn xinput_get_state;
static DWORD pad_index;

static void load_xinput(void)
{
    static const char *names[] = {"xinput1_4.dll", "xinput1_3.dll", "xinput9_1_0.dll"};
    for (size_t i = 0; i < sizeof names / sizeof names[0] && !xinput_get_state; i++)
    {
        HMODULE module = LoadLibraryA(names[i]);
        if (module)
            xinput_get_state = (xinput_get_state_fn)GetProcAddress(module, "XInputGetState");
        if (xinput_get_state)
            log_line("camera: right stick read through %s", names[i]);
    }
    if (!xinput_get_state)
        log_line("camera: no XInput DLL; right stick orbit unavailable (mouse and keys still work)");
}

// The right stick of the first connected XInput pad (0 when there is none).
static CameraStick read_right_stick(void)
{
    CameraStick none = {0, 0};
    if (!xinput_get_state)
        return none;
    XInputState state;
    for (DWORD tries = 0; tries < 4; tries++)
    {
        DWORD index = (pad_index + tries) % 4;
        memset(&state, 0, sizeof state);
        if (xinput_get_state(index, &state) == ERROR_SUCCESS)
        {
            pad_index = index;
            return camera_radial_stick(state.pad.thumb_rx, state.pad.thumb_ry, settings.deadzone);
        }
    }
    return none;
}

static int game_has_focus(void)
{
    HWND foreground = GetForegroundWindow();
    DWORD process = 0;
    if (!foreground)
        return 0;
    GetWindowThreadProcessId(foreground, &process);
    return process == GetCurrentProcessId();
}

// The wheel zooms (instead of reaching the game) in mode 2, or in mode 1 when the game does not turn the camera with
// the wheel (config.ini WROT=0) or Ctrl is held.
static int wheel_zooms(int control)
{
    if (settings.wheel_zoom == 2)
        return 1;
    return settings.wheel_zoom == 1 && (GLOBAL(char, VA_WHEEL_ROTATES) == 0 || control);
}

static int free_follow_camera(void);
static int player_controls_camera(void);

static LRESULT CALLBACK window_hook(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_MOUSEWHEEL && active && player_controls_camera() &&
        wheel_zooms((GET_KEYSTATE_WPARAM(wparam) & MK_CONTROL) != 0))
    {
        wheel_notches += GET_WHEEL_DELTA_WPARAM(wparam);
        return 0;
    }
    return CallWindowProcA(game_window_proc, window, message, wparam, lparam);
}

void camera_set_window(HWND window)
{
    if (!active || !window || game_window_proc || settings.wheel_zoom == 0)
        return;
    game_window = window;
    game_window_proc = (WNDPROC)SetWindowLongPtrA(window, GWLP_WNDPROC, (LONG_PTR)window_hook);
    log_line("camera: wheel zoom %s (mode %d; the game's own wheel turn WROT=%d)",
             game_window_proc ? "on" : "unavailable", settings.wheel_zoom, GLOBAL(char, VA_WHEEL_ROTATES));
}

// ---- game state ----------------------------------------------------------------------------------------------

// The free follow camera runs: the negation of FUN_1016c300 (decomp/src/camera.cpp), the test the game makes before
// it turns the camera for the mouse wheel and the camera keys.
static int free_follow_camera(void)
{
    int mode = GLOBAL(int, VA_CAMERA_MODE);
    if (GLOBAL(int, VA_SCRIPT_CAMERA) != 0)
        return 0;
    if (mode != 0 ? mode != 5 : GLOBAL(int, VA_EFFECT_CAMERA) != 0)
        return 0;
    if (GLOBAL(int, VA_FOLLOW_CAMERA) != 1)
        return 0;
    return GLOBAL(int, VA_PATH_CAMERA) == 0;
}

// Free follow camera, no queued script (the game's camera keys need DAT_1032d69c too), no demo, window in front.
static int player_controls_camera(void)
{
    return free_follow_camera() && GLOBAL(int, VA_SCRIPTS_IDLE) != 0 && GLOBAL(int, VA_DEMO) == 0 && game_has_focus();
}

static void save_distance(void)
{
    char text[16];
    snprintf(text, sizeof text, "%d", (int)lroundf(zoom.target));
    WritePrivateProfileStringA("camera", "distance", text, ini_path);
    log_line("camera: distance %s %% saved", text);
}

// Keeps `eye` in front of walls between it and `target`: Camera_PullToItemStand's test (decomp/src/
// camera_item_stand_pull.cpp), a segment from 25 units beyond the eye to the target against the map's "CA" shapes
// (mask 4) and then its "CS" shapes; a hit puts the eye 25 units short of it (3/4 of the way when nearer than 50).
static void keep_out_of_walls(Vec4 *eye, const Vec4 *target)
{
    void *root = GLOBAL(void *, VA_MAP_ROOT);
    if (!root)
        return;
    float dx = eye->x - target->x, dy = eye->y - target->y, dz = eye->z - target->z;
    float distance = sqrtf(dx * dx + dy * dy + dz * dz);
    if (!(distance > 1.0f))
        return;
    dx /= distance, dy /= distance, dz /= distance;
    static const char *const names[2] = {"CA", "CS"};
    static const unsigned masks[2] = {CAMERA_MASK, 0};
    for (int pass = 0; pass < 2; pass++)
    {
        Vec4 from = {target->x + dx * (distance + WALL_CLEARANCE), target->y + dy * (distance + WALL_CLEARANCE),
                     target->z + dz * (distance + WALL_CLEARANCE), 0};
        Vec4 to = *target, hit = {0, 0, 0, 0};
        if (!cast_segment(root, names[pass], masks[pass], &from, &to, &hit, NULL))
            continue;
        float hx = hit.x - target->x, hy = hit.y - target->y, hz = hit.z - target->z;
        float clearance = sqrtf(hx * hx + hy * hy + hz * hz);
        clearance = clearance > WALL_NEAR_DISTANCE ? clearance - WALL_CLEARANCE : clearance * WALL_NEAR_SHRINK;
        if (clearance < distance)
        {
            distance = clearance;
            eye->x = target->x + dx * distance;
            eye->y = target->y + dy * distance;
            eye->z = target->z + dz * distance;
        }
    }
}

// ---- the hook ------------------------------------------------------------------------------------------------

static int __cdecl camera_update_hook(Vec4 *eye, Vec4 *target)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    float seconds = last_time.QuadPart ? (float)(now.QuadPart - last_time.QuadPart) / (float)frequency.QuadPart : 0;
    last_time = now;
    seconds = camera_clampf(seconds, 0, 0.1f);   // Camera_Refresh runs three times in a row while a map loads
    frames++;

    int follow = free_follow_camera();
    float yaw = 0;
    float before = zoom.target;
    if (!follow)
    {
        pitch = 0;   // events and scripted cameras end the user's tilt, as in the EN mod
        mouse_held = 0;
    }
    else if (player_controls_camera())
    {
        const float degrees = 0.017453292f;
        CameraStick stick = read_right_stick();
        yaw = stick.x * settings.yaw_speed * degrees * seconds;
        float tilt = settings.stick_zoom ? 0 : stick.y * settings.pitch_speed * degrees * seconds;
        if (settings.stick_zoom)
            camera_zoom_step(&zoom, -90.0f * stick.y * seconds);
        if (settings.mouse_button && (GetAsyncKeyState(settings.mouse_button) & 0x8000))
        {
            POINT position;
            GetCursorPos(&position);
            if (mouse_held)
            {
                yaw += (position.x - mouse_last.x) * settings.mouse_sensitivity * degrees;
                tilt += -(position.y - mouse_last.y) * settings.mouse_sensitivity * degrees;
            }
            mouse_last = position;
            mouse_held = 1;
        }
        else
            mouse_held = 0;
        // The game's "turn the camera right" key (DAT_1065cb42) turns by +0.08 through Project(v, RotationZ(a)),
        // clockwise with Matrix4::SetRotationZ's layout, while camera_orbit_yaw turns counter-clockwise for a
        // positive angle: negate so that stick right turns like that key (invert_x reverses it).
        if (!settings.invert_x)
            yaw = -yaw;
        pitch += settings.invert_y ? tilt : -tilt;
        pitch = camera_clampf(pitch, -2.8f, 2.8f);
        // Held keys: 75 % of the game's distance per second; a wheel notch (120) towards the screen: 10 % closer.
        if (settings.zoom_in_key && (GetAsyncKeyState(settings.zoom_in_key) & 0x8000))
            camera_zoom_step(&zoom, -75.0f * seconds);
        if (settings.zoom_out_key && (GetAsyncKeyState(settings.zoom_out_key) & 0x8000))
            camera_zoom_step(&zoom, 75.0f * seconds);
        camera_zoom_step(&zoom, -10.0f * wheel_notches / 120.0f);
    }
    wheel_notches = 0;
    if (zoom.target != before)
    {
        zoom_dirty = 1;
        zoom_changed = (double)now.QuadPart / (double)frequency.QuadPart;
    }
    if (zoom_dirty && (double)now.QuadPart / (double)frequency.QuadPart - zoom_changed > 1.5)
    {
        zoom_dirty = 0;
        save_distance();
    }
    camera_zoom_ease(&zoom, seconds);

    if (yaw != 0)
    {
        // What the game's own wheel and camera keys do (game_frame.cpp): turn the position about the target and
        // mark the frame as a manual turn.
        Vec4 *position = &GLOBAL(Vec4, VA_CAMERA_POSITION);
        const Vec4 *about = &GLOBAL(Vec4, VA_CAMERA_TARGET);
        if (camera_orbit_yaw(&position->x, &about->x, yaw))
            GLOBAL(int, VA_MANUAL_TURN) = 2;
    }

    int result = game_camera_update(eye, target);

    float scale = zoom.current / 100.0f;
    if (result && follow && free_follow_camera() && (pitch != 0 || scale != 1.0f))
    {
        Vec4 drawn = *eye;
        if (pitch != 0)
        {
            float accepted = pitch;
            if (camera_orbit_pitch(&drawn.x, &target->x, pitch, &accepted))
                pitch = accepted;   // no stored input beyond the 80-degree limit
        }
        if (scale != 1.0f)
            camera_zoom(&drawn.x, &target->x, scale);
        if (pitch != 0 || scale > 1.0f)
            keep_out_of_walls(&drawn, target);
        if (camera_finite3(&drawn.x))
            *eye = drawn;
    }
    if (frames % 1800 == 1 && follow)
        log_line("camera: pitch %.3f, distance %.0f %%, eye %.0f,%.0f,%.0f target %.0f,%.0f,%.0f", pitch, zoom.current,
                 eye->x, eye->y, eye->z, target->x, target->y, target->z);
    return result;
}

// ---- install -------------------------------------------------------------------------------------------------

// An integer setting; 0x.. hexadecimal is accepted (virtual-key codes).
static int read_int(const char *key, int fallback)
{
    char text[32], *end;
    GetPrivateProfileStringA("camera", key, "", text, sizeof text, ini_path);
    long value = strtol(text, &end, 0);
    return end == text ? fallback : (int)value;
}

static void read_settings(void)
{
    char text[32];
    settings.enabled = read_int("enabled", 0);
    settings.yaw_speed = read_int("yaw_speed", 120);
    settings.pitch_speed = read_int("pitch_speed", 90);
    settings.deadzone = read_int("deadzone", 8689);
    settings.invert_x = read_int("invert_x", 0);
    settings.invert_y = read_int("invert_y", 0);
    settings.distance = read_int("distance", 100);
    settings.stick_zoom = read_int("stick_zoom", 0);
    settings.wheel_zoom = read_int("wheel_zoom", 1);
    settings.zoom_in_key = read_int("zoom_in_key", 0x21);    // PageUp
    settings.zoom_out_key = read_int("zoom_out_key", 0x22);  // PageDown
    settings.mouse_button = read_int("mouse_button", 0);
    GetPrivateProfileStringA("camera", "mouse_sensitivity", "0.25", text, sizeof text, ini_path);
    settings.mouse_sensitivity = (float)atof(text);

    settings.yaw_speed = (int)camera_clampf((float)settings.yaw_speed, 30, 360);
    settings.pitch_speed = (int)camera_clampf((float)settings.pitch_speed, 30, 180);
    settings.deadzone = (int)camera_clampf((float)settings.deadzone, 0, 29490);
    settings.wheel_zoom = (int)camera_clampf((float)settings.wheel_zoom, 0, 2);
    if (settings.zoom_in_key < 0 || settings.zoom_in_key > 254)
        settings.zoom_in_key = 0x21;
    if (settings.zoom_out_key < 0 || settings.zoom_out_key > 254)
        settings.zoom_out_key = 0x22;
    if (settings.mouse_button < 0 || settings.mouse_button > 254)
        settings.mouse_button = 0;
    if (!(settings.mouse_sensitivity > 0 && settings.mouse_sensitivity < 10))
        settings.mouse_sensitivity = 0.25f;
}

void camera_install(void (*log)(const char *format, ...), const char *ini)
{
    log_line = log;
    lstrcpynA(ini_path, ini, MAX_PATH);
    read_settings();
    if (!settings.enabled)
    {
        log_line("camera: off");
        return;
    }
    DWORD sites[4];
    int found = find_calls(VA_CAMERA_REFRESH, SIZE_CAMERA_REFRESH, VA_CAMERA_UPDATE, sites, 4);
    log_line("camera: Camera_Refresh %08lx (%lu bytes) has %d call(s) to Camera_Update %08lx (expected 1)",
             (unsigned long)VA_CAMERA_REFRESH, (unsigned long)SIZE_CAMERA_REFRESH, found,
             (unsigned long)VA_CAMERA_UPDATE);
    if (found != 1)
    {
        log_line("camera: call site not as in decomp/src/camera_refresh.cpp: orbit camera not installed");
        return;
    }
    game_camera_update = (camera_update_fn)app_address(VA_CAMERA_UPDATE);
    cast_segment = (cast_segment_fn)app_address(VA_CAST_SEGMENT);
    QueryPerformanceFrequency(&frequency);
    camera_zoom_reset(&zoom, settings.distance);
    load_xinput();
    redirect_calls(sites, 1, (void *)camera_update_hook);
    active = 1;
    log_line("camera: call at %08lx redirected; Node::CastSegment %08lx, position %08lx, target %08lx, map root %08lx",
             sites[0], (unsigned long)VA_CAST_SEGMENT, (unsigned long)VA_CAMERA_POSITION,
             (unsigned long)VA_CAMERA_TARGET, (unsigned long)VA_MAP_ROOT);
    log_line("camera: free follow test %08lx/%08lx/%08lx/%08lx/%08lx (as FUN_1016c300), manual turn %08lx, "
             "scripts idle %08lx, demo %08lx, WROT %08lx",
             (unsigned long)VA_SCRIPT_CAMERA, (unsigned long)VA_CAMERA_MODE, (unsigned long)VA_EFFECT_CAMERA,
             (unsigned long)VA_FOLLOW_CAMERA, (unsigned long)VA_PATH_CAMERA, (unsigned long)VA_MANUAL_TURN,
             (unsigned long)VA_SCRIPTS_IDLE, (unsigned long)VA_DEMO, (unsigned long)VA_WHEEL_ROTATES);
    log_line("camera: yaw %d deg/s, pitch %d deg/s, dead zone %d, invert x %d y %d, distance %.0f %%, stick zoom %d, "
             "keys %02x/%02x, mouse button %02x x %.2f",
             settings.yaw_speed, settings.pitch_speed, settings.deadzone, settings.invert_x, settings.invert_y,
             zoom.target, settings.stick_zoom, settings.zoom_in_key, settings.zoom_out_key, settings.mouse_button,
             settings.mouse_sensitivity);
}

int camera_active(void)
{
    return active;
}
