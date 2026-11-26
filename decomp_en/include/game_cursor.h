// The game's own mouse cursor (EN build, EN-only: no JP twin found).
#ifndef GURUMIN_EN_GAME_CURSOR_H
#define GURUMIN_EN_GAME_CURSOR_H

// Non-zero while the game draws its own cursor over the client area (set when a scene starts). EN 0x0090842e.
extern "C" char g_cursorDrawn;
// Direction the turning cursor points to: set to (1, 0) when a scene starts (roles inferred from the cursor code: the arrow
// turns by atan2(g_cursorAimX, -g_cursorAimY)). EN 0x00908764, 0x00a207d4.
extern "C" float g_cursorAimX;
extern "C" float g_cursorAimY;
// Non-zero = the plain arrow cursor is drawn, whatever the game mode. EN 0x00a1ddec.
extern "C" int g_cursorPlain;
// Not recovered; non-zero while it is set keeps the large turning cursor off. EN 0x009e33fa.
extern "C" char g_sceneFlag9e33fa;

// Non-zero keeps the game from drawing its cursor in GameCursor_DrawAtTarget (not recovered). EN 0x01336ab0.
extern "C" int g_cursorHidden;
// Point GameCursor_DrawAtTarget turns the cursor towards, in screen pixels. EN 0x00916fc8, 0x00916fcc.
extern "C" int g_cursorTargetX;
extern "C" int g_cursorTargetY;
// Not recovered; the turning cursor of GameCursor_DrawAtTarget shows only when it is outside UI_STATE_TURN_FIRST..UI_STATE_TURN_LAST. EN 0x0090812c.
extern "C" int g_uiState90812c;
const unsigned int UI_STATE_TURN_FIRST = 6;
const unsigned int UI_STATE_TURN_RANGE = 0x12;
// g_gameMode value in which GameCursor_DrawAtTarget turns the cursor.
const int CURSOR_TARGET_MODE = 0xf;

// Cell of the cursor sheet (a quarter-sheet grid: cursor cells are 32 pixels, at row 1 of the menu sheet 1).
const float CURSOR_ARROW_U0 = 0.0f;
const float CURSOR_ARROW_U1 = 0.125f;
const float CURSOR_ARROW_V0 = 0.25f;
const float CURSOR_ARROW_V1 = 0.375f;
const float CURSOR_TURN_U0 = 0.125f;
const float CURSOR_TURN_U1 = 0.25f;
const float CURSOR_ROW_V0 = 0.25f;
const float CURSOR_ROW_V1 = 0.375f;
// Pixels around the position: the plain cursor cell is CURSOR_PLAIN_SIZE wide, the turning cell CURSOR_TURN_HALF to each side.
const int CURSOR_PLAIN_SIZE = 0x20;
const int CURSOR_ARROW_LEFT = 4;          // the arrow cell starts this far left of the position
const int CURSOR_ARROW_HEIGHT = 0x24;
const int CURSOR_TURN_HALF = 0x10;
// Added to the heading: the cursor's turn is CURSOR_ANGLE_BASE minus the heading angle.
const float CURSOR_ANGLE_BASE = 3.926875f;
// x + y below this lets the turning cursor show.
const int CURSOR_TURN_LIMIT = 0x410;
// g_gameMode value in which the turning cursor shows.
const int CURSOR_TURN_MODE = 1;

// Draws the game's cursor at (x, y) when the Windows cursor is inside the client area (which is hidden then), else shows
// the Windows cursor again. Returns 1. No direct caller (a hook). EN 0x5de050 (see game_cursor.cpp).
extern "C" int GameCursor_Draw(int x, int y);

// Like GameCursor_Draw, but the turning cursor (game mode CURSOR_TARGET_MODE, only while g_overlayModelValueD is 0) points
// from the screen position towards (g_cursorTargetX, g_cursorTargetY), and nothing is drawn while g_cursorHidden is set.
// Returns 1. No direct caller (a hook). EN 0x6315e0 (see game_cursor.cpp).
extern "C" int GameCursor_DrawAtTarget(int x, int y);

#endif
