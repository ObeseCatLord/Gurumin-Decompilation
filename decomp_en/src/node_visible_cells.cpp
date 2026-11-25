// Visible cells of a map (EN build; JP twin: Node::UpdateVisibleCells, 0x10199be0).
#include <string.h>
#include "node.h"
#include "visibility.h"
#include "camera.h"
#include "map_view.h"
#include "render.h"

// Marks the visibility cells that can be seen: the cell the camera target lies in (its box compared at the back buffer's
// height over width ratio) and the neighbours listed for that cell. Clears every mark first. Does nothing and returns 0
// for a node without visibility data. Called by the draw and update passes of the map root. Returns 1.
int Node::UpdateVisibleCells()
{
    if (visibilityCells != 0)
    {
        Vec4 target;
        Box2 *box = visibilityCells->boxes;
        if (g_mapPassRunning != 0)
            target.Copy(g_cameraTargetMapPass);
        else
            target.Copy(g_cameraTarget);
        memset(g_visibleCells, 0, VIS_CELL_COUNT);
        int cell = 0;
        do
        {
            if (visibilityCells->cells[cell].count != 0 &&
                target.InBoxAspect(box, (float)g_deviceBackBufferHeight / (float)g_deviceBackBufferWidth))
            {
                g_visibleCells[cell] = 1;
                for (int n = 0; n < visibilityCells->cells[cell].count; n++)
                    g_visibleCells[visibilityCells->cells[cell].neighbours[n]] = 1;
            }
            cell++;
            box++;
        } while (cell < VIS_CELL_COUNT);
        return 1;
    }
    return 0;
}
