// The per-frame step of the game (EN build; JP twin APP.dll 10137b10, whose screenshot code the EN build lacks).
#include <stdlib.h>
#include <windows.h>

#include "anti_tamper.h"
#include "char.h"
#include "font_renderer.h"
#include "frame_step.h"
#include "hud_fade.h"
#include "input_device.h"
#include "mouse.h"
#include "render.h"
#include "script_queue.h"
#include "texture_mgr.h"
#include "vec4.h"

// Highest health the player can have.
const int PLAYER_HEALTH_LIMIT = 100;
// Anti-tamper rolls: a limit breach closes the game when rand() % TAMPER_ROLL_RANGE is below the threshold.
const int TAMPER_ROLL_RANGE = 50;
const int TAMPER_ROLL_WINDOW_THRESHOLD = 20;
const int TAMPER_ROLL_QUIT_THRESHOLD = 10;
// Message put up while the game is paused: where it is drawn, and its scale.
const int PAUSE_MESSAGE_X = 0x8d;
const int PAUSE_MESSAGE_Y = 0xc1;
const float PAUSE_MESSAGE_ALPHA = 1.0f;
const float PAUSE_MESSAGE_SCALE = 1.2f;
// Render_Command arguments that prepare the frame (command 7) and clear to the pause colour (command 4).
const int RENDER_COMMAND_BEGIN_FRAME = 7;
const int RENDER_COMMAND_FILL = 4;
const int PAUSE_FILL_COLOUR = 0xf7941d;

// Runs once per frame (callers: the game loop and the script). Pumps the messages (returning at once while the window
// is inactive), limits the player's health against tampering (a breach of the limit closes the window with some
// probability), clears the per-frame counters, runs the update of the current game mode, paces and presents the frame.
// While a pause message is up the mode update is skipped and the message is drawn instead. Returns 1 to go on, 0 when
// the game should end.
extern "C" int Game_FrameStep(void)
{
    if (g_quitRequested != 0)
        return 0;
    if (g_windowActive == 0)
    {
        g_textureManager.PumpMessages();
        return 1;
    }
    int limit;
    if (g_gameMode == MODE_PLAY && (limit = g_charRecords[0].stats[STAT_MAX_HEALTH]) > PLAYER_HEALTH_LIMIT)
    {
        if (rand() % TAMPER_ROLL_RANGE < TAMPER_ROLL_WINDOW_THRESHOLD)
            PostMessageA(g_windowHandle, WM_CLOSE, 0, 0);
        limit = PLAYER_HEALTH_LIMIT;
        g_charRecords[0].stats[STAT_MAX_HEALTH] = limit;
        AntiTamper_StoreValues();
        g_healthClampCountA++;
    }
    if (g_gameMode == MODE_PLAY && g_charRecords[0].stats[STAT_HEALTH] > limit)
    {
        g_charRecords[0].stats[STAT_HEALTH] = limit;
        AntiTamper_StoreValues();
    }
    if (g_textureManager.PumpAndRecover() == 0)
    {
        if (g_quitRequested != 0)
            g_scriptStateA = 1;
        return 0;
    }
    if (g_gameMode == MODE_PLAY && (limit = g_charRecords[0].stats[STAT_MAX_HEALTH]) > PLAYER_HEALTH_LIMIT)
    {
        if (rand() % TAMPER_ROLL_RANGE < TAMPER_ROLL_QUIT_THRESHOLD)
            PostMessageA(g_windowHandle, WM_CLOSE, 0, 0);
        limit = PLAYER_HEALTH_LIMIT;
        g_charRecords[0].stats[STAT_MAX_HEALTH] = limit;
        AntiTamper_StoreValues();
        g_healthClampCountB++;
    }
    if (g_gameMode == MODE_PLAY && g_charRecords[0].stats[STAT_HEALTH] > limit)
    {
        g_charRecords[0].stats[STAT_HEALTH] = limit;
        AntiTamper_StoreValues();
    }
    if (g_loadDataPending != 0 && Game_LoadData() == 0)
        return 0;
    Render_Command(RENDER_COMMAND_BEGIN_FRAME, 1, 0);
    g_frameStepDataReady = 1;
    g_frameCounterA = 0;
    g_frameCounterB = 0;
    g_frameCounterC = 0;
    g_frameCounterD = 0;
    g_textureBindCount = 0;
    g_matrixProductCount = 0;
    g_frameCounterE = 0;
    g_drawCallCount = 0;
    g_pendingBatches = 0;
    g_frameCounterGroup[0] = 0;
    g_frameCounterGroup[1] = 0;
    g_frameCounterGroup[2] = 0;
    g_frameCounterGroup[3] = 0;
    g_frameCounterF = 0;
    g_motionBlendCount = 0;
    if (g_pauseMessage == 0)
    {
        if (g_gameMode == MODE_TITLE)
            TitleScreen_Update();
        else if (g_gameMode == MODE_PLAY)
            Gameplay_Update();
        else if (g_gameMode == MODE_ITEM_DICTIONARY)
            ItemDictionary_Update();
        else if (g_gameMode == MODE_NAME_ENTRY)
            NameEntry_Update();
        else if (g_gameMode == MODE_FIELD)
            Field_Update();
    }
    if (g_gameMode == MODE_PLAY && g_charRecords[0].stats[STAT_MAX_HEALTH] > PLAYER_HEALTH_LIMIT)
    {
        if (rand() % TAMPER_ROLL_RANGE < TAMPER_ROLL_QUIT_THRESHOLD)
            g_quitRequested = 1;
        g_charRecords[0].stats[STAT_MAX_HEALTH] = PLAYER_HEALTH_LIMIT;
        AntiTamper_StoreValues();
        g_healthClampCountA++;
        if (rand() % TAMPER_ROLL_RANGE < TAMPER_ROLL_QUIT_THRESHOLD)
            g_quitRequested = 1;
    }
    if (g_gameMode == MODE_PLAY && g_charRecords[0].stats[STAT_HEALTH] > g_charRecords[0].stats[STAT_MAX_HEALTH])
    {
        g_charRecords[0].stats[STAT_HEALTH] = g_charRecords[0].stats[STAT_MAX_HEALTH];
        AntiTamper_StoreValues();
    }
    g_textureManager.PaceFrame();
    if (g_pauseMessage != 0)
    {
        g_input.Update();
        g_pauseControls = g_input.ReadControls(0);
        Render_ResetTarget();
        Render_Command(RENDER_COMMAND_FILL, PAUSE_FILL_COLOUR, 0);
        g_textRenderer.DrawText(PAUSE_MESSAGE_X, PAUSE_MESSAGE_Y, &g_pauseMessage, 0, 0.0f, PAUSE_MESSAGE_ALPHA, PAUSE_MESSAGE_SCALE);
        g_textureManager.PresentFrame();
        g_mouse.Update();
        char button = 0;
        g_mouse.GetState(0, &button, 0, 0, 0, 0, 0);
        if (g_pauseControls != 0 || button != 0)
            return 0;
    }
    else
        g_textureManager.PresentSimple();
    return 1;
}
