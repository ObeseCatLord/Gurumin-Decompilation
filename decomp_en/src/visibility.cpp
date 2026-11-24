// Map visibility (EN build). Ported from the matched JP source decomp/src/visibility.cpp.
#include "node.h"
#include "visibility.h"
#include "char.h"
#include "anim_model.h"

// Vis_IsVisible: 1 when `pos` lies in a currently visible cell (or the map has no visibility data), else 0. Used to
// skip updating or drawing objects that cannot be seen.
extern "C" int Vis_IsVisible(Vec4 pos)
{
    VisData *visibilityCells = g_mapRootNode->visibilityCells;
    if (visibilityCells == 0)
        return 1;
    Box2 *box = visibilityCells->boxes;
    for (int i = 0; i < VIS_CELL_COUNT; i++, box++)
    {
        if (g_visibleCells[i] != 0 && pos.InBox(box))
            return 1;
    }
    return 0;
}

// Char_IsInVisibleCell: 1 when character `index` stands in a currently visible cell (or the map has no visibility
// data), else 0. Called when the character models are updated.
extern "C" int Char_IsInVisibleCell(int index)
{
    Vec4 position;
    if (g_mapRootNode->visibilityCells == 0)
        return 1;
    position.Copy(g_characterModels[index].matrix.GetTranslation());
    Box2 *box = g_mapRootNode->visibilityCells->boxes;
    for (int i = 0; i < VIS_CELL_COUNT; i++, box++)
    {
        if (g_visibleCells[i] != 0 && position.InBox(box))
            return 1;
    }
    return 0;
}
