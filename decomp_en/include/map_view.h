// Overhead (top) view of the map (EN build; JP: map_view.cpp). This is the one declaration of these globals and
// functions in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_MAP_VIEW_H
#define GURUMIN_EN_MAP_VIEW_H

#include "node.h"

// Number of children a map root has when its model is the first child.
const int MAP_ROOT_CHILD_COUNT = 0x20;
// The view height is the middle of the model's box.
const float MAP_VIEW_HEIGHT_HALF = 0.5f;
// The projection's z scale (-1 mirrors z) and the factor of its w element.
const float MAP_PROJECTION_MIRROR = -1.0f;
const float MAP_PROJECTION_W_SCALE = 2.0f;

// Node of the map being played (a single child of it when it has 0x20 children). EN 0x009e334c (JP DAT_104c007c).
extern "C" Node *g_mapNode;
// Lowest z of the map's box. EN 0x0091d600 (JP DAT_104bfe24).
extern "C" float g_mapBottomZ;
// Height of the overhead view. EN 0x021a34c4 (JP DAT_10f65594).
extern "C" float g_topViewHeight;
// Pit area corners of the map's box. EN 0x013385b8 / 0x013385c8 (JP DAT_104d29e8 / DAT_104d29f8).
extern "C" Vec4 g_mapPitMin;
extern "C" Vec4 g_mapPitMax;
// Visible area at the view height. EN 0x021ffd08 (JP DAT_10f668a8).
extern "C" Box2 g_mapVisibleArea;

// Builds the overhead projection of the map into `out` (times `matrix`), sets the pit and visible areas and projects the
// camera position and target. Returns the map's node. EN 0x5c9750.
Node *Map_BuildTopView(Matrix4 *out, const Matrix4 &matrix);

#endif
