// Effect definition file loader: reads the 0x47e effect definitions (EffectDef) from the game's effect file.

#include <stdio.h>

#include "tri_batch.h"
#include "effect_manager.h"
#include "game_file.h"
#include "key_track.h"
#include "texture_load.h"
#include "char_vars.h"
#include "render_vars.h"

// File version tags: the older files (20000) have a 0x20-byte block where the newer ones (22000) have 0x80.
#define EFFECT_FILE_VERSION_OLD 20000
#define EFFECT_FILE_VERSION_NEW 22000
// Size of a key bank as stored in the file, and the number of key tracks per definition.
#define EFFECT_DEF_TRACK_COUNT 12
// Every notification of the host comes after this many definitions.
#define EFFECT_LOAD_NOTIFY_EVERY 10

// 0x101886d0 (EffectManager::LoadDefinitions). Reads the effect definition file `path` into the free slots of the
// definition table DAT_1101dfdc and then loads their textures and resources. The file is a version tag followed by
// definitions, each led by its script id (a negative id ends the list). Twelve key banks per definition are read as
// raw KeyBanks and copied into the definition's tracks. The first fields of a definition are stored in the tail of
// the slot before it (the real record starts 0x174 bytes before the script id), hence the [slot - 1] accesses.
// Returns 1, or 0 when the file cannot be opened or has no known version tag.
int EffectManager::LoadDefinitions(const char *path)
{
    GameFile file;
    int loaded = 0;
    DAT_1101c804.SetBuffer(DAT_10fc6a90, 0x400);
    FILE *opened = file.Open(path, "rb");
    if (opened)
    {
        int version;
        file.Read(&version, 1, 4);
        if (version == EFFECT_FILE_VERSION_NEW || version == EFFECT_FILE_VERSION_OLD)
        {
            do
            {
                if (loaded % EFFECT_LOAD_NOTIFY_EVERY == 0 && DAT_10f65538 != 0)
                    DAT_10f65538(0, 0);
                int slot;
                for (slot = 0; slot < EFFECT_DEFINITION_COUNT; slot++)
                {
                    if (DAT_1101dfdc[slot].id == EFFECT_NONE)
                        break;
                }
                if (slot == EFFECT_DEFINITION_COUNT)
                {
                    fread(0, 1, 1, 0);
                    break;
                }
                int id;
                file.Read(&id, 1, 4);
                if (id < 0)
                    break;
                DAT_1101dfdc[slot].id = id;
                file.Read(&DAT_1101dfdc[slot].kind, 1, 4);
                file.Read(&DAT_1101dfdc[slot].m_0008, 1, 4);
                file.Read(DAT_1101dfdc[slot - 1].m_0264[0], 1, 0x10);
                file.Read(DAT_1101dfdc[slot - 1].m_0264[1], 1, 0x10);
                file.Read(DAT_1101dfdc[slot - 1].m_0264[2], 1, 0x10);
                file.Read(DAT_1101dfdc[slot - 1].m_0264[3], 1, 0x10);
                file.Read(DAT_1101dfdc[slot - 1].m_0264[4], 1, 0x10);
                file.Read(DAT_1101dfdc[slot - 1].m_0264[5], 1, 0x10);
                file.Read(DAT_1101dfdc[slot - 1].m_0264[6], 1, 0x10);
                file.Read(DAT_1101dfdc[slot - 1].m_0264[7], 1, 0x10);
                file.Read(&DAT_1101dfdc[slot].m_000c, 1, 4);
                file.Read(&DAT_1101dfdc[slot].m_0010, 1, 4);
                file.Read(&DAT_1101dfdc[slot].m_0014, 1, 4);
                KeyBank bank;
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&DAT_1101dfdc[slot - 1].tracks[0]);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&DAT_1101dfdc[slot - 1].tracks[1]);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&DAT_1101dfdc[slot - 1].tracks[2]);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&DAT_1101dfdc[slot - 1].tracks[3]);
                file.Read(&DAT_1101dfdc[slot].trackFlags[0], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&DAT_1101dfdc[slot - 1].tracks[4]);
                file.Read(&DAT_1101dfdc[slot].trackFlags[1], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&DAT_1101dfdc[slot - 1].tracks[5]);
                file.Read(&DAT_1101dfdc[slot].trackFlags[2], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&DAT_1101dfdc[slot - 1].tracks[6]);
                file.Read(&DAT_1101dfdc[slot].trackFlags[3], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&DAT_1101dfdc[slot - 1].tracks[7]);
                file.Read(&DAT_1101dfdc[slot].trackFlags[4], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&DAT_1101dfdc[slot - 1].tracks[8]);
                file.Read(&DAT_1101dfdc[slot].trackFlags[5], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&DAT_1101dfdc[slot - 1].tracks[9]);
                file.Read(&DAT_1101dfdc[slot].trackFlags[6], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&DAT_1101dfdc[slot - 1].tracks[10]);
                file.Read(&DAT_1101dfdc[slot].startFrame, 1, 4);
                if (version == EFFECT_FILE_VERSION_OLD)
                    file.Read(&DAT_1101dfdc[slot].m_007c, 1, 0x20);
                else
                    file.Read(&DAT_1101dfdc[slot].m_007c, 1, 0x80);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&DAT_1101dfdc[slot - 1].tracks[11]);
                file.Read(&DAT_1101dfdc[slot].texture, 1, 4);
                file.Read(&DAT_1101dfdc[slot].blend, 1, 4);
                file.Read(DAT_1101dfdc[slot].m_0104, 1, 0x10);
                file.Read(DAT_1101dfdc[slot].m_0114, 1, 0x40);
                file.Read(&DAT_1101dfdc[slot].m_0154, 1, 0x10);
                file.Read(&DAT_1101dfdc[slot].m_0164, 1, 0x10);
                file.Read(DAT_1101dfdc[slot].m_0174, 1, 0x40);
                file.Read(DAT_1101dfdc[slot].m_01b4, 1, 0x40);
                file.Read(&DAT_1101dfdc[slot].spawnKind, 1, 4);
                file.Read(DAT_1101dfdc[slot].name, 1, 0x20);
                file.Read(DAT_1101dfdc[slot].textureName, 1, 0x20);
                file.Read(&DAT_1101dfdc[slot].textureName[0x20], 1, 0x20);
                file.Read(DAT_1101dfdc[slot].m_0018, 1, 0x40);
                file.Read(&DAT_1101dfdc[slot].list, 1, 1);
                file.Read(&DAT_1101dfdc[slot].flat, 1, 1);
                file.Read(&DAT_1101dfdc[slot - 1].m_03d4, 1, 4);
                file.Read(&DAT_1101dfdc[slot].m_0261, 1, 1);
                file.Read(DAT_1101dfdc[slot].m_0058, 1, 0x10);
                file.Read(&DAT_1101dfdc[slot].m_0068, 1, 0x10);
                loaded++;
                DAT_10ff1df8[slot] = 0;
            } while (loaded < EFFECT_DEFINITION_COUNT);
            file.Close();
            ClearTables();
            int i;
            for (i = 0; i < EFFECT_DEFINITION_COUNT; i++)
            {
                if (DAT_1101dfdc[i].texture < 0)
                    FUN_10174ce0(DAT_1101dfdc[i].textureName, 0, 0);
            }
            for (i = 0; i < EFFECT_DEFINITION_COUNT; i++)
            {
                if (DAT_10ff1df8[i] == 0)
                    LoadDefinition(i);
            }
            for (i = 0; i < 32; i++)
                DAT_110186a8[i] = -1;
            DAT_1101de64 = 0;
            DAT_1101d820[0] = -1;
            DAT_1101d820[1] = -1;
            DAT_1101d820[2] = -1;
            DAT_1101d820[3] = -1;
            DAT_1101d820[4] = -1;
            DAT_11018604[0] = -1;
            DAT_11018604[1] = -1;
            DAT_11018604[2] = -1;
            DAT_11018604[3] = -1;
            DAT_11018604[4] = -1;
            return 1;
        }
        file.Close();
    }
    return 0;
}
