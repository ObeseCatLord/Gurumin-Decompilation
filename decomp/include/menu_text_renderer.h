// MenuTextRenderer: declared here once (first used by src/game_panels.cpp).
#ifndef GURUMIN_MENU_TEXT_RENDERER_H
#define GURUMIN_MENU_TEXT_RENDERER_H

// The menu text renderer (Game::m_7664, see menu_list.cpp).
struct MenuTextRenderer
{
    int M_101501f0(int x, int y, const char *text, int colour, int a, float scaleX, float scaleY);  // draws text
};

#endif
