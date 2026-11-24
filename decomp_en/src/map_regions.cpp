// Finding the map region at a point (EN build). Ported from the matched JP source decomp/src/map_regions.cpp.
#include <string.h>
#include "collide_regions.h"

// Node name that marks a region (collision area) of the map.
const char REGION_NAME[] = "@CDL";

// Substrings of the region names that Map_FindRegionAt leaves out.
const char REGION_SKIP_EGG[] = "eggobj";
const char REGION_SKIP_CROWD[] = "croud";
const char REGION_SKIP_FIRE[] = "fire";

// Margin a point must keep from the edges of a region's rectangle to count as inside it.
const float REGION_POINT_MARGIN = 50.0f;

// Map_FindRegionAt: returns the first region whose x/y rectangle contains `point` with REGION_POINT_MARGIN to spare on
// every side, leaving out the egg, crowd and fire regions; 0 when there is none or no world is loaded.
extern "C" Node *Map_FindRegionAt(const Vec4 *point)
{
    int i = 0;
    Vec4 low;
    Vec4 high;
    while (g_worldRoot != 0)
    {
        Node *region = g_worldRoot->Find(REGION_NAME, i);
        if (region == 0)
            return 0;
        if (strstr(region->name, REGION_SKIP_EGG) == 0 && strstr(region->name, REGION_SKIP_CROWD) == 0 &&
            strstr(region->name, REGION_SKIP_FIRE) == 0)
        {
            low.x = region->box[NODE_BOX_MIN].x > region->box[NODE_BOX_MAX].x ? region->box[NODE_BOX_MAX].x : region->box[NODE_BOX_MIN].x;
            low.y = region->box[NODE_BOX_MIN].y > region->box[NODE_BOX_MAX].y ? region->box[NODE_BOX_MAX].y : region->box[NODE_BOX_MIN].y;
            high.x = region->box[NODE_BOX_MIN].x < region->box[NODE_BOX_MAX].x ? region->box[NODE_BOX_MAX].x : region->box[NODE_BOX_MIN].x;
            high.y = region->box[NODE_BOX_MIN].y < region->box[NODE_BOX_MAX].y ? region->box[NODE_BOX_MAX].y : region->box[NODE_BOX_MIN].y;
            if (low.x + REGION_POINT_MARGIN < point->x && high.x - REGION_POINT_MARGIN > point->x &&
                low.y + REGION_POINT_MARGIN < point->y && high.y - REGION_POINT_MARGIN > point->y)
                return region;
        }
        i++;
    }
    return 0;
}
