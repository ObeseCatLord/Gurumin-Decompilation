// Small model, view and table helpers.

#include <string.h>

struct Collider
{
    int M_101ab3e0(int a, int b);
};

struct ModelObject
{
    char pad_0000[0x268];
    int m_0268;
    int m_026c;
    char pad_0270[0x207c];
    Collider *collider;
    char pad_22f0[0x2c];
    int m_231c;
    char pad_2320[0x1550];
    int m_3870;

    int M_10162f20(int value);
    int M_10163e70(int value);
    int M_10164090(int a, int b);
    void M_101687c0();
};

struct ViewLimits
{
    char pad_0000[0x20];
    int m_0020;
    char pad_0024[0x54];
    float minimum[3];
    int pad_0084;
    float maximum[3];
    char pad_0094[0x24];
    int m_00b8;
    void M_10166870();
};

struct SlotList
{
    char data[0x14];
    void Clear();
};

struct SlotLists
{
    SlotList lists[5];
    int m_0064;
    int m_0068;
    int m_006c;
    void M_10169620();
};

struct CharacterInfo
{
    char pad_0000[0x265];
    char m_0265;
    char pad_0266[0x2236];
};

struct CharacterBank
{
    char pad_0000[0x8860];
    CharacterInfo *characters;
    char pad_8864[0xe23f4];
    int m_eac58[0x80];
    int m_eae58[0x80];
    int M_10166910(int slot, int character, int value);
};

struct MeshRef
{
    char pad_0000[0x4c];
    int index;
};

extern "C" int DAT_10feb3e0[0xc80];
extern "C" char DAT_10fada90[0xc80][0x20];

// Sets the model's blend value; any non-zero value selects blend mode 0x17, zero turns blending off.
int ModelObject::M_10162f20(int value)
{
    m_026c = 0x17;
    m_0268 = value;
    if (value == 0)
        m_026c = 0;
    return 0;
}

// Sets m_231c. Returns 1.
int ModelObject::M_10163e70(int value)
{
    m_231c = value;
    return 1;
}

// Forwards a collision query (a, b) to the model's collider; 0 when it has none.
int ModelObject::M_10164090(int a, int b)
{
    if (collider != 0)
        return collider->M_101ab3e0(a, b);
    return 0;
}

// Counts the m_3870 timer down to 0.
void ModelObject::M_101687c0()
{
    if (m_3870 != 0)
        m_3870--;
}

// Resets the rotation limits to +-45 degrees (0.785375 rad) on all three axes.
void ViewLimits::M_10166870()
{
    m_00b8 = 0;
    minimum[0] = -0.785375f;
    minimum[1] = -0.785375f;
    minimum[2] = -0.785375f;
    maximum[0] = 0.785375f;
    maximum[1] = 0.785375f;
    maximum[2] = 0.785375f;
    m_0020 = 0;
}

// Assigns character `character` to slot `slot` with `value` and flags its model as important (motion
// changes are flagged while applied, see SceneOwner::PlayMotion).
int CharacterBank::M_10166910(int slot, int character, int value)
{
    m_eac58[slot] = character;
    characters[character].m_0265 = 1;
    m_eae58[slot] = value;
    return 1;
}

// Clears the five index lists and their counters.
void SlotLists::M_10169620()
{
    lists[0].Clear();
    lists[1].Clear();
    lists[2].Clear();
    lists[3].Clear();
    lists[4].Clear();
    m_0064 = 0;
    m_0068 = 0;
    m_006c = 0;
}

// Clears the shared model name table (0xc80 names) and its reference counts. Not byte-identical yet.
extern "C" int FUN_10169720(void)
{
    memset(DAT_10feb3e0, 0, sizeof(DAT_10feb3e0));
    for (int i = 0; i < 0xc80; i++)
        DAT_10fada90[i][0] = 0;
    return 0;
}

// Returns the first free entry of the shared model name table, or -1.
extern "C" int FUN_10169760(void)
{
    for (int i = 0; i < 0xc80; i++)
    {
        if (DAT_10fada90[i][0] == 0)
            return i;
    }
    return -1;
}

// Adds a reference to a shared model file and returns the new count (see FUN_10169990).
extern "C" int FUN_10169970(const MeshRef *mesh)
{
    int index = mesh->index;
    return ++DAT_10feb3e0[index];
}

#include "key_track.h"

#include "node_data.h"

// Empties the five tracks and gives each its default key at frame 0: track 0 = (0, 0, 0, 0), track 1 =
// (0, 0, 0, 1), track 2 = (1, 1, 1, 0), track 3 = (1, 1, 1, 1), track 4 = (0, 0, 0, 0).
void NodeExtra::Init()
{
    m_0070[0] = 0;
    m_0070[1] = 0;
    m_0070[2] = 0;
    m_0070[3] = 0;
    tracks[NODE_TRACK_TRANSLATION].Init();
    tracks[NODE_TRACK_ROTATION].Init();
    tracks[NODE_TRACK_SCALE].Init();
    tracks[NODE_TRACK_COLOUR].Init();
    tracks[NODE_TRACK_EXTRA].Init();
    translationOverride = 0;
    rotationOverride = 0;
    scaleOverride = 0;
    tracks[NODE_TRACK_TRANSLATION].Insert(0, 0, 0, 0, 0);
    tracks[NODE_TRACK_ROTATION].Insert(0, 0, 0, 0, 1.0f);
    tracks[NODE_TRACK_SCALE].Insert(0, 1.0f, 1.0f, 1.0f, 0);
    tracks[NODE_TRACK_COLOUR].Insert(0, 1.0f, 1.0f, 1.0f, 1.0f);
    tracks[NODE_TRACK_EXTRA].Insert(0, 0, 0, 0, 0);
}

// Sorts `count` keys into ascending order (bubble sort) and applies every swap to the parallel arrays `a` and `b`
// (either may be 0) and `c`. Used to sort draw lists by depth. Returns 1. Not byte-identical yet: the original keeps
// count - 1 in a local of its own without unrolling the loop (see open.tsv).
extern "C" int FUN_10165020(float *keys, int *a, int *b, int *c, int count)
{
    int swapped;
    do
    {
        swapped = 0;
        for (int i = 0; i < count - 1; i++)
        {
            if (keys[i] > keys[i + 1])
            {
                float key = keys[i];
                keys[i] = keys[i + 1];
                keys[i + 1] = key;
                if (a != 0)
                {
                    int t = a[i];
                    a[i] = a[i + 1];
                    a[i + 1] = t;
                }
                if (b != 0)
                {
                    int t = b[i];
                    b[i] = b[i + 1];
                    b[i + 1] = t;
                }
                int t = c[i];
                c[i] = c[i + 1];
                c[i + 1] = t;
                swapped = 1;
            }
        }
    } while (swapped);
    return 1;
}
