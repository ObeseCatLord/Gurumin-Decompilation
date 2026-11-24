// Water region visibility (EN build). Ported from the matched JP source decomp/src/region_view.cpp.
#include "map_view.h"
#include "region_view.h"

// View volume margin the region corners are tested with.
const float REGION_VIEW_MARGIN = 50.0f;

// Region_IsVisible: 1 when one of the four corners of region `index` (at half its height) lies in the view volume
// widened by 50, else 0 (also for hidden regions). Called by the region update.
extern "C" int Region_IsVisible(int index)
{
    Vec4 corner;
    if (g_mapRegions[index].hidden != 0)
        return 0;
    float middle = g_mapRegions[index].max.z + g_mapRegions[index].min.z;
    middle *= 0.5f;
    corner.Copy(g_mapRegions[index].min);
    corner.z = middle;
    if (View_ContainsPoint(&corner, &g_cameraViewProjection, REGION_VIEW_MARGIN) != 0)
        return 1;
    corner.Copy(g_mapRegions[index].max);
    corner.z = middle;
    if (View_ContainsPoint(&corner, &g_cameraViewProjection, REGION_VIEW_MARGIN) != 0)
        return 1;
    corner.x = g_mapRegions[index].min.x;
    corner.y = g_mapRegions[index].max.y;
    corner.z = middle;
    if (View_ContainsPoint(&corner, &g_cameraViewProjection, REGION_VIEW_MARGIN) != 0)
        return 1;
    corner.x = g_mapRegions[index].max.x;
    corner.y = g_mapRegions[index].min.y;
    corner.z = middle;
    return View_ContainsPoint(&corner, &g_cameraViewProjection, REGION_VIEW_MARGIN) != 0;
}
