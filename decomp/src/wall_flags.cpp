// Map-specific invisible walls. Some maps contain probe nodes named kabe_atari_<dir> ("wall hit", one per
// compass direction) and wall nodes wall_<dir>; a wall is made solid (node flag 0x20) while the player's
// movement touches its probe.

#include "vec4.h"

#include "node.h"

extern "C" Node *DAT_10255f08;
extern "C" Vec4 DAT_118e69e8;
extern "C" Vec4 DAT_118e6cb8;
extern "C" int DAT_10c62ed0;
extern "C" int DAT_10c63370;

// Makes `wallName` solid while the player (position DAT_118e69e8, velocity DAT_118e6cb8) touches the probe
// node `probeName`, and passable otherwise.
static __forceinline void UpdateWall(const char *probeName, const char *wallName, Vec4 *hit)
{
    Node *probe = DAT_10255f08->Find(probeName, 0);
    Node *wall = DAT_10255f08->Find(wallName, 0);
    if (probe != 0 && wall != 0)
    {
        if (probe->Collide(0, -1, &DAT_118e69e8, &DAT_118e6cb8, hit) != 0)
            wall->SetFlags(0x20);
        else
            wall->ClearFlags(0x20);
    }
}

// Clears the visible flag (4) of node `name`.
static __forceinline void HideNode(const char *name)
{
    Node *node = DAT_10255f08->Find(name, 0);
    if (node != 0)
        node->ClearFlags(4);
}

// Updates the four walls of the current map. On map 0x18 it also hides the four probe meshes once
// (DAT_10c63370 remembers that this was done).
extern "C" int FUN_100182a0(void)
{
    Vec4 hit;
    if (DAT_10255f08 != 0)
    {
        UpdateWall("kabe_atari_w", "wall_w", &hit);
        UpdateWall("kabe_atari_n", "wall_n", &hit);
        UpdateWall("kabe_atari_e", "wall_e", &hit);
        UpdateWall("kabe_atari_s", "wall_s", &hit);
        if (DAT_10c62ed0 == 0x18 && DAT_10c63370 == 0)
        {
            DAT_10c63370 = 1;
            HideNode("@kabe_atari_w");
            HideNode("@kabe_atari_s");
            HideNode("@kabe_atari_n");
            HideNode("@kabe_atari_e");
        }
    }
    return 0;
}
