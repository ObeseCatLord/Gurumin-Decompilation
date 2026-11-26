// Water region visibility (EN build). Ported from the matched JP source decomp/src/region_view.cpp.
#include "map_view.h"
#include "region_view.h"
#include "shaga.h"
#include <math.h>

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

// Wave constants of Region_WaveVertex: the phase flip (pi), the per-axis phase speed per frame, the frames after
// which each axis phase repeats, and the amplitude of the wave on each axis.
const float REGION_WAVE_FLIP = 3.1415f;
const float REGION_WAVE_SPEED_X = 0.026179167f;
const float REGION_WAVE_SPEED_Y = 0.020943332f;
const float REGION_WAVE_SPEED_Z = 0.017648876f;
const unsigned int REGION_WAVE_PERIOD_X = 240;
const unsigned int REGION_WAVE_PERIOD_Y = 300;
const unsigned int REGION_WAVE_PERIOD_Z = 356;
const float REGION_WAVE_AMPLITUDE_XY = 15.0f;
const float REGION_WAVE_AMPLITUDE_Z = 10.0f;
// Hundreds of a point's base coordinate are even or odd: that is the parity the wave phase depends on.
const int REGION_WAVE_PARITY_DIVISOR = 100;

// Region_WaveVertex: moves vertex `vertex` of region `region` by a sine wave per axis. The phase of each axis follows
// the frame counter and is shifted by pi when exactly one of the two parities of the point's coordinates is odd.
// Called by the region update (EN 0x5b3ac0).
int Region_WaveVertex(int region, int vertex)
{
    MapRegion *regionData = &g_mapRegions[region];
    int oddCount = 0;
    if (((int)regionData->points[vertex].m_0060 / REGION_WAVE_PARITY_DIVISOR & 1) != 0)
        oddCount = 1;
    if (((int)regionData->points[vertex].m_0064 / REGION_WAVE_PARITY_DIVISOR & 1) != 0)
        oddCount++;
    unsigned int frame = g_colourPlugFrames;
    float phaseFlip = (float)(oddCount & 1) * REGION_WAVE_FLIP;
    float phaseY = (float)(frame % REGION_WAVE_PERIOD_Y) * REGION_WAVE_SPEED_Y + phaseFlip;
    float phaseZ = (float)(frame % REGION_WAVE_PERIOD_Z) * REGION_WAVE_SPEED_Z + phaseFlip;
    float phaseX = (float)(frame % REGION_WAVE_PERIOD_X) * REGION_WAVE_SPEED_X + phaseFlip;
    RegionVertex *target = &g_mapRegions[region].vertices[vertex];
    target->x += (float)sin(phaseX) * REGION_WAVE_AMPLITUDE_XY;
    target = &g_mapRegions[region].vertices[vertex];
    target->y += (float)sin(phaseY) * REGION_WAVE_AMPLITUDE_XY;
    target = &g_mapRegions[region].vertices[vertex];
    target->z += (float)sin(phaseZ) * REGION_WAVE_AMPLITUDE_Z;
    return 0;
}
