// Desktop folder lookup of the EN build (used by the installer menu that creates the desktop shortcut).
#ifndef GURUMIN_EN_DESKTOP_PATH_H
#define GURUMIN_EN_DESKTOP_PATH_H

// Writes the path of the user's desktop folder into `path` (MAX_PATH characters); leaves it unchanged when the shell
// allocator is not available. EN 0x7aa0f0 (stdcall).
void __stdcall DesktopPath_Get(char *path);

#endif
