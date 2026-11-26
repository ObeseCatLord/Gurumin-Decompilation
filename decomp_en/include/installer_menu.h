// The installer menu object of the EN build (the menu that creates and removes the desktop shortcut). Declared once
// here; only the members EN code has used so far are named.
#ifndef GURUMIN_EN_INSTALLER_MENU_H
#define GURUMIN_EN_INSTALLER_MENU_H

#include <windows.h>

struct InstallerMenu
{
    // EN 0x7a9db0: creates the shortcut file `linkFile` that starts `target`, with `description` and, when not empty,
    // the command line `arguments`; the working directory is the folder of `target`. Returns the HRESULT of the save.
    HRESULT CreateShortcut(const char *target, const char *linkFile, const char *description, const char *arguments);
    // EN 0x7aa130: creates the desktop shortcut "<desktop folder>\<name>.lnk" that starts `target`. Returns 0.
    int CreateDesktopShortcut(const char *target, const char *name, const char *arguments);
};

#endif
