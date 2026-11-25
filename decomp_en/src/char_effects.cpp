// The effect manager's list handling (EN build; JP: decomp/src/char_effects.cpp): Reset, RemoveChildren and
// RemoveOfType. Effects are records in g_effectRecords, linked into EFFECT_LIST_COUNT lists.

#include <string.h>

#include "effect_manager.h"
#include "effect_nodes.h"
#include "texture_mgr.h"

// Stops every effect: rebinds the effect batch to its buffer, clears all effect tables and marks their slots free
// (-1), and resets the six group offsets. The offset table has only five values in the original; the sixth group
// gets whatever was on the stack (kept as is). EN 0x734ce0 (JP 0x10184170).
int EffectManager::Reset()
{
    int offsets[EFFECT_GROUP_COUNT];
    g_effectBatch.SetBuffer(g_effectBatchVertices, EFFECT_BATCH_CAPACITY);
    runningCount = 0;
    memset(g_effectLights, 0, sizeof(EffectLight) * EFFECT_LIGHT_COUNT);
    memset(g_effectRecords, 0, EFFECT_RECORD_COUNT * sizeof(EffectRecord));
    memset(g_effectSpritesA, 0, EFFECT_SPRITE_COUNT * sizeof(EffectSprite));
    memset(g_effectSpritesB, 0, EFFECT_SPRITE_COUNT * sizeof(EffectSprite));
    int i;
    for (i = 0; i < EFFECT_RECORD_COUNT; i++)
        g_effectRecords[i].id = EFFECT_NONE;
    for (i = 0; i < EFFECT_SPRITE_COUNT; i++)
        g_effectSpritesA[i].type = EFFECT_NONE;
    for (i = 0; i < EFFECT_SPRITE_COUNT; i++)
        g_effectSpritesB[i].type = EFFECT_NONE;
    for (i = 0; i < EFFECT_LIST_COUNT; i++)
        g_effectListFirst[i] = EFFECT_NONE;
    for (i = 0; i < EFFECT_LIST_COUNT; i++)
        g_effectListLast[i] = EFFECT_NONE;
    offsets[0] = 0;
    offsets[1] = EFFECT_GROUP_START_1;
    offsets[2] = EFFECT_GROUP_START_2;
    offsets[3] = EFFECT_GROUP_START_3;
    offsets[4] = EFFECT_GROUP_START_4;
    for (i = 0; i < EFFECT_GROUP_COUNT; i++)
    {
        groupCurrent[i] = offsets[i];
        groupStart[i] = offsets[i];
    }
    return 0;
}

// Stops every running effect attached to record `parent`, and the effects attached to those in turn: each one is
// marked free, unlinked from its list, detached and its light entry cleared. Returns 0. EN 0x747950 (JP 0x10172260).
int EffectManager::RemoveChildren(int parent)
{
    for (int list = 0; list < EFFECT_LIST_COUNT; list++)
    {
        for (int i = g_effectListFirst[list]; i > -1; i = g_effectRecords[i].next)
        {
            if (g_effectRecords[i].id != EFFECT_NONE && g_effectRecords[i].parent == parent)
            {
                g_effectRecords[i].id = EFFECT_NONE;
                if (g_effectRecords[i].prev == -1)
                    g_effectListFirst[list] = g_effectRecords[i].next;
                else
                    g_effectRecords[g_effectRecords[i].prev].next = g_effectRecords[i].next;
                if (g_effectRecords[i].next != -1)
                    g_effectRecords[g_effectRecords[i].next].prev = g_effectRecords[i].prev;
                else
                    g_effectListLast[list] = g_effectRecords[i].prev;
                g_effectRecords[i].parent = -1;
                if (g_effectRecords[i].light != EFFECT_NO_LIGHT)
                    g_effectLights[g_effectRecords[i].light].active = 0;
                RemoveChildren(i);
            }
        }
    }
    return 0;
}

// Stops every running effect whose definition has type `type` (the record's id indexes g_effectDefs), including the
// effects attached to it. Returns 0. EN 0x747a50 (JP 0x10188490).
int EffectManager::RemoveOfType(int type)
{
    for (int list = 0; list < EFFECT_LIST_COUNT; list++)
    {
        for (int i = g_effectListFirst[list]; i > -1; i = g_effectRecords[i].next)
        {
            if (g_effectRecords[i].id > -1 && g_effectDefs[g_effectRecords[i].id].id == type)
            {
                g_effectRecords[i].id = EFFECT_NONE;
                if (g_effectRecords[i].prev == -1)
                    g_effectListFirst[list] = g_effectRecords[i].next;
                else
                    g_effectRecords[g_effectRecords[i].prev].next = g_effectRecords[i].next;
                if (g_effectRecords[i].next != -1)
                    g_effectRecords[g_effectRecords[i].next].prev = g_effectRecords[i].prev;
                else
                    g_effectListLast[list] = g_effectRecords[i].prev;
                if (g_effectRecords[i].light != EFFECT_NO_LIGHT)
                    g_effectLights[g_effectRecords[i].light].active = 0;
                RemoveChildren(i);
            }
        }
    }
    return 0;
}

// Stops every running effect whose start options name `owner` (character index + 100), including the effects attached
// to it. Called when a character is removed or its effects must end. Returns 0. EN 0x747b50 (JP 0x101883b0).
int EffectManager::RemoveOwnedBy(int owner)
{
    for (int list = 0; list < EFFECT_LIST_COUNT; list++)
    {
        for (int i = g_effectListFirst[list]; i > -1; i = g_effectRecords[i].next)
        {
            if (g_effectRecords[i].id > -1 && g_effectRecords[i].options.owner == owner)
            {
                g_effectRecords[i].id = EFFECT_NONE;
                if (g_effectRecords[i].prev == -1)
                    g_effectListFirst[list] = g_effectRecords[i].next;
                else
                    g_effectRecords[g_effectRecords[i].prev].next = g_effectRecords[i].next;
                if (g_effectRecords[i].next != -1)
                    g_effectRecords[g_effectRecords[i].next].prev = g_effectRecords[i].prev;
                else
                    g_effectListLast[list] = g_effectRecords[i].prev;
                if (g_effectRecords[i].light != EFFECT_NO_LIGHT)
                    g_effectLights[g_effectRecords[i].light].active = 0;
                RemoveChildren(i);
            }
        }
    }
    return 0;
}

// Full initialisation at load time: the colour becomes white, the effect batch is bound to its buffer, the used nodes
// of the effect node pool are released, every table is cleared and marked free, the lists are emptied, the six group
// offsets set and the default definition added. The sixth offset is uninitialised in the original, as in Reset.
// Returns 0. EN 0x734e60 (JP 0x101842a0). Called when the game starts.
int EffectManager::Init()
{
    int offsets[EFFECT_GROUP_COUNT];
    colour.Set(1.0f, 1.0f, 1.0f, 1.0f);
    g_effectBatch.SetBuffer(g_effectBatchVertices, EFFECT_BATCH_CAPACITY);
    for (int node = 0; node < EFFECT_NODE_COUNT; node++)
    {
        if (g_effectNodeLoaded[node] != 0)
            g_effectNodes[node].Release();
    }
    runningCount = 0;
    memset(g_effectLights, 0, sizeof(EffectLight) * EFFECT_LIGHT_COUNT);
    memset(g_effectRecords, 0, EFFECT_RECORD_COUNT * sizeof(EffectRecord));
    memset(g_effectDefStorage, 0, sizeof(g_effectDefStorage));
    memset(g_effectSpritesA, 0, EFFECT_SPRITE_COUNT * sizeof(EffectSprite));
    memset(g_effectSpritesB, 0, EFFECT_SPRITE_COUNT * sizeof(EffectSprite));
    int i;
    for (i = 0; i < EFFECT_RECENT_COUNT; i++)
        g_recentEffectIds[i] = EFFECT_NONE;
    g_recentEffectNext = 0;
    for (i = 0; i < EFFECT_RECORD_COUNT; i++)
        g_effectRecords[i].id = EFFECT_NONE;
    for (i = 0; i < EFFECT_SPRITE_COUNT; i++)
        g_effectSpritesA[i].type = EFFECT_NONE;
    for (i = 0; i < EFFECT_SPRITE_COUNT; i++)
        g_effectSpritesB[i].type = EFFECT_NONE;
    for (i = 0; i < EFFECT_DEFINITION_COUNT; i++)
    {
        g_effectDefs[i].id = EFFECT_NONE;
        g_effectDefs[i].texture = EFFECT_NONE;
    }
    for (i = 0; i < EFFECT_LIST_COUNT; i++)
        g_effectListFirst[i] = EFFECT_NONE;
    for (i = 0; i < EFFECT_LIST_COUNT; i++)
        g_effectListLast[i] = EFFECT_NONE;
    ClearTables();
    offsets[0] = 0;
    offsets[1] = EFFECT_GROUP_START_1;
    offsets[2] = EFFECT_GROUP_START_2;
    offsets[3] = EFFECT_GROUP_START_3;
    offsets[4] = EFFECT_GROUP_START_4;
    for (i = 0; i < EFFECT_GROUP_COUNT; i++)
    {
        groupCurrent[i] = offsets[i];
        groupStart[i] = offsets[i];
    }
    AddDefaultDefinition();
    return 0;
}
