// Message boxes of the EN build (JP: decomp/src/message_fades.cpp, ui_vars.h): their state machine variables. This is the
// one declaration of them.
#ifndef GURUMIN_EN_MESSAGES_H
#define GURUMIN_EN_MESSAGES_H

// Message boxes the game keeps.
const int MESSAGE_COUNT = 0x7c;

// States of a message box (g_messageState). Timed boxes (states below MESSAGE_WAIT_KEY_BASE) show for a fixed time, boxes
// that wait for a key use the states above it.
enum MessageState
{
    MESSAGE_HIDDEN = 0,         // not shown
    MESSAGE_TIMED_FADE_IN = 1,  // timed box fading in
    MESSAGE_TIMED_FADE_OUT = 2, // timed box fading out
    MESSAGE_TIMED_SHOWN = 3,    // timed box fully shown, counting its time
    MESSAGE_WAIT_KEY_BASE = 9,  // states above this wait for a key
    MESSAGE_KEY_FADE_IN = 11,   // key box fading in
    MESSAGE_KEY_FADE_OUT = 12,  // key box fading out
    MESSAGE_KEY_SHOWN = 13      // key box fully shown, waiting for a key
};

// Fade value (g_messageFade) at which a box counts as fully shown, and its change per frame.
const int MESSAGE_FADE_FULL = 15;
const int MESSAGE_FADE_STEP = 2;

// Frames a timed box stays shown.
const int MESSAGE_SHOWN_FRAMES = 60;

// Frames a key box must have been waiting before a key closes it.
const unsigned int MESSAGE_KEY_DELAY = 8;

// Bits of g_costumeUnlockBits that make a pending request answerable by a key.
const unsigned int MESSAGE_ANSWER_BITS = 3;

// Lines of text a message box shows below the speaker's name.
const int MESSAGE_LINE_COUNT = 4;
// Characters in a message line or speaker name, including the terminator.
const int MESSAGE_LINE_SIZE = 0x80;

// The text of a message box (0x280 bytes).
struct MessageText
{
    char lines[MESSAGE_LINE_COUNT][MESSAGE_LINE_SIZE];  // 0x000: the text lines
    char speaker[MESSAGE_LINE_SIZE];                    // 0x200: the speaker's name
};

extern "C" int g_messageState[MESSAGE_COUNT];            // EN 0x00a21348 (JP DAT_104bfb08): MessageState of each box
extern "C" int g_messageTimer[MESSAGE_COUNT];            // EN 0x00a217c8 (JP DAT_10467c20): frames the box has been fully shown
extern "C" int g_messageFade[MESSAGE_COUNT];             // EN 0x00a215c8 (JP DAT_104481d8): fade value of the box, 0 = gone, MESSAGE_FADE_FULL = full
extern "C" int g_messageExtra[MESSAGE_COUNT];            // EN 0x00a219c8: extra value Message_Open stores for the box (callers pass 0)
extern "C" MessageText g_messageText[MESSAGE_COUNT];     // EN 0x00a21bc8 (JP DAT_10453208): the text of each box
extern "C" unsigned int g_messageKeyWait;                // EN 0x00b493fc (JP DAT_104bfd5c): frames spent waiting for a key
extern "C" int g_messageKeyReset;                        // EN 0x00a36174 (JP DAT_104bfd10): cleared when a key box is closed
extern "C" char g_messageRequestPending;                 // EN 0x00bc8871 (JP DAT_104c0339): a request waits for an answer by key
extern "C" unsigned int g_costumeUnlockBits;             // EN 0x00a207c4 (JP DAT_104c0230): costume unlock bits
extern "C" char g_keyPressedNow;                         // EN 0x00bce1b0 (JP DAT_1065cb30): a key of the current input frame

// Sound effect played when a message box opens with text.
const int SOUND_MESSAGE_OPEN = 0x38e;

// Opens message box `slot` with up to four text lines (0 = empty) spoken by character `slot`. Returns 1. EN 0x644970 (JP 0x10056260).
extern "C" int Message_Open(int slot, const char *line1, const char *line2, const char *line3, const char *line4, int extra);

// Advances every message box by one frame; answers a pending request on a key or button press. Returns 0. EN 0x5ac740 (JP 0x1000cdf0).
extern "C" int Messages_Update(int shownFrames);

#endif
