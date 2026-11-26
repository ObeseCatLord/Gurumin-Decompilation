// Creation of the desktop shortcut (EN build): the installer menu's "create shortcut" action.
#include <windows.h>
#include <shlobj.h>
#include <string.h>

#include "installer_menu.h"

// Characters of the desktop path buffer and of the shortcut path.
const int DESKTOP_SHORTCUT_PATH_SIZE = 0x104;
// Character that ends a folder name in a path.
const char DESKTOP_SHORTCUT_SEPARATOR = '\\';

// Creates the shortcut "<desktop folder>\<name>.lnk" that starts `target` (its description is `name`, its command
// line `arguments`). The desktop folder comes from the shell (an empty path when the shell cannot be asked). Always
// returns 0. Called by the installer menu.
int InstallerMenu::CreateDesktopShortcut(const char *target, const char *name, const char *arguments)
{
    char desktop[DESKTOP_SHORTCUT_PATH_SIZE];
    char link[DESKTOP_SHORTCUT_PATH_SIZE];
    IMalloc *allocator;
    LPITEMIDLIST list;

    memset(desktop, 0, sizeof(desktop));
    if (SHGetMalloc(&allocator) >= 0)
    {
        SHGetSpecialFolderLocation(GetDesktopWindow(), CSIDL_DESKTOP, &list);
        SHGetPathFromIDListA(list, desktop);
    }
    size_t length = strlen(desktop);
    if (desktop[length - 1] != DESKTOP_SHORTCUT_SEPARATOR)
    {
        desktop[length] = DESKTOP_SHORTCUT_SEPARATOR;
        desktop[strlen(desktop) + 1] = 0;   // the original terminates one byte after the end (harmless: the buffer is zeroed)
    }
    strcpy(link, desktop);
    strcat(link, name);
    strcat(link, ".lnk");
    CreateShortcut(target, link, name, arguments);
    return 0;
}
