// Larger dynamic-shadow render targets for the JP APP.dll (shadows.c, docs/CAMERA_SHADOWS_JP.md). Ported from
// Gurumin Enhanced Rendering (MIT, ObeseCatLord).
#ifndef GURUMIN_SHADOWS_H
#define GURUMIN_SHADOWS_H

// Reads [shadows] from `ini` and, when enabled, redirects the shadow-target calls (after the build-stamp check).
void shadows_install(void (*log)(const char *format, ...), const char *ini);
// 1 once the call sites were verified and redirected.
int shadows_active(void);
// Called with the new IDirect3DDevice9: clamps the size to the device and hooks CreateDepthStencilSurface.
void shadows_device_created(void *device);

#endif
