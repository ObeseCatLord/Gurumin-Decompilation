// Dialog menus, history ring, key prompts and mesh slots (EN build). JP twins: app_0008.cpp, app_0010.cpp.
#include "dialogs.h"
#include "engine_memory.h"
#include "game_vars.h"

// Menu window ids of the dialogs.
const int WINDOW_DIALOG_YESNO = 0x216;
const int WINDOW_DIALOG_CHOICE = 0x218;

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

// Releases and frees the models of all 64 attached mesh slots and clears the slot fields. Returns 1.
extern "C" int MeshSlots_ReleaseAll(void)
{
    for (int i = 0; i < MESH_SLOT_COUNT; i++)
    {
        g_meshSlots[i].m_002c = 0;
        g_meshSlots[i].m_0030 = 0;
        if (g_meshSlots[i].mesh != 0)
        {
            g_meshSlots[i].mesh->Release();
            g_engineFree(g_meshSlots[i].mesh);
        }
        g_meshSlots[i].mesh = 0;
    }
    return 1;
}
