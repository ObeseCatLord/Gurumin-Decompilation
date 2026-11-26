// EN-only launcher: the folders the game saves into (EN-only: no JP twin found).
#include <windows.h>
#include <stdio.h>

#include "desktop_path.h"
#include "launcher.h"

// Bytes of the user name buffer and of the current directory buffer.
const int LAUNCHER_NAME_BYTES = 0x100;
// Bytes of the path of the game's own library, handed to the Falcom folder lookup.
const int LAUNCHER_MODULE_PATH_BYTES = 0x104;
// Name the Falcom folder lookup uses for this game.
static const char LAUNCHER_GAME_FOLDER[] = "GURUMIN";

// Works out the folders the game saves into and the path of its configuration file. The save folder is the game's
// folder below the user's Falcom data folder (created with its parents) or, when that lookup fails, "SaveData\<user
// name>" (or "SaveData\Default" when the name is unknown) inside the install folder. The configuration file is
// "config.ini" in the save folder when it exists, else the "gurumin.def" of the install folder. The current directory
// is restored afterwards. Called by the launcher's start-up.
void Launcher_PrepareSaveFolders(void)
{
    char currentDirectory[LAUNCHER_NAME_BYTES];
    char userName[LAUNCHER_NAME_BYTES];
    char modulePath[LAUNCHER_MODULE_PATH_BYTES];
    DWORD nameSize;
    GetCurrentDirectoryA(LAUNCHER_NAME_BYTES, currentDirectory);
    nameSize = LAUNCHER_NAME_BYTES;
    sprintf(g_launcherSaveDir, "%sSaveData", g_launcherInstallDir);
    CreateDirectoryA(g_launcherSaveDir, 0);
    userName[0] = 0;
    BOOL gotName = GetUserNameA(userName, &nameSize);
    if (gotName == 0)
        userName[0] = 0;
    sprintf(modulePath, "%sbin\\app.dll", g_launcherInstallDir);
    if (FalcomPath_Get(g_launcherSaveDir, userName, LAUNCHER_GAME_FOLDER, modulePath) < 0)
    {
        if (GetUserNameA(userName, &nameSize) != 0)
        {
            sprintf(g_launcherSaveDir, "%sSaveData", g_launcherInstallDir, userName);
            CreateDirectoryA(g_launcherSaveDir, 0);
            sprintf(g_launcherSaveDir, "%sSaveData\\%s", g_launcherInstallDir, userName);
            CreateDirectoryA(g_launcherSaveDir, 0);
        }
        else
        {
            sprintf(g_launcherSaveDir, "%sSaveData", g_launcherInstallDir, userName);
            CreateDirectoryA(g_launcherSaveDir, 0);
            sprintf(g_launcherSaveDir, "%sSaveData\\Default", g_launcherInstallDir, userName);
            CreateDirectoryA(g_launcherSaveDir, 0);
        }
    }
    else
    {
        Directory_CreateAll(g_launcherSaveDir);
    }
    sprintf(g_launcherConfigPath, "%s\\config.ini", g_launcherSaveDir);
    FILE *config = fopen(g_launcherConfigPath, "rb");
    if (config != 0)
    {
        fclose(config);
        SetCurrentDirectoryA(currentDirectory);
        return;
    }
    sprintf(g_launcherConfigPath, "%s\\bin\\gurumin.def", g_launcherInstallDir);
    SetCurrentDirectoryA(currentDirectory);
}
