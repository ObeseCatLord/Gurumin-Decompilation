// Effect definition file loader (EN build): reads the 0x47e effect definitions (EffectDef) from the game's effect file.
// Ported from the matched JP source decomp/src/effect_definition_loader.cpp.
#include <stdio.h>

#include "effect_manager.h"
#include "effect_nodes.h"
#include "engine.h"
#include "game_file.h"
#include "key_bank.h"
#include "node.h"
#include "texture_mgr.h"

// File version tags: the older files (20000) have a 0x20-byte block where the newer ones (22000) have 0x80.
const int EFFECT_FILE_VERSION_OLD = 20000;
const int EFFECT_FILE_VERSION_NEW = 22000;
// The host is told about the load after this many definitions.
const int EFFECT_LOAD_NOTIFY_EVERY = 10;

// Effect_LoadModel as an inline function of the header it comes from: the compiler expands it here.
static inline int Effect_LoadModelInline(int index)
{
    int kind = g_effectNames[index].kind;
    if (kind == EFFECT_KIND_MODEL_A || kind == EFFECT_KIND_MODEL_B || kind == EFFECT_KIND_MODEL_C)
    {
        g_effectNodes[index].LoadIt3(Path_Data(g_effectNames[index].name, 0), 0, 0);
        g_effectNodes[index].BuildMesh(1, 0);
        g_effectNodeLoaded[index] = 1;
    }
    return 0;
}

// EffectManager::LoadDefinitions: reads the effect definition file `path` into the free slots of the definition table
// g_effectDefs and then loads their textures and resources. The file is a version tag followed by definitions, each led by
// its script id (a negative id ends the list). Twelve key banks per definition are read as raw KeyBanks and copied into the
// definition's tracks. The first fields of a definition are stored in the tail of the slot before it (the real record
// starts 0x174 bytes before the script id), hence the [slot - 1] accesses. After the file: forgets the textures and
// sprites, loads the textures of definitions without one, builds the models of the model effects and resets the
// recent-effect cache and the effect lists. Returns 1, or 0 when the file cannot be opened or has no known version tag.
// Called when the effect file is (re)loaded.
int EffectManager::LoadDefinitions(const char *path)
{
    GameFile file;
    g_effectBatch.SetBuffer(g_effectBatchVertices, EFFECT_BATCH_CAPACITY);
    FILE *opened = file.Open(path, "rb");
    if (opened)
    {
        int version;
        file.Read(&version, 1, 4);
        if (version == EFFECT_FILE_VERSION_NEW || version == EFFECT_FILE_VERSION_OLD)
        {
            int slot;
            int loaded = 0;
            do
            {
                if (loaded % EFFECT_LOAD_NOTIFY_EVERY == 0 && g_modelLoadNotify != 0)
                    g_modelLoadNotify(0, 0);
                for (slot = 0; slot < EFFECT_DEFINITION_COUNT; slot++)
                {
                    if (g_effectDefs[slot].id == EFFECT_NONE)
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
                g_effectDefs[slot].id = id;
                file.Read(&g_effectDefs[slot].kind, 1, 4);
                file.Read(&g_effectDefs[slot].m_0008, 1, 4);
                file.Read(&g_effectDefs[slot - 1].m_0264[0], 1, 0x10);
                file.Read(&g_effectDefs[slot - 1].m_0264[1], 1, 0x10);
                file.Read(&g_effectDefs[slot - 1].m_0264[2], 1, 0x10);
                file.Read(&g_effectDefs[slot - 1].m_0264[3], 1, 0x10);
                file.Read(&g_effectDefs[slot - 1].m_0264[4], 1, 0x10);
                file.Read(&g_effectDefs[slot - 1].m_0264[5], 1, 0x10);
                file.Read(&g_effectDefs[slot - 1].m_0264[6], 1, 0x10);
                file.Read(&g_effectDefs[slot - 1].m_0264[7], 1, 0x10);
                file.Read(&g_effectDefs[slot].m_000c, 1, 4);
                file.Read(&g_effectDefs[slot].m_0010, 1, 4);
                file.Read(&g_effectDefs[slot].m_0014, 1, 4);
                KeyBank bank;
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&g_effectDefs[slot - 1].tracks[0]);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&g_effectDefs[slot - 1].tracks[1]);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&g_effectDefs[slot - 1].tracks[2]);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&g_effectDefs[slot - 1].tracks[3]);
                file.Read(&g_effectDefs[slot].trackFlags[0], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&g_effectDefs[slot - 1].tracks[4]);
                file.Read(&g_effectDefs[slot].trackFlags[1], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&g_effectDefs[slot - 1].tracks[5]);
                file.Read(&g_effectDefs[slot].trackFlags[2], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&g_effectDefs[slot - 1].tracks[6]);
                file.Read(&g_effectDefs[slot].trackFlags[3], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&g_effectDefs[slot - 1].tracks[7]);
                file.Read(&g_effectDefs[slot].trackFlags[4], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&g_effectDefs[slot - 1].tracks[8]);
                file.Read(&g_effectDefs[slot].trackFlags[5], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&g_effectDefs[slot - 1].tracks[9]);
                file.Read(&g_effectDefs[slot].trackFlags[6], 1, 1);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&g_effectDefs[slot - 1].tracks[10]);
                file.Read(&g_effectDefs[slot].startFrame, 1, 4);
                if (version == EFFECT_FILE_VERSION_OLD)
                    file.Read(&g_effectDefs[slot].m_007c, 1, 0x20);
                else
                    file.Read(&g_effectDefs[slot].m_007c, 1, 0x80);
                file.Read(&bank, 1, sizeof(KeyBank));
                bank.CopyTo(&g_effectDefs[slot - 1].tracks[11]);
                file.Read(&g_effectDefs[slot].texture, 1, 4);
                file.Read(&g_effectDefs[slot].blend, 1, 4);
                file.Read(g_effectDefs[slot].m_0104, 1, 0x10);
                file.Read(g_effectDefs[slot].m_0114, 1, 0x40);
                file.Read(&g_effectDefs[slot].m_0154, 1, 0x10);
                file.Read(&g_effectDefs[slot].m_0164, 1, 0x10);
                file.Read(g_effectDefs[slot].m_0174, 1, 0x40);
                file.Read(g_effectDefs[slot].m_01b4, 1, 0x40);
                file.Read(&g_effectDefs[slot].spawnKind, 1, 4);
                file.Read(g_effectDefs[slot].name, 1, 0x20);
                file.Read(g_effectDefs[slot].textureName, 1, 0x20);
                file.Read(&g_effectDefs[slot].textureName[0x20], 1, 0x20);
                file.Read(g_effectDefs[slot].m_0018, 1, 0x40);
                file.Read(&g_effectDefs[slot].list, 1, 1);
                file.Read(&g_effectDefs[slot].flat, 1, 1);
                file.Read(&g_effectDefs[slot - 1].m_03d4, 1, 4);
                file.Read(&g_effectDefs[slot].m_0261, 1, 1);
                file.Read(g_effectDefs[slot].m_0058, 1, 0x10);
                file.Read(&g_effectDefs[slot].m_0068, 1, 0x10);
                loaded++;
                g_effectNodeLoaded[slot] = 0;
            } while (loaded < EFFECT_DEFINITION_COUNT);
            file.Close();
            ClearTables();
            int i;
            for (i = 0; i < EFFECT_DEFINITION_COUNT; i++)
            {
                if (g_effectDefs[i].texture < 0)
                    Texture_LoadIti(g_effectDefs[i].textureName, 0, 0);
            }
            for (i = 0; i < EFFECT_DEFINITION_COUNT; i++)
            {
                if (g_effectNodeLoaded[i] == 0)
                    Effect_LoadModelInline(i);
            }
            for (i = 0; i < EFFECT_RECENT_COUNT; i++)
                g_recentEffectIds[i] = EFFECT_NONE;
            g_recentEffectNext = 0;
            g_effectListFirst[0] = EFFECT_NONE;
            g_effectListFirst[1] = EFFECT_NONE;
            g_effectListFirst[2] = EFFECT_NONE;
            g_effectListFirst[3] = EFFECT_NONE;
            g_effectListFirst[4] = EFFECT_NONE;
            g_effectListLast[0] = EFFECT_NONE;
            g_effectListLast[1] = EFFECT_NONE;
            g_effectListLast[2] = EFFECT_NONE;
            g_effectListLast[3] = EFFECT_NONE;
            g_effectListLast[4] = EFFECT_NONE;
            return 1;
        }
        file.Close();
    }
    return 0;
}
