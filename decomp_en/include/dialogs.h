// Dialog, key-prompt, history and mesh-slot state of the EN build (JP: app_0008.cpp, app_0010.cpp).
// This is the one declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_DIALOGS_H
#define GURUMIN_EN_DIALOGS_H

#include "game.h"
#include "node.h"

// Entries of the history ring, and its index mask.
const int HISTORY_COUNT = 8;
const int HISTORY_MASK = HISTORY_COUNT - 1;
// Characters that can show a key prompt.
const int KEY_PROMPT_COUNT = 0x100;
// Attached mesh slots.
const int MESH_SLOT_COUNT = 0x40;

// One attached mesh slot (0xb08 bytes).
struct MeshSlot
{
    Node *mesh;                 // 0x000: the attached model, 0 when free
    char m_0004[0x28];          // 0x004: not used by EN code yet
    int m_002c;                 // 0x02c: cleared when the slot is released
    int m_0030;                 // 0x030: cleared when the slot is released
    char m_0034[0xb08 - 0x34];  // 0x034: not used by EN code yet
};

// The Game object that owns the dialog menus (windows 0x216, 0x218 and 0x22c). EN 0x00a15298 (JP DAT_104b7200).
extern "C" Game g_dialogGame;
// First character of the dialog message text (0 = no message). EN 0x00a20c80 (JP DAT_1020e0f8).
extern "C" char g_dialogMessage[];
// Sound engine call that stops its looping sounds; non-zero when it did something. EN 0x009094c4 (JP FUN_10001b40).
extern "C" int (*g_soundArcStopLoops)(void);
// Ring of the last values pushed by History_Push, and the next write index. EN 0x00908134, 0x00920028.
extern "C" int g_history[HISTORY_COUNT];
extern "C" int g_historyNext;
// Key prompt state per character: shown flag, key, frames left. EN 0x00a20d00, 0x00a21100, 0x00a20f00.
extern "C" int g_keyPromptShown[KEY_PROMPT_COUNT];
extern "C" int g_keyPromptKey[KEY_PROMPT_COUNT];
extern "C" int g_keyPromptFrames[KEY_PROMPT_COUNT];
// Attached mesh slots. EN 0x015bd794 (JP DAT_10edbe0c).
extern "C" MeshSlot g_meshSlots[MESH_SLOT_COUNT];

// Closes the yes/no dialog and its sibling menu and clears the message. EN 0x5ca1a0.
extern "C" int Dialog_CloseAll(void);
// Marks every looping-sound slot free and stops the sound engine's loops. EN 0x63fe20.
extern "C" void SoundLoops_StopAll(void);
// Pushes a value into the history ring. EN 0x5a2da0.
extern "C" int History_Push(int value);
// Shows a key prompt over a character. EN 0x5a2150.
extern "C" int KeyPrompt_Show(int index, int key, int frames);
// Releases and frees every attached mesh. EN 0x63f400.
extern "C" int MeshSlots_ReleaseAll(void);

#endif
