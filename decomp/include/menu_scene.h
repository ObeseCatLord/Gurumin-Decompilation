// Globals of the menu scene (Menu_DrawScene, src/menu_scene.cpp): the 3D map drawn behind the menus.
// This is the one declaration of each of them: include this header, never redeclare one.
#ifndef GURUMIN_MENU_SCENE_H
#define GURUMIN_MENU_SCENE_H

#include "matrix4.h"
#include "node.h"
#include "vec4.h"

extern "C" int DAT_104c0220;                 // non-zero while a menu has set up the scene's render target itself (set by Menu_Main and Menu_SaveLoad)
extern "C" int DAT_10c63384;                 // non-zero = the player model is not drawn in the menu scene
extern "C" int DAT_10c6336c;                 // non-zero = the player model is not drawn in the menu scene
extern "C" int DAT_102027f0;                 // non-zero = the swap target preview is drawn after the scene
extern "C" int DAT_108360f4;                 // texture slot of the swap target that the scene is drawn into
extern "C" Node *DAT_1021c634;               // the map's "ClearORoot" node (found on first use)
extern "C" int DAT_1020c320;                 // texture slot of the shadow of the first and third caster
extern "C" int DAT_102028e4;                 // texture slot of the first haze layer of the water regions
extern "C" Vec4 DAT_10803ab8[8];             // the shadow casters' positions: two per row, four per two casters
extern "C" Node DAT_108654f8;                // the scene's model tree (searched for the node the shadows are drawn from)

int Menu_DrawScene();                                      // 0x10022830
Matrix4 Scaling(float x, float y, float z);                // 0x101b13c0
Matrix4 TranslationMatrix(float x, float y, float z);      // 0x101b1360

#endif
