// Desktop folder lookup of the EN build (used by the installer menu that creates the desktop shortcut).
#ifndef GURUMIN_EN_DESKTOP_PATH_H
#define GURUMIN_EN_DESKTOP_PATH_H

// Writes the path of the user's desktop folder into `path` (MAX_PATH characters); leaves it unchanged when the shell
// allocator is not available. EN 0x7aa0f0 (stdcall).
void __stdcall DesktopPath_Get(char *path);

// Builds the path of a Falcom data folder in `path` (MAX_PATH characters): the user's profile folder, then
// "\Application Data\FALCOM\" and `name`; `userName` names the profile when the shell cannot. `modulePath` (the game's library) is not used. Returns 0. EN 0x7aefd0 (cdecl).
extern "C" int FalcomPath_Get(char *path, const char *userName, const char *name, const char *modulePath);

// Deletes the files directly inside the directory `path` and then the directory. EN 0x7aa020 (stdcall).
void __stdcall Directory_RemoveWithFiles(const char *path);

// Creates the directory `path` and every missing parent. EN 0x7aef40 (cdecl; not ported).
extern "C" int Directory_CreateAll(char *path);

#endif
