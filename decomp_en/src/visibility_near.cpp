// Cells near a character (EN build). JP twin: decomp/src/visibility.cpp (FUN_100157b0, not matched in JP).
#include "node.h"
#include "visibility.h"
#include "char.h"
#include "anim_model.h"

// Char_UpdateNearCells: marks the cell boxes within CELL_NEAR_MARGIN units of character `index` in its row of
// g_charNearCells and points the character's nearCells at that row. Returns 1 when the map has no visibility data
// (nearCells stays 0), else 0. Called by the character update.
extern "C" int Char_UpdateNearCells(int index)
{
    Vec4 position;
    g_characterModels[index].nearCells = 0;
    if (g_mapRootNode->visibilityCells == 0)
        return 1;
    position.Copy(g_characterModels[index].matrix.GetTranslation());
    Box2 *box = g_mapRootNode->visibilityCells->boxes;
    for (int i = 0; i < VIS_CELL_COUNT; i++, box++)
    {
        g_charNearCells[index][i] = 0;
        if (position.InBoxMargin(box, CELL_NEAR_MARGIN) != 0)
            g_charNearCells[index][i] = 1;
    }
    g_characterModels[index].nearCells = g_charNearCells[index];
    return 0;
}
