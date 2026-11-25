// The loading indicator (EN build). Ported from decomp/src/loading_tick.cpp: while data loads, the loaders call this to
// keep a small spinner, a progress bar and the percentage on screen (in the top-left 64x64 corner).
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "dialogs.h"
#include "game.h"
#include "loading.h"
#include "render.h"
#include "texture_mgr.h"

// Milliseconds between two redraws is this plus one (redraw when more than this many have passed).
const int LOADING_MAX_TICK_GAP = 33;
// Spinner turn per redraw (radians).
const double LOADING_SPINNER_STEP = 0.02;
// Spinner cell: size and its cell in the menu sheet (u0, v0, u1, v1).
const int LOADING_CORNER_SIZE = 64;
const int LOADING_SPINNER_SIZE = 48;
const float LOADING_SPINNER_U0 = 0.0f;
const float LOADING_SPINNER_V0 = 0.5f;
const float LOADING_SPINNER_U1 = 0.1875f;
const float LOADING_SPINNER_V1 = 0.6875f;
// Progress bar: position, width at 100 percent plus its end caps, and its cell in the menu sheet.
const int LOADING_BAR_LEFT = 2;
const int LOADING_BAR_TOP = 51;
const int LOADING_BAR_BOTTOM = 57;
const float LOADING_BAR_WIDTH = 44.0f;
const float LOADING_BAR_PERCENT = 100.0f;
const float LOADING_BAR_CAPS = 2.0f;
const float LOADING_BAR_U0 = 0.75390625f;
const float LOADING_BAR_V0 = 0.25390625f;
const float LOADING_BAR_U1 = 0.7578125f;
const float LOADING_BAR_V1 = 0.2578125f;
// Frame drawn over the bar: top and its cell in the menu sheet.
const int LOADING_FRAME_TOP = 48;
const float LOADING_FRAME_U0 = 0.5f;
const float LOADING_FRAME_V0 = 0.25f;
const float LOADING_FRAME_U1 = 0.6875f;
const float LOADING_FRAME_V1 = 0.3125f;
// Percentage text position.
const int LOADING_TEXT_X = 8;
const int LOADING_TEXT_Y = 45;
const int LOADING_TEXT_SIZE = 20;

// Loading_Tick: at most every 34 ms, turns the spinner a little, draws it, the progress bar (44 pixels at 100 percent)
// and the percentage, presents the corner and handles the window messages. Returns 1, or 0 once the game is quitting.
// Called by the loaders while data loads.
extern "C" int Loading_Tick(void)
{
    char text[LOADING_TEXT_SIZE];
    if (g_quitRequested != 0)
        return 0;
    DWORD now = GetTickCount();
    if (abs((int)(g_loadingLastTick - now)) > LOADING_MAX_TICK_GAP)
    {
        g_loadingLastTick = now;
        g_offscreenTargetActive = 1;
        Render_ResetTarget();
        g_loadingSpinnerAngle += LOADING_SPINNER_STEP;
        g_offscreenTargetActive = 0;
        g_textures[g_dialogGame.texMenu1].DrawCell(0, 0, LOADING_SPINNER_SIZE, LOADING_SPINNER_SIZE,
                                                    LOADING_SPINNER_U0, LOADING_SPINNER_V0, LOADING_SPINNER_U1,
                                                    LOADING_SPINNER_V1, 0, 1.0f, g_loadingSpinnerAngle);
        g_textures[g_dialogGame.texMenu1].DrawCell(
            LOADING_BAR_LEFT, LOADING_BAR_TOP,
            (int)(g_loadingProgress * LOADING_BAR_WIDTH / LOADING_BAR_PERCENT + LOADING_BAR_CAPS), LOADING_BAR_BOTTOM,
            LOADING_BAR_U0, LOADING_BAR_V0, LOADING_BAR_U1, LOADING_BAR_V1, 0, 1.0f, 0);
        g_textures[g_dialogGame.texMenu1].DrawCell(0, LOADING_FRAME_TOP, LOADING_SPINNER_SIZE, LOADING_CORNER_SIZE,
                                                    LOADING_FRAME_U0, LOADING_FRAME_V0, LOADING_FRAME_U1,
                                                    LOADING_FRAME_V1, 0, 1.0f, 0);
        sprintf(text, "%d%%", (int)g_loadingProgress);
        g_dialogGame.DrawDebugText(text, LOADING_TEXT_X, LOADING_TEXT_Y);
        RECT corner;
        corner.left = 0;
        corner.top = 0;
        corner.right = LOADING_CORNER_SIZE;
        corner.bottom = LOADING_CORNER_SIZE;
        g_textureManager.Present(&corner);
        g_textureManager.PumpAndRecover();
        Render_RestoreTarget();
    }
    return 1;
}
