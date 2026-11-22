// Map regions: the world's "@CDL" nodes, each with a rectangle on the ground (two corners of its bounds, in no
// particular order).

#include <string.h>

// windows.h-style min/max (vec4.cpp): each argument is evaluated twice.
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

#include "vec4.h"

// A scene node (scene_node.cpp Node): its name and the corners of its bounds.
struct Node
{
    char name[0x6e4];         // 0x000: the node's name (the struct starts with it)
    float cornerAx;           // 0x6e4
    float cornerAy;           // 0x6e8
    char pad_06ec[8];
    float cornerBx;           // 0x6f4
    float cornerBy;           // 0x6f8
    Node *Find(const char *name, int index);    // 0x1019ece0: the index-th node called `name` in the subtree, or 0
};

extern "C" Node *DAT_10255f08;   // the world's root node

// 0x10018100 (Map_FindRegionAt). Returns the first "@CDL" region whose rectangle contains `point` (x and y) with a
// 50-unit margin on every side, leaving out the regions whose names contain "eggobj", "croud" or "fire"; 0 when
// there is none or no world is loaded.
extern "C" Node *FUN_10018100(const Vec4 *point)
{
    int i = 0;
    Vec4 low;
    Vec4 high;
    while (DAT_10255f08 != 0)
    {
        Node *region = DAT_10255f08->Find("@CDL", i);
        if (region == 0)
            return 0;
        if (strstr(region->name, "eggobj") == 0 && strstr(region->name, "croud") == 0 &&
            strstr(region->name, "fire") == 0)
        {
            low.x = MIN(region->cornerAx, region->cornerBx);
            low.y = MIN(region->cornerAy, region->cornerBy);
            high.x = MAX(region->cornerAx, region->cornerBx);
            high.y = MAX(region->cornerAy, region->cornerBy);
            if (low.x + 50.0f < point->x && high.x - 50.0f > point->x && low.y + 50.0f < point->y &&
                high.y - 50.0f > point->y)
                return region;
        }
        i++;
    }
    return 0;
}
