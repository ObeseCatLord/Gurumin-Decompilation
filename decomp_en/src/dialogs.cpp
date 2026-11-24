// Dialog menus, history ring, key prompts and mesh slots (EN build). JP twins: app_0008.cpp, app_0010.cpp.
#include "dialogs.h"
#include "engine_memory.h"
#include <string.h>

#include "game_vars.h"
#include "texture_mgr.h"
#include "volume_fade.h"

// Menu window ids of the dialogs.
const int WINDOW_DIALOG_YESNO = 0x216;
const int WINDOW_DIALOG_CHOICE = 0x218;
const int WINDOW_OVERLAY = 0x22c;

// Closes the yes/no dialog (window 0x216) and window 0x218 and clears the message text. Returns 1. Called by the
// script and dialog code when a dialog ends.
extern "C" int Dialog_CloseAll(void)
{
    g_dialogGame.CloseWindow(WINDOW_DIALOG_CHOICE);
    g_dialogGame.CloseWindow(WINDOW_DIALOG_YESNO);
    g_dialogMessage[0] = 0;
    return 1;
}

// Marks all looping-sound slots free and stops the sound engine's loops (a second call when the first did
// something), unless sound is muted. Called when a map is left.
extern "C" void SoundLoops_StopAll(void)
{
    if (g_soundMuted == 0)
    {
        for (int i = 0; i < SOUND_LOOP_COUNT; i++)
            g_soundLoopHandle[i] = SOUND_LOOP_FREE;
        if (g_soundArcStopLoops())
            g_soundArcStopLoops();
    }
}

// Pushes `value` into the 8-entry history ring and advances the write index. Returns 0.
extern "C" int History_Push(int value)
{
    g_history[g_historyNext] = value;
    g_historyNext = (g_historyNext + 1) & HISTORY_MASK;
    return 0;
}

// Shows key prompt `key` over character `index` for `frames` frames; when the same prompt is already shown only the
// timer is refreshed. Returns 0.
extern "C" int KeyPrompt_Show(int index, int key, int frames)
{
    if (g_keyPromptShown[index] != 0 && g_keyPromptKey[index] == key)
    {
        g_keyPromptFrames[index] = frames;
        return 0;
    }
    g_keyPromptFrames[index] = frames;
    g_keyPromptShown[index] = 1;
    g_keyPromptKey[index] = key;
    return 0;
}

// Releases and frees the models of all 64 effect model slots and clears their visible and frame fields. Returns 1.
extern "C" int MeshSlots_ReleaseAll(void)
{
    for (int i = 0; i < EFFECT_MODEL_COUNT; i++)
    {
        g_effectModels[i].visible = 0;
        g_effectModels[i].frame = 0;
        if (g_effectModels[i].node != 0)
        {
            g_effectModels[i].node->Release();
            g_engineFree(g_effectModels[i].node);
        }
        g_effectModels[i].node = 0;
    }
    return 1;
}

// Texture slot value meaning "no texture".
const int OVERLAY_NO_TEXTURE = -1;

// Advances the music volume fade (the current volume moves by the step each frame and lands on the target on the last
// frame). Once the fade has left zero while the overlay mode is open, the mode is closed: its menus (0x22c, 0x218,
// 0x216) and message texts, its model and texture slots and its state are released. Called every frame.
extern "C" void Overlay_AdvanceFade(void)
{
    if (g_bgmFadeFramesLeft > 0)
    {
        g_bgmFadeFramesLeft--;
        g_bgmFadeVolume = g_bgmFadeStep + g_bgmFadeVolume;
        if (g_bgmFadeFramesLeft == 0)
            g_bgmFadeVolume = g_bgmFadeTarget;
    }
    if (g_bgmFadeVolume != 0.0f && g_overlayOpen != 0)
    {
        g_overlayState[OVERLAY_STATE_FADE_ENTRY] = 0;
        g_overlayFlag = 0;
        g_dialogGame.CloseWindow(WINDOW_OVERLAY);
        g_dialogGame.CloseWindow(WINDOW_DIALOG_CHOICE);
        g_dialogGame.CloseWindow(WINDOW_DIALOG_YESNO);
        g_dialogMessage[0] = 0;
        MessageWindow_SetTitle("");
        MessageWindow_SetBody("");
        g_overlayClosed = 1;
        if (g_overlayModel != 0)
        {
            g_overlayModelValueD = 0;
            g_overlayModel->Release();
            g_engineFree(g_overlayModel);
            g_overlayModel = 0;
            g_overlayModelValueA = 0;
            g_overlayModelValueB = 0;
            g_overlayModelValueC = 0;
            g_overlayModelValueE = 0;
            g_overlayModelValueF = 0;
            g_overlayModelValueG = 0;
            for (int i = 0; i < OVERLAY_TEXTURE_COUNT; i++)
            {
                if (g_overlayTextures[i] > -1)
                {
                    Texture_Release(g_overlayTextures[i], 1);
                    g_overlayTextures[i] = OVERLAY_NO_TEXTURE;
                }
            }
        }
        g_dialogGame.CloseWindow(WINDOW_DIALOG_CHOICE);
        g_dialogGame.CloseWindow(WINDOW_DIALOG_YESNO);
        g_dialogMessage[0] = 0;
        g_overlayOpen = 0;
        memset(g_overlayState, 0, sizeof(g_overlayState));
    }
}
