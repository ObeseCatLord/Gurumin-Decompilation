// Model bank: the global object at 0x10804bf8 that holds up to 16 extra models (map objects, effects) with their
// own model records (same 0x249c-byte layout as the model part of a Character), plus a list of the scene's light
// nodes. Loaded per map; the bank is only active when `enabled` is set.
#include <string.h>

struct Node
{
    Node *Find(const char *name, int index);  // 0x1019ece0: the index-th node called `name` in the subtree, or 0
};

// One model record (the CharModel part of a Character, see world.h / texture_mgr.cpp SceneOwner).
struct BankModel
{
    char pad_0000[0x260];
    float scale;            // 0x260: model scale, 1.0 when added
    char pad_0264[0xc];
    int index;              // 0x270: the record's own index in the bank
    char pad_0274[0x2078];
    int id;                 // 0x22ec: model id (0 = free record)
    int id2;                // 0x22f0: copy of the id
    char pad_22f4[0x9c];
    int timer;              // 0x2390: 99999 = no timeout
    char pad_2394[0x108];
};

struct ModelBank
{
    Node *lights[0x20];     // 0x0000: light nodes of the scene (Omn..., then Spot...)
    int m_0080[0x80];       // 0x0080
    char pad_0280[0x1604];
    int m_1884;             // 0x1884
    char pad_1888[0xb160];
    BankModel models[16];   // 0xc9e8
    int params[16];         // 0x313a8: per-model parameter given when the model was added
    int m_313e8;            // 0x313e8
    int enabled;            // 0x313ec: 1 when the current map uses the bank
    int Reset();
    int CollectLights(Node *root);
    int Add(int id, int param);
    int M_10164af0(BankModel *model);
};

// Empties the bank: clears the light list, the 0x80-entry table, every model record's id and the counters.
int ModelBank::Reset()
{
    for (int j = 0; j < 0x80; j++)
        m_0080[j] = 0;
    m_1884 = 0;
    for (int i = 0; i < 16; i++)
        models[i].id = 0;
    m_313e8 = 0;
    memset(lights, 0, sizeof(lights));
    return 1;
}

// Rebuilds the light list from the scene below `root`: up to 32 nodes whose names start with "Omn" (omni lights)
// followed by up to 32 starting with "Spot" (spot lights); the leading '@' makes Node::Find a prefix search. Missing lights are skipped, so the list stays packed. Returns 0 when the bank
// is not in use, else 1. (The list has 32 entries; with more than 32 lights in total it overruns into m_0080.)
int ModelBank::CollectLights(Node *root)
{
    if (enabled == 0)
        return 0;
    memset(lights, 0, sizeof(lights));
    int count = 0;
    int i;
    for (i = 0; i < 0x20; i++)
    {
        lights[count] = root->Find("@Omn", i);
        if (lights[count] != 0)
            count++;
    }
    Node **slot;
    for (i = 0, slot = &lights[count]; i < 0x20; i++)
    {
        *slot = root->Find("@Spot", i);
        if (*slot != 0)
            slot++;
    }
    return 1;
}

// Adds model `id` to the first free record (scale 1, no timeout), remembers `param` for it and sets the record up
// (M_10164af0). Returns 0 when all 16 records are in use.
int ModelBank::Add(int id, int param)
{
    for (int i = 0; i < 16; i++)
    {
        if (models[i].id == 0)
        {
            models[i].id = id;
            models[i].id2 = id;
            models[i].scale = 1.0f;
            models[i].timer = 99999;
            models[i].index = i;
            params[i] = param;
            M_10164af0(&models[i]);
            return 1;
        }
    }
    return 0;
}
