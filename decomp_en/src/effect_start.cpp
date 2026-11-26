// Starting an effect (EN build, JP twin effect_start.cpp): an effect definition (by script id) gets a free record in one of
// the five effect lists (effect_update.cpp steps and draws them).
#include <string.h>

#include "effect_manager.h"
#include "effect_nodes.h"
#include "engine.h"
#include "node.h"
#include "texture_mgr.h"

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

// Texture_FindByName as an inline function of its header: the slot holding file `name` (case-insensitive), or -1.
static inline int Texture_FindByNameInline(const char *name)
{
    if (*name == 0)
        return -1;
    for (int i = 0; i < TEXTURE_SLOT_COUNT; i++)
    {
        if (_stricmp(g_textureNames[i], name) == 0)
            return i;
    }
    return -1;
}

// Effect kinds (EffectDef::spawnKind) whose Start returns 1.
const int SPAWN_KIND_RETURNS_ONE_A = 2;
const int SPAWN_KIND_RETURNS_ONE_B = 4;

// EffectManager::Start. Starts the effect with script id `effect` at `position`: finds its definition (through a
// 32-entry cache of recently used ids), takes the first free record after the last one of the definition's list (wrapping
// once around the records), links it at the list's end and fills it in: parent record, matrix owner, value, start frame,
// the caller's matrix and options (or defaults) and the pause flag. Looks up the texture and loads the definition's
// model on first use. Returns 1 for effects of kind 2 and 4, 0 otherwise or when there is no definition or free record.
// Called by the characters, the weapons and the scripts.
int EffectManager::Start(int effect, Vec4 position, int parent, int unused, const Matrix4 *owner, int value, int frame,
                         const Matrix4 *matrix, const EffectOptions *options, int *pause)
{
    int i;
    for (i = 0; i < EFFECT_RECENT_COUNT; i++)
    {
        if (g_recentEffectIds[i] == effect)
            break;
    }
    int definition;
    if (i == EFFECT_RECENT_COUNT || (definition = g_recentEffectDefinitions[i]) < 0)
    {
        for (definition = 0; definition < EFFECT_DEFINITION_COUNT; definition++)
        {
            if (g_effectDefs[definition].id == effect)
                break;
        }
        if (definition == EFFECT_DEFINITION_COUNT)
            return 0;
    }
    g_recentEffectIds[g_recentEffectNext] = effect;
    g_recentEffectDefinitions[g_recentEffectNext++] = definition;
    if (g_recentEffectNext > EFFECT_RECENT_COUNT - 1)
        g_recentEffectNext = 0;
    int list;
    if (options != 0)
    {
        unsigned char own = g_effectDefs[definition].list;
        if (own != 0)
            list = own;
        else
            list = options->list;
        if (g_effectDefs[definition].fixedList != 0)
            list = own;
    }
    else
        list = g_effectDefs[definition].list;
    int last = g_effectListLast[list];
    int candidate = last + 1;
    if (candidate < 0)
        candidate = 0;
    int index;
    int tries = 0;
    for (;;)
    {
        index = candidate + tries;
        if (index >= EFFECT_RECORD_COUNT)
            index -= EFFECT_RECORD_COUNT;
        if (g_effectRecords[index].id == EFFECT_NONE)
            break;
        tries++;
        if (tries >= EFFECT_RECORD_COUNT)
            return 0;
    }
    g_effectRecords[index].prev = last;
    if (g_effectListFirst[list] == -1)
        g_effectListFirst[list] = index;
    if (last != -1)
        g_effectRecords[last].next = index;
    g_effectRecords[index].next = -1;
    g_effectRecords[index].pause = pause;
    g_effectListLast[list] = index;
    if (g_effectDefs[definition].texture < 0)
        g_effectDefs[definition].texture = Texture_FindByNameInline(g_effectDefs[definition].textureName);
    if (g_effectNodeLoaded[definition] == 0)
        Effect_LoadModelInline(definition);
    g_effectRecords[index].matrixOwner = owner;
    g_effectRecords[index].value = value;
    g_effectRecords[index].serial = g_effectSerial;
    g_effectRecords[index].frame = g_effectDefs[definition].startFrame + frame;
    g_effectRecords[index].parent = parent;
    g_effectRecords[index].id = definition;
    g_effectRecords[index].m_0010 = -1;
    g_effectRecords[index].position.Copy(position);
    g_effectRecords[index].velocity.Fill(0.0f);
    g_effectRecords[index].m_001c = 0;
    g_effectRecords[index].m_0018 = 0;
    g_effectRecords[index].light = 0;
    if (options != 0)
    {
        g_effectRecords[index].options = *options;
        if (g_effectDefs[definition].list != 0)
            g_effectRecords[index].options.list = g_effectDefs[definition].list;
        if (g_effectDefs[definition].fixedList != 0)
            g_effectRecords[index].options.list = g_effectDefs[definition].list;
    }
    else
    {
        g_effectRecords[index].options.settings[4] = 0;
        g_effectRecords[index].options.settings[0] = 0;
        g_effectRecords[index].options.settings[1] = 0;
        g_effectRecords[index].options.settings[2] = 0;
        g_effectRecords[index].options.m_000e = 0;
        g_effectRecords[index].options.settings[5] = 0;
        g_effectRecords[index].options.scale = 0;
        g_effectRecords[index].options.settings[3] = 0;
        g_effectRecords[index].options.settings[6] = 0;
        g_effectRecords[index].options.m_0010 = 0;
        g_effectRecords[index].options.colour = 0;
        g_effectRecords[index].options.m_0000 = 0;
        g_effectRecords[index].options.owner = 0;
        g_effectRecords[index].options.list = g_effectDefs[definition].list;
    }
    g_effectRecords[index].part = 1;
    g_effectRecords[index].m_018c = 0;
    if (matrix != 0)
        g_effectRecords[index].matrix = *matrix;
    else
        g_effectRecords[index].matrix.SetIdentity();
    g_effectSerial++;
    if (g_effectDefs[definition].spawnKind == SPAWN_KIND_RETURNS_ONE_A ||
        g_effectDefs[definition].spawnKind == SPAWN_KIND_RETURNS_ONE_B)
        return 1;
    return 0;
}
