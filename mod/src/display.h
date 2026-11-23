#ifndef GURUMIN_DISPLAY_H
#define GURUMIN_DISPLAY_H
#include <windows.h>

void display_set_module(HINSTANCE instance);
void display_before_direct3d(void);
void display_hook_direct3d(void *d3d);
void display_shutdown(void);
void movies_install(BYTE *app_base, void (*log)(const char *format, ...));

#endif
