// Orbit camera and zoom for the JP APP.dll (camera.c, docs/CAMERA_SHADOWS_JP.md). Ported from Gurumin Enhanced
// Rendering (MIT, ObeseCatLord).
#ifndef GURUMIN_CAMERA_H
#define GURUMIN_CAMERA_H
#include <windows.h>

// Reads [camera] from `ini` and, when enabled, redirects Camera_Refresh's call to Camera_Update (after the
// build-stamp check).
void camera_install(void (*log)(const char *format, ...), const char *ini);
// 1 once the call site was verified and redirected.
int camera_active(void);
// The game window (from CreateDevice): subclassed for wheel zoom.
void camera_set_window(HWND window);

#endif
