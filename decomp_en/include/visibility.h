// Visibility cells of a map (EN build): which parts of the map are visible from the player's cell.
// This is the one declaration of VisCell, VisData and the visibility globals in decomp_en.
#ifndef GURUMIN_EN_VISIBILITY_H
#define GURUMIN_EN_VISIBILITY_H

#include "vec4.h"

struct Node;

// Visibility cells per map.
const int VIS_CELL_COUNT = 0x40;

// One visibility cell: its neighbour count and neighbour numbers.
struct VisCell
{
    unsigned char count;                  // 0x00: neighbours
    unsigned char neighbours[0x3f];       // 0x01: cell numbers of the neighbours
};

// The visibility data of a map root node.
struct VisData
{
    VisCell cells[VIS_CELL_COUNT];        // 0x0000: the cells
    Box2 boxes[VIS_CELL_COUNT];           // 0x1000: the box of each cell
};

extern "C" Node *g_mapRootNode;                     // EN 0x00927104 (JP DAT_10255f08): root node of the current map
extern "C" char g_regionsOnlyEnabled;                  // EN 0x0217faff (JP DAT_11b04514): 1 while a node with visibility cells is drawn: only enabled collision regions are drawn
extern "C" char g_visibleCells[VIS_CELL_COUNT];     // EN 0x0217fb08 (JP DAT_11afffe8): 1 for each cell currently visible

// For each character slot, 1 for each cell box within CELL_NEAR_MARGIN units of the character (rows of 0x40). EN 0x00a1e200 (JP DAT_1021d338).
extern "C" char g_charNearCells[][VIS_CELL_COUNT];

// Margin around a cell box inside which a character counts as near the cell.
const float CELL_NEAR_MARGIN = 75.0f;

// Marks the cell boxes near character slot `index` in its row of g_charNearCells and points the character's nearCells
// at that row. Returns 1 when the map has no visibility data (nearCells stays 0), else 0. EN 0x5a67f0 (JP 0x100157b0).
extern "C" int Char_UpdateNearCells(int index);

// 1 when `pos` lies in a currently visible cell (or the map has no visibility data), else 0. EN 0x5a67a0.
extern "C" int Vis_IsVisible(Vec4 pos);

// 1 when character slot `index` stands in a currently visible cell (or the map has no visibility data). EN 0x5a68a0 (JP 0x10015710).
extern "C" int Char_IsInVisibleCell(int index);

#endif
