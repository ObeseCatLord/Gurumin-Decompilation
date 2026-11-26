// ASCII text input (letters typed through the key press callback, no IME). Declared once here.
#ifndef GURUMIN_EN_ASCII_INPUT_H
#define GURUMIN_EN_ASCII_INPUT_H

#include <windows.h>

// Size of each ASCII input text buffer in bytes.
const int ASCII_INPUT_BUFFER_SIZE = 256;
// Frames a held key stays flagged after the repeat marker byte is set.
const int ASCII_INPUT_REPEAT_FRAMES = 10;

// The text typed so far (zero terminated). EN 0x02226950.
extern "C" char g_asciiInputText[ASCII_INPUT_BUFFER_SIZE];
// Second buffer after the text; its first byte non-zero restarts the repeat counter. EN 0x02226a50.
extern "C" char g_asciiInputRepeatMark[ASCII_INPUT_BUFFER_SIZE];
// Copy of the text the input started with. EN 0x02226b50.
extern "C" char g_asciiInputInitial[ASCII_INPUT_BUFFER_SIZE];
// Not yet named: cleared when ASCII input starts. EN 0x02228050.
extern "C" int g_asciiInputM50;
// Repeat counter, set to ASCII_INPUT_REPEAT_FRAMES by the repeat mark and counted down on each key but Enter. EN 0x02228058.
extern "C" int g_asciiInputRepeat;
// Set to 1 by every key press the callback sees. EN 0x0222805c.
extern "C" int g_asciiInputKeyPressed;

// Starts ASCII text input: copies `initial` into the text and initial buffers and installs AsciiInput_OnKey. Returns 0.
// EN 0x7ad4b0. Called by the name entry and menu code.
extern "C" int AsciiInput_Begin(const char *initial);
// Key press callback of the ASCII input: Backspace/Delete remove the last character, letters A-Z are appended (lower
// case unless Shift xor Caps Lock). EN 0x7ad550. Installed in g_textInputFlag by AsciiInput_Begin.
extern "C" void AsciiInput_OnKey(WPARAM keyCode, LPARAM keyData);

#endif
