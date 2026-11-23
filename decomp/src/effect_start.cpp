// Starting an effect: an effect definition (by script id) gets a free record in one of the five effect lists
// (char_effects.cpp updates and draws them). This unit lays out the record fields the start fills in.

#include "vec4.h"

#include "matrix4.h"

#include "effect_manager.h"

extern "C" int DAT_110186a8[32];                     // recently started script ids
extern "C" int DAT_10fa73f8[32];                     // their definitions
extern "C" unsigned char DAT_1101de64;               // next entry of that cache
extern "C" int DAT_1101de60;                         // effects started so far
extern "C" int DAT_10ff1df8[0x47e];                  // 1 once a definition's resources are loaded
extern "C" int FUN_10161920(const char *name);       // Texture_FindByName

// 0x10172350 (EffectManager::Start). Starts the effect with script id `effect` at `position`: finds its definition
// (through a 32-entry cache), takes the first free record after the last one of the definition's list (wrapping
// once around the 0x348 records), links it at the list's end and fills it in: parent record, matrix owner, value,
// start frame, the caller's matrix and options (or defaults) and the pause flag. Looks up the texture and loads
// the definition's resources on first use. Returns 1 for effects of kind 2 and 4, 0 otherwise or when there is no
// definition or free record.
int EffectManager::Start(int effect, Vec4 position, int parent, int unused, const Matrix4 *owner, int value, int frame,
                         const Matrix4 *matrix, const EffectOptions *options, int *pause)
{
    int i;
    for (i = 0; i < 32; i++)
    {
        if (DAT_110186a8[i] == effect)
            break;
    }
    int definition;
    if (i == 32 || (definition = DAT_10fa73f8[i]) < 0)
    {
        for (definition = 0; definition < 0x47e; definition++)
        {
            if (DAT_1101dfdc[definition].id == effect)
                break;
        }
        if (definition == 0x47e)
            return 0;
    }
    DAT_110186a8[DAT_1101de64] = effect;
    DAT_10fa73f8[DAT_1101de64++] = definition;
    if (DAT_1101de64 > 0x1f)
        DAT_1101de64 = 0;
    int list;
    if (options != 0)
    {
        unsigned char own = DAT_1101dfdc[definition].list;
        if (own != 0)
            list = own;
        else
            list = options->list;
        if (DAT_1101dfdc[definition].fixedList != 0)
            list = own;
    }
    else
        list = DAT_1101dfdc[definition].list;
    int last = DAT_11018604[list];
    int candidate = last + 1;
    if (candidate < 0)
        candidate = 0;
    int index;
    int tries = 0;
    for (;;)
    {
        index = candidate;
        if (candidate >= 0x348)
            index = candidate - 0x348;
        if (DAT_1115cfe8[index].id == -1)
            break;
        tries++;
        candidate++;
        if (tries >= 0x348)
            return 0;
    }
    DAT_1115cfe8[index].prev = last;
    if (DAT_1101d820[list] == -1)
        DAT_1101d820[list] = index;
    if (last != -1)
        DAT_1115cfe8[last].next = index;
    DAT_1115cfe8[index].pause = pause;
    DAT_1115cfe8[index].next = -1;
    DAT_11018604[list] = index;
    if (DAT_1101dfdc[definition].texture < 0)
        DAT_1101dfdc[definition].texture = FUN_10161920(DAT_1101dfdc[definition].textureName);
    if (DAT_10ff1df8[definition] == 0)
        LoadDefinition(definition);
    DAT_1115cfe8[index].m_0188 = value;
    DAT_1115cfe8[index].matrixOwner = owner;
    DAT_1115cfe8[index].serial = DAT_1101de60;
    DAT_1115cfe8[index].parent = parent;
    DAT_1115cfe8[index].id = definition;
    DAT_1115cfe8[index].m_0010 = -1;
    DAT_1115cfe8[index].frame = DAT_1101dfdc[definition].startFrame + frame;
    DAT_1115cfe8[index].position.Copy(position);
    DAT_1115cfe8[index].velocity.Fill(0.0f);
    DAT_1115cfe8[index].m_001c = 0;
    DAT_1115cfe8[index].m_0018 = 0;
    DAT_1115cfe8[index].light = 0;
    if (options != 0)
    {
        DAT_1115cfe8[index].options = *options;
        if (DAT_1101dfdc[definition].list != 0)
            DAT_1115cfe8[index].options.list = DAT_1101dfdc[definition].list;
        if (DAT_1101dfdc[definition].fixedList != 0)
            DAT_1115cfe8[index].options.list = DAT_1101dfdc[definition].list;
    }
    else
    {
        DAT_1115cfe8[index].options.settings[4] = 0;
        DAT_1115cfe8[index].options.settings[0] = 0;
        DAT_1115cfe8[index].options.settings[1] = 0;
        DAT_1115cfe8[index].options.settings[2] = 0;
        DAT_1115cfe8[index].options.m_000e = 0;
        DAT_1115cfe8[index].options.settings[5] = 0;
        DAT_1115cfe8[index].options.m_0008 = 0;
        DAT_1115cfe8[index].options.settings[3] = 0;
        DAT_1115cfe8[index].options.settings[6] = 0;
        DAT_1115cfe8[index].options.m_0010 = 0;
        DAT_1115cfe8[index].options.m_0004 = 0;
        DAT_1115cfe8[index].options.m_0000 = 0;
        DAT_1115cfe8[index].options.owner = 0;
        DAT_1115cfe8[index].options.list = DAT_1101dfdc[definition].list;
    }
    DAT_1115cfe8[index].part = 1;
    DAT_1115cfe8[index].m_018c = 0;
    if (matrix != 0)
        DAT_1115cfe8[index].matrix = *matrix;
    else
        DAT_1115cfe8[index].matrix.SetIdentity();
    DAT_1101de60++;
    if (DAT_1101dfdc[definition].spawnKind == 2 || DAT_1101dfdc[definition].spawnKind == 4)
        return 1;
    return 0;
}
typedef char check_options[sizeof(EffectOptions) == 0x44 ? 1 : -1];
