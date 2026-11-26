// Loading a save file by slot name through the Steam save loader (EN build, no JP twin).
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "game_file.h"
#include "save_slot.h"

// Size of the path buffers, and the character that ends a folder name.
const int SAVE_STEAM_PATH_SIZE = 0x100;
const char SAVE_STEAM_PATH_SEPARATOR = '\\';

// Save_LoadSlotSteam: like Save_LoadSlot, but hands the path to the Steam save loader Save_LoadSteam. The program's
// folder is cut back to its last backslash, "SaveData\" and the name are appended. Called by the load menu.
int Save_LoadSlotSteam(const char *name, int unused)
{
    GameFile file;      // never used: the original constructs and destroys one here (leftover from a file-based version)
    char folder[SAVE_STEAM_PATH_SIZE];
    char path[SAVE_STEAM_PATH_SIZE];
    GetModuleFileNameA(0, folder, SAVE_STEAM_PATH_SIZE);
    char *end = folder + strlen(folder) - 1;
    if (*end != SAVE_STEAM_PATH_SEPARATOR)
    {
        do
        {
            *end = 0;
            end--;
        } while (*end != SAVE_STEAM_PATH_SEPARATOR);
    }
    sprintf(path, "%sSaveData\\%s", folder, name);
    return Save_LoadSteam(path, unused, name);
}
