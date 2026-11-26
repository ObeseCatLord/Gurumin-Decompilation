// Menu and message input state of the EN build (written by the menu and message script commands). This is the one
// declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_MENU_STATE_H
#define GURUMIN_EN_MENU_STATE_H

// Value g_menuStateA363a0 takes when the menu input is reset.
const int MENU_STATE_RESET_VALUE = 1;

extern "C" int g_menuStateA363a0;   // EN 0x00a363a0: set to 1 when the menu input is reset, cleared by the frame update and the menu scripts (not recovered)
extern "C" int g_menuStateA363a4;   // EN 0x00a363a4: read by the menu drawing (0x5df1c0), cleared when the menu input is reset (not recovered)

extern "C" int g_menuOpenedA1da50;  // EN 0x00a1da50: set to 1 when MenuFade_Tick finishes opening the menu (reader not recovered)

// g_menuStateA363a0 values: the menu is opening (counter counts up), open, or closing (counter counts down).
const int MENU_STATE_OPENING = 1;
const int MENU_STATE_OPEN = 2;
const int MENU_STATE_CLOSING = 3;
// Counter value at which MenuFade_Tick counts the menu as open.
const int MENU_FADE_OPEN_AFTER = 15;
// Value MenuFade_Tick stores in g_menuOpenedA1da50 and returns once the menu is open.
const int MENU_FADE_OPENED = 1;
// Steps the menu opening or closing counter by a frame: an opening menu becomes open after MENU_FADE_OPEN_AFTER frames
// (sets g_menuOpenedA1da50), a closing one is gone when the counter reaches 0. Returns 1. EN 0x5b4710 (see menu_state.cpp).
extern "C" int MenuFade_Tick(void);

// Resets the menu input state: g_menuStateA363a0 to MENU_STATE_RESET_VALUE, g_menuStateA363a4, the remembered menu cursor
// and the message key wait to 0. Returns 0. EN 0x63d5a0. Called by the game script builtins (RunGameBuiltin) after a menu or message command.
extern "C" int MenuInput_Reset(void);

#endif
