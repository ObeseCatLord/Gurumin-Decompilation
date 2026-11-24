// Map-specific invisible walls (EN build). Ported from the matched JP source decomp/src/wall_flags.cpp.
// Some maps contain probe nodes named kabe_atari_<dir> ("wall hit", one per compass direction) and wall nodes
// wall_<dir>; a wall is made hidden (node flag NODE_FLAG_HIDDEN, see node.h) while the camera's movement touches its probe.
#include "camera.h"
#include "game_vars.h"
#include "node.h"
#include "visibility.h"
#include "wall_flags.h"

// Map on which the probe meshes are hidden once the walls are set up.
const int MAP_WITH_HIDDEN_PROBES = 0x18;
// Collide mask that accepts every node.
const unsigned int COLLIDE_ANY_NODE = 0xffffffff;

// Hides the wall `wallName` while the camera path touches the probe node `probeName`, shows it otherwise.
static void UpdateWall(const char *probeName, const char *wallName, Vec4 *hit)
{
    Node *probe = g_mapRootNode->Find(probeName, 0);
    Node *wall = g_mapRootNode->Find(wallName, 0);
    if (probe != 0 && wall != 0)
    {
        if (probe->Collide(0, COLLIDE_ANY_NODE, &g_cameraPositionNow, &g_cameraTargetNow, hit) != 0)
            wall->SetFlags(NODE_FLAG_HIDDEN);
        else
            wall->ClearFlags(NODE_FLAG_HIDDEN);
    }
}

// Clears the keep-mesh flag (4) of node `name`, which hides the probe mesh.
static void HideNode(const char *name)
{
    Node *node = g_mapRootNode->Find(name, 0);
    if (node != 0)
        node->ClearFlags(NODE_FLAG_KEEP_MESH);
}

// Map_UpdateWalls: updates the four walls of the current map. On map 0x18 it also hides the four probe meshes once.
// Returns 0. Called every frame by the map update.
extern "C" int Map_UpdateWalls(void)
{
    Vec4 hit;
    if (g_mapRootNode != 0)
    {
        UpdateWall("kabe_atari_w", "wall_w", &hit);
        UpdateWall("kabe_atari_n", "wall_n", &hit);
        UpdateWall("kabe_atari_e", "wall_e", &hit);
        UpdateWall("kabe_atari_s", "wall_s", &hit);
        if (g_currentMap == MAP_WITH_HIDDEN_PROBES && g_wallProbesHidden == 0)
        {
            g_wallProbesHidden = 1;
            HideNode("@kabe_atari_w");
            HideNode("@kabe_atari_s");
            HideNode("@kabe_atari_n");
            HideNode("@kabe_atari_e");
        }
    }
    return 0;
}
