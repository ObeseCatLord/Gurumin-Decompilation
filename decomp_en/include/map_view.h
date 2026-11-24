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

// Pass state of the overhead view. Overhead pass on (JP DAT_102022c4); render targets work (JP DAT_10f654cc); the map
// pass is running (JP DAT_10f65520); the map is drawn with its model hidden (JP DAT_10f655bc); the effect pass is
// running (JP DAT_10daaad0).
extern "C" int g_overheadPassOn;        // EN 0x00908394
extern "C" int g_renderTargetsWork;     // EN 0x021e4800
extern "C" int g_mapPassRunning;        // EN 0x021e4828
extern "C" int g_mapHidden;             // EN 0x021a34f4
extern "C" int g_effectPassRunning;     // EN 0x01487fe8
// Per area switch: non-zero = the "CDL<nn>" collision region of the map is on. EN 0x0217f8f8 (JP DAT_11afffe8).
extern "C" char g_regionSwitches[100];
// Draw area of the overhead pass. EN 0x02200ef8 (JP DAT_10f66918).
extern "C" Box2 g_mapDrawArea;
// Draw list of the overhead pass. EN 0x015e9eb8 (JP DAT_10836ec8).
extern "C" DrawList g_mapDrawList;
// View matrix of the camera. EN 0x00da9b00 (JP DAT_10836330).
extern "C" Matrix4 g_cameraView;
// Camera view times the projection, and times the base projection. EN 0x00da7900, 0x00da9c00 (JP DAT_108392d8, DAT_10836cd8).
extern "C" Matrix4 g_cameraViewProjection;
extern "C" Matrix4 g_cameraViewBase;
// Projection and base projection matrices. EN 0x00dac940, 0x00d75d18 (JP DAT_10866da8, DAT_104c03f8).
extern "C" Matrix4 g_projection;
extern "C" Matrix4 g_baseProjection;

// Draws the map's overhead pass: the map into the pass's draw list over the visible area widened by these margins.
const float MAP_DRAW_BELOW = 1200.0f;
const float MAP_DRAW_ABOVE = 2000.0f;
const float MAP_DRAW_MARGIN = 1100.0f;

// Draws one pass of the draw list `list` with `view` from `eye`. EN 0x751300 (JP 0x101999a0).
extern "C" int DrawList_DrawPass(DrawList *list, Matrix4 *view, Vec4 *eye, int flag);
// Builds the overhead view of the map, then rebuilds the camera matrices. EN 0x5dcea0.
void Map_DrawTopView(void);

// Builds the overhead projection of the map into `out` (times `matrix`), sets the pit and visible areas and projects the
// camera position and target. Returns the map's node. EN 0x5c9750.
Node *Map_BuildTopView(Matrix4 *out, const Matrix4 &matrix);

#endif
