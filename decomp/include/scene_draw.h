// The frame's 3D scene (Scene_DrawFrame, src/scene_draw.cpp): the one declaration of its entry point. Callers:
// Menu_SaveLoad, TitleScreen_Update, Menu_Main and the mode 15 update.
#ifndef GURUMIN_SCENE_DRAW_H
#define GURUMIN_SCENE_DRAW_H

void Scene_DrawFrame(int unused);   // 0x10038c10: the argument is pushed by the callers and never read

#endif
