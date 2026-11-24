// Small model, view and table helpers.

#include <string.h>

#include "model_misc.h"
#include "world_renderer.h"
#include "mesh_ref.h"








#include "render_vars.h"

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

// 0x101687c0. Counts the caption timer down to 0.
void WorldRenderer::TickCaption()
{
    if (captionVisible != 0)
        captionVisible--;
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
    modelAlias[slot] = character;
    characters[character].m_0265 = 1;
    modelPartner[slot] = value;
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
extern "C" int FUN_10165020(float *keys, float *a, float *b, int *c, int count)
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
                    float t = a[i];
                    a[i] = a[i + 1];
                    a[i + 1] = t;
                }
                if (b != 0)
                {
                    float t = b[i];
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
