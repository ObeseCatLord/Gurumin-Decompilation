// Reading the summary of a save file for the load menu (EN build, JP twin Save_ReadSlotInfo in save_slot.cpp).
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game_file.h"
#include "itm_scramble.h"
#include "save_body.h"
#include "save_slot.h"
#include "steam_storage.h"

// Size of the path buffers, and the character that ends a folder name.
const int SAVE_INFO_PATH_SIZE = 0x100;
const char SAVE_INFO_PATH_SEPARATOR = '\\';
// The first word of a scrambled save, and of an old save that is not scrambled.
const int SAVE_MAGIC_SCRAMBLED = 9999;
const int SAVE_MAGIC_PLAIN = 0x36b1;
// A scrambled save: the header (magic and key), and the trailer after the body (offset, sum and xor check).
const int SAVE_HEADER_BYTES = 8;
const int SAVE_TRAILER_BYTES = 0xc;
// The key adds to every second one of the first four characters of the descrambling name.
const int SAVE_KEY_STEP = 2;
const int SAVE_KEY_CHARACTERS = 4;
// Old saves: the three seeks that skip from the angle to the player record (in bytes).
const int SAVE_OLD_SKIP_A = 4;
const int SAVE_OLD_SKIP_B = 0x140;
const int SAVE_OLD_SKIP_C = 0x14;
// Chapter numbers the summary reports for the extra modes, and the map that ends the game.
const int SLOT_MODE_PLAIN = 0;
const int SLOT_MODE_EXTRA_A = 1;
const int SLOT_MODE_EXTRA_B = 2;
const int SLOT_CHAPTER_EXTRA_A = 3;
const int SLOT_CHAPTER_EXTRA_B = 5;
const int SLOT_CHAPTER_ENDING = 4;
const int MAP_ENDING = 1000;
static const char SAVE_FOLDER_FORMAT[] = "%sSaveData\\%s";
static const char SAVE_NAME_POPON[] = "Popon";

// Save_ReadSlotInfo. Called by the load menu and the slot list with the name of a save file. Reads the whole file from
// the Steam cloud (or, when the cloud has none, from the SaveData folder), descrambles it with its base name (every
// second one of the first four characters shifted by the file's key) and takes the summary from it: map, play time,
// the player's name ("Popon" in costume 6) and the chapter in *mode (3 and 5 for the extra modes, 4 once the map is
// 1000). Old plain saves (first word 0x36b1) are read field by field. Returns 1, or 0 when the file is missing, has
// another first word or does not descramble.
int Save_ReadSlotInfo(const char *name, int *map, int *playTime, char *playerName, int *mode)
{
    *playTime = 0;
    *map = 0;
    if (mode != 0)
        *mode = 0;
    *playerName = 0;

    GameFile file;
    char folder[SAVE_INFO_PATH_SIZE];
    GetModuleFileNameA(0, folder, SAVE_INFO_PATH_SIZE);
    char *end = folder + strlen(folder) - 1;
    if (*end != SAVE_INFO_PATH_SEPARATOR)
    {
        do
        {
            *end = 0;
            end--;
        } while (*end != SAVE_INFO_PATH_SEPARATOR);
    }
    char path[SAVE_INFO_PATH_SIZE + 4];
    sprintf(path, SAVE_FOLDER_FORMAT, folder, name);

    if (SteamRemoteStorage()->FileExists(name))
    {
        SavedPlayer player;
        bool inCloud = false;
        int length;                  // the body size, later the Popon flag
        int slotMap;
        unsigned char *buffer;
        int chapter;
        int y;
        int angle;
        int z;
        int x;
        int size = SteamRemoteStorage()->GetFileSize(name);
        if (size != 0)
            inCloud = true;
        else if (file.handle != 0)
        {
            file.Seek(0, SEEK_END);
            size = file.Size();
            file.Seek(0, SEEK_SET);
        }
        length = size - SAVE_HEADER_BYTES - SAVE_TRAILER_BYTES;
        buffer = (unsigned char *)operator new(size);
        if (inCloud)
            SteamRemoteStorage()->FileRead(name, buffer, size);
        else
        {
            file.Read(buffer, 1, size);
            file.Close();
        }
        int magic = *(int *)buffer;
        if (magic == SAVE_MAGIC_SCRAMBLED)
        {
            int key = *(int *)(buffer + sizeof(int));
            SaveTrailer *trailer = (SaveTrailer *)(buffer + SAVE_HEADER_BYTES + length);
            int sum = trailer->sum;
            unsigned int check = trailer->check;
            char baseName[SAVE_INFO_PATH_SIZE];
            Path_GetBaseNameWorldCopy(baseName, name);
            for (int i = 0; i < SAVE_KEY_CHARACTERS; i++)
                baseName[i * SAVE_KEY_STEP] += (char)key;
            SaveBody *body = (SaveBody *)(buffer + SAVE_HEADER_BYTES);
            if (Itm_Descramble((unsigned char *)body, length, baseName, sum, check) == 0)
            {
                free(buffer);
                return 0;
            }
            slotMap = body->map;
            x = body->x;
            y = body->y;
            z = body->z;
            angle = body->angle;
            player = body->player;
            int *variables = (int *)operator new(SAVE_SCRIPT_BYTES);
            memcpy(variables, body->scriptVariables, SAVE_SCRIPT_BYTES);
            chapter = variables[SAVE_VAR_CHAPTER];
            int modeVariable = variables[SAVE_VAR_MODE];
            length = variables[SAVE_VAR_COSTUME] == SAVE_COSTUME_POPON ? 1 : 0;
            free(variables);
            *playTime = body->playTime;
            if (mode != 0)
            {
                *mode = chapter;
                if (modeVariable == SLOT_MODE_EXTRA_A)
                    *mode = SLOT_CHAPTER_EXTRA_A;
                else if (modeVariable == SLOT_MODE_EXTRA_B)
                    *mode = SLOT_CHAPTER_EXTRA_B;
            }
            *map = slotMap;
            if (slotMap == MAP_ENDING)
                *mode = SLOT_CHAPTER_ENDING;
            free(buffer);
        }
        else
        {
            if (magic != SAVE_MAGIC_PLAIN)
            {
                file.Close();
                return 0;
            }
            file.Seek(SAVE_THUMBNAIL_BYTES, SEEK_CUR);
            file.Read(&slotMap, 1, sizeof(slotMap));
            file.Read(&x, 1, sizeof(x));
            file.Read(&y, 1, sizeof(y));
            file.Read(&z, 1, sizeof(z));
            file.Read(&angle, 1, sizeof(angle));
            file.Seek(SAVE_OLD_SKIP_A, SEEK_CUR);
            file.Seek(SAVE_OLD_SKIP_B, SEEK_CUR);
            file.Seek(SAVE_OLD_SKIP_C, SEEK_CUR);
            file.Read(&player, 1, sizeof(player));
            int *variables = (int *)operator new(SAVE_SCRIPT_BYTES);
            file.Read(variables, 1, SAVE_SCRIPT_BYTES);
            chapter = variables[SAVE_VAR_CHAPTER];
            int modeVariable = variables[SAVE_VAR_MODE];
            length = variables[SAVE_VAR_COSTUME] == SAVE_COSTUME_POPON ? 1 : 0;
            free(variables);
            file.Seek(SAVE_BYTES_AFTER_SCRIPT, SEEK_CUR);
            file.Read(playTime, 1, sizeof(*playTime));
            if (mode != 0)
            {
                *mode = chapter;
                if (modeVariable == SLOT_MODE_EXTRA_A)
                    *mode = SLOT_CHAPTER_EXTRA_A;
                else if (modeVariable == SLOT_MODE_EXTRA_B)
                    *mode = SLOT_CHAPTER_EXTRA_B;
            }
            int ignored;
            file.Read(&ignored, 1, sizeof(ignored));
            *map = slotMap;
            file.Close();
        }
        strcpy(playerName, player.name);
        if (length != 0)
            strcpy(playerName, SAVE_NAME_POPON);
        return 1;
    }
    return 0;
}
