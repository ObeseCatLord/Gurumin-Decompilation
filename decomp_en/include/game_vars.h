// Global game state of the EN build (one declaration each; JP equivalents in decomp/include/char_vars.h etc.).
#ifndef GURUMIN_EN_GAME_VARS_H
#define GURUMIN_EN_GAME_VARS_H

extern "C" int g_currentMap;   // EN 0x01332c78 (JP DAT_10c62ed0): number of the map being played


#include "vec4.h"

// Items the inventory can hold (stock per item id).
const int ITEM_COUNT = 0x80;
// Bytes of the message window's text and of its second line.
const int MESSAGE_TEXT_SIZE = 0x80;
const int MESSAGE_SUBTEXT_SIZE = 0x80;
// Looping sounds that can play at once, and the g_soundLoopHandle value of a free slot.
const int SOUND_LOOP_COUNT = 0x44c;
const short SOUND_LOOP_FREE = -11;

extern "C" signed char g_itemStock[ITEM_COUNT];   // EN 0x009e3400 (JP DAT_103150c0): stock per item id, saved with the game

// Message window state (game_state.cpp).
extern "C" int g_messageFrames;                   // EN 0x00b493e0 (JP DAT_104bfd4c): frames the message stays up
extern "C" char g_messageAlternateStyle;          // EN 0x00a1d9e4 (JP DAT_104c01fc): 1 = alternative style
extern "C" int g_messageOpen;                     // EN 0x00a20824 (JP DAT_104bf8c8): 1 once the window was opened
extern "C" int g_messageProgress;                 // EN 0x00a20828 (JP DAT_104bf8cc): cleared when a message opens
extern "C" int g_messageScroll;                   // EN 0x00a2082c (JP DAT_104bf8d0): cleared when a message opens
extern "C" char g_messageText[MESSAGE_TEXT_SIZE];       // EN 0x00a20830 (JP DAT_10219bc0): the message shown
extern "C" char g_messageSubtext[MESSAGE_SUBTEXT_SIZE]; // EN 0x00a208b0 (JP DAT_10467ea0): second line (empty = none)

// Looping sounds (game_state.cpp).
extern "C" short g_soundLoopHandle[SOUND_LOOP_COUNT];   // EN 0x009e26a8 (JP DAT_10447338): handle of each looping sound while it plays
extern "C" int g_soundMuted;                            // EN 0x00a1515c (JP DAT_104c01b0): non-zero = no sound effects
extern "C" void (*g_soundArcStopLoop)(int handle);      // EN 0x009094ac (JP FUN_10001c50): stops a looping sound

// Full-screen colour fade (game_state.cpp).
extern "C" int g_fadeFramesLeft;                  // EN 0x0092f210 (JP DAT_1042da68): frames left in the fade
extern "C" Vec4 g_fadeColour;                     // EN 0x0147a1c8 (JP DAT_10da90d0): current fade colour
extern "C" Vec4 g_fadeStep;                       // EN 0x013374a8 (JP DAT_10daa800): colour change per frame
extern "C" Vec4 g_fadeTarget;                     // EN 0x015e9ac8 (JP DAT_10c67f60): colour the fade ends on

// Recomputes the anti-tamper copies of the guarded values. EN 0x5f4700 (JP 0x1000e810).
extern "C" void AntiTamper_StoreArrays(void);

// Takes *amount of item *index. EN 0x609f10.
extern "C" int Inventory_Remove(const int *index, int *amount);
// Opens the message window (normal and alternative style). EN 0x631380, 0x631400.
extern "C" int Message_Show(const char *text, const char *subtext, int frames);
extern "C" int Message_ShowAlt(const char *text, const char *subtext, int frames);
// Stops looping sound `index`. EN 0x63fe50.
extern "C" void SoundLoop_Stop(int index);
// Advances the fade by a frame. EN 0x5af380.
extern "C" void ColourBlend_Step(void);
// Starts a fade to (r, g, b, a) over `frames` frames. EN 0x645960.
extern "C" void ColourFade_Start(int frames, float r, float g, float b, float a);

#endif
