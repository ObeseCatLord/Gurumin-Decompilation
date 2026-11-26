// Desktop folder lookup (EN build, EN-only: no JP twin found).
#include <windows.h>
#include <shlobj.h>

#include "desktop_path.h"

// Asks the shell for the desktop folder (CSIDL_DESKTOP) of the desktop window and writes its path into `path`. The
// result of SHGetSpecialFolderLocation is not checked. No direct caller.
void __stdcall DesktopPath_Get(char *path)
{
    IMalloc *shellAllocator;
    LPITEMIDLIST folderList;
    if (SHGetMalloc(&shellAllocator) >= 0)
    {
        SHGetSpecialFolderLocation(GetDesktopWindow(), CSIDL_DESKTOP, &folderList);
        SHGetPathFromIDListA(folderList, path);
    }
}
