// Windows shortcut creation (EN build): the helper behind the "create shortcut" option of the game's installer menu.
#include <windows.h>
#include <shlobj.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Milliseconds the helper waits after COM start-up before creating the shell link object.
const DWORD SHORTCUT_COM_DELAY_MS = 1000;
// Characters of the wide path buffer.
const int SHORTCUT_WIDE_PATH_SIZE = 0x104;
// Character that ends a folder name in a path.
const char SHORTCUT_PATH_SEPARATOR = '\\';

// CreateShortcut: creates the shortcut file `linkFile` that starts `target` with the description `description` and,
// when it is not empty, the command line `arguments`. The working directory of the shortcut is the folder of `target`.
// Returns the HRESULT of the save (a negative value when COM or the link object could not be created). Called by the
// installer menu.
HRESULT __stdcall CreateShortcut(const char *target, const char *linkFile, const char *description, const char *arguments)
{
    IShellLinkA *link;
    IPersistFile *file;
    char folder[_MAX_PATH];
    char directory[_MAX_DIR];
    char extension[_MAX_EXT];
    char fileName[_MAX_FNAME];
    char drive[_MAX_DRIVE];
    WCHAR wideLinkFile[SHORTCUT_WIDE_PATH_SIZE];
    HRESULT result;

    CoInitialize(0);
    Sleep(SHORTCUT_COM_DELAY_MS);
    result = CoCreateInstance(CLSID_ShellLink, 0, CLSCTX_INPROC_SERVER, IID_IShellLinkA, (void **)&link);
    if (result >= 0)
    {
        result = link->QueryInterface(IID_IPersistFile, (void **)&file);
        if (result >= 0)
        {
            link->SetPath(target);
            link->SetDescription(description);
            if (arguments != 0 && *arguments != 0)
                link->SetArguments(arguments);
            _splitpath(target, drive, directory, fileName, extension);
            sprintf(folder, "%s%s", drive, directory);
            size_t length = strlen(folder);
            if (folder[length - 1] == SHORTCUT_PATH_SEPARATOR)
                folder[length - 1] = 0;
            link->SetWorkingDirectory(folder);
            MultiByteToWideChar(CP_ACP, 0, linkFile, -1, wideLinkFile, SHORTCUT_WIDE_PATH_SIZE);
            result = file->Save(wideLinkFile, TRUE);
            file->Release();
        }
        link->Release();
    }
    CoUninitialize();
    return result;
}

// Characters of the desktop path buffer and of the shortcut path.
const int SHORTCUT_PATH_SIZE = 0x104;

// Deletes the desktop shortcut called `name` (the file "<desktop folder>\<name>.lnk"). The desktop folder comes from
// the shell (an empty path when the shell cannot be asked). Always returns 0. Called by the uninstaller menu.
// EN 0x7a9f00 (JP 0x1013e260).
int __stdcall DeleteDesktopShortcut(const char *name)
{
    char desktop[SHORTCUT_PATH_SIZE];
    char path[SHORTCUT_PATH_SIZE];
    IMalloc *allocator;
    LPITEMIDLIST list;

    memset(desktop, 0, sizeof(desktop));
    memset(path, 0, sizeof(path));
    if (SHGetMalloc(&allocator) >= 0)
    {
        SHGetSpecialFolderLocation(GetDesktopWindow(), CSIDL_DESKTOP, &list);
        SHGetPathFromIDListA(list, desktop);
    }
    size_t length = strlen(desktop);
    if (desktop[length - 1] != SHORTCUT_PATH_SEPARATOR)
    {
        desktop[length] = SHORTCUT_PATH_SEPARATOR;
        desktop[strlen(desktop) + 1] = 0;   // the original terminates one byte after the end (harmless: the buffer is zeroed)
    }
    strcpy(path, desktop);
    strcat(path, name);
    strcat(path, ".lnk");
    DeleteFileA(path);
    return 0;
}
