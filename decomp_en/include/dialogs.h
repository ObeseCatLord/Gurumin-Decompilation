// Dialog, key-prompt, history and mesh-slot state of the EN build (JP: app_0008.cpp, app_0010.cpp).
// This is the one declaration of these globals in decomp_en: include this header, never redeclare them.
#ifndef GURUMIN_EN_DIALOGS_H
#define GURUMIN_EN_DIALOGS_H

#include "game.h"
#include "effect_model.h"
#include "node.h"

// Entries of the history ring, and its index mask.
const int HISTORY_COUNT = 8;
const int HISTORY_MASK = HISTORY_COUNT - 1;
// Characters that can show a key prompt.
const int KEY_PROMPT_COUNT = 0x100;

// The Game object that owns the dialog menus (windows 0x216, 0x218 and 0x22c). EN 0x00a15298 (JP DAT_104b7200).
extern "C" Game g_dialogGame;
// First character of the dialog message text (0 = no message). EN 0x00a20c80 (JP DAT_1020e0f8).
extern "C" char g_dialogMessage[];
// First character of the banner text drawn at the upper left (0 = none). EN 0x00a20c40 (JP DAT_10210ae8).
extern "C" char g_bannerText[];
// Fade-in counter of the banners, 15 = fully visible. EN 0x00a20c3c (JP DAT_104bf8d8).
extern "C" int g_bannerFade;
// Sound engine call that stops its looping sounds; non-zero when it did something. EN 0x009094c4 (JP FUN_10001b40).
extern "C" int (*g_soundArcStopLoops)(void);
// Ring of the last values pushed by History_Push, and the next write index. EN 0x00908134, 0x00920028.
extern "C" int g_history[HISTORY_COUNT];
extern "C" int g_historyNext;
// Key prompt state per character: shown flag, key, frames left. EN 0x00a20d00, 0x00a21100, 0x00a20f00.
extern "C" int g_keyPromptShown[KEY_PROMPT_COUNT];
extern "C" int g_keyPromptKey[KEY_PROMPT_COUNT];
extern "C" int g_keyPromptFrames[KEY_PROMPT_COUNT];
// Balloon sheet slots of g_dialogGame indexed by sheet: 0 = texBalloon5, 1 = texBalloon6, 0x18 = texBalloon7 (the original
// indexes the Game object's texture slots as an array, EN 0x00a152b4 = g_dialogGame + 0x1c; JP DAT_104b721c).
extern "C" int g_dialogGameSlots[];
// Size of the key prompt balloon in pixels (the drawn cell is half of it plus 15). EN 0x00a1ddf0 (JP DAT_104bf88c).
extern "C" int g_keyPromptSize;
// Draws the key prompt balloon above every character that has one. Returns 0. EN 0x5cdcd0 (JP 0x1000d6b0).
extern "C" int KeyPrompt_DrawAll(const Matrix4 *view);

// Texture slots held by the overlay mode, and the bytes of its state block.
const int OVERLAY_TEXTURE_COUNT = 0x22;
const int OVERLAY_STATE_INTS = 0x100;
// Index of the overlay state entry cleared when the fade starts.
const int OVERLAY_STATE_FADE_ENTRY = 0x13;

extern "C" int g_overlayOpen;                                // EN 0x00915ec4 (JP DAT_104bfe1c): 1 while the overlay mode is open
extern "C" int g_overlayState[OVERLAY_STATE_INTS];           // EN 0x00a1d9e8 (JP DAT_1030c500): overlay mode state
extern "C" int g_overlayFlag;                                // EN 0x00b91ea4 (JP DAT_104bfd78): cleared when the overlay closes
extern "C" int g_overlayClosed;                              // EN 0x00a20c38 (JP DAT_104bf8d4): set to 1 when the overlay closes
extern "C" Node *g_overlayModel;                             // EN 0x00920468 (JP DAT_104c03c8): the overlay's model
extern "C" int g_overlayModelValueA;                         // EN 0x0092046c (JP DAT_104c03cc): cleared with the model
extern "C" int g_overlayModelValueB;                         // EN 0x00920870 (JP DAT_104c03d0)
extern "C" int g_overlayModelValueC;                         // EN 0x00920874 (JP DAT_104c03d4)
extern "C" int g_overlayModelValueD;                         // EN 0x00920880 (JP DAT_104c03dc): cleared before the release
extern "C" int g_overlayModelValueE;                         // EN 0x0092087c (JP DAT_104c03d8)
extern "C" int g_overlayModelValueF;                         // EN 0x00920884 (JP DAT_104c03e0)
extern "C" int g_overlayModelValueG;                         // EN 0x00920b18 (JP DAT_104c03e4)
extern "C" int g_overlayPages[OVERLAY_TEXTURE_COUNT];        // EN 0x00920990 (JP DAT_1030c908): script flag of each page (0 = no page)
extern "C" int g_overlayTextures[OVERLAY_TEXTURE_COUNT];     // EN 0x00920e48 (JP DAT_1030b870): texture slots of the overlay, -1 = none
extern "C" Vec4 g_overlayCameraEye;                          // EN 0x01337498 (JP DAT_10c66edc): camera position the overlay model is drawn from
extern "C" Vec4 g_overlayCameraTarget;                       // EN 0x015e9a90 (JP DAT_10edbdf8): point the overlay camera looks at
extern "C" float g_overlayProgress;                          // EN 0x00920e20 (JP DAT_104c03ec): opening progress of the overlay, 0..1
// Draws the overlay model, if any, from the overlay camera. Returns 0. EN 0x5e2320 (JP 0x100236c0).
int Overlay_DrawModel();

// Labels of the yes/no dialog's two choices ("Yes" and "No"). EN 0x84a674, 0x84a678.
extern "C" const char g_textYes[];
extern "C" const char g_textNo[];
// Shows `message` with "No" first (for dangerous questions) / "Yes" first; the chosen value goes to the one pointed
// at by yesValue / noValue. EN 0x6e0790, 0x6e0850 (JP 0x100105e0, 0x10010520).
extern "C" int Dialog_YesNoNoFirst(const char *message, int yesValue, int noValue);
extern "C" int Dialog_YesNoYesFirst(const char *message, int yesValue, int noValue);

// Sets the message window's title text (0 closes it). EN 0x5b96a0 (JP 0x1000cb30).
extern "C" int MessageWindow_SetTitle(const char *text);
// Sets the message window's body text. EN 0x5b9650 (JP 0x1000cb90).
extern "C" int MessageWindow_SetBody(const char *text);
// Advances the music volume fade and closes the overlay mode once a fade is under way. EN 0x5b4240.
extern "C" void Overlay_AdvanceFade(void);

// Closes the yes/no dialog and its sibling menu and clears the message. EN 0x5ca1a0.
extern "C" int Dialog_CloseAll(void);
// Marks every looping-sound slot free and stops the sound engine's loops. EN 0x63fe20.
extern "C" void SoundLoops_StopAll(void);
// Pushes a value into the history ring. EN 0x5a2da0.
extern "C" int History_Push(int value);
// Shows a key prompt over a character. EN 0x5a2150.
extern "C" int KeyPrompt_Show(int index, int key, int frames);
// Releases and frees the model of every effect model slot. EN 0x63f400.
extern "C" int MeshSlots_ReleaseAll(void);

#endif
