#ifndef GURUMIN_DISPLAY_H
#define GURUMIN_DISPLAY_H
#include <windows.h>

void display_set_module(HINSTANCE instance);
void display_before_direct3d(void);
void display_hook_direct3d(void *d3d);
void display_shutdown(void);
void movies_install(void (*log)(const char *format, ...));
void files_install(void (*log)(const char *format, ...));

// hooks.c
void hooks_init(BYTE *app_base, void (*log)(const char *format, ...));
BYTE *app_address(DWORD va);
int code_matches(DWORD va, const short *expect, int length);
void *detour(DWORD va, int length, void *hook);

#endif
