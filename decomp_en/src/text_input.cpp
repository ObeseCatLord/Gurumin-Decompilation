// Start of text input (name entry) of the EN build. Ported from the matched JP source decomp/src/ime.cpp.
#include "ime.h"

// TextInput_Begin: starts text input into `buffer` (name entry): resets the input state, attaches the IME to the
// game window and installs the four IME callbacks. TextInput_End ends it. Returns 1.
extern "C" int TextInput_Begin(char *buffer)
{
    g_textInputActive = 0;
    g_textInputAllowed = 1;
    g_textInputState = 0;
    g_textInputCursor = 0;
    g_textInputBuffer = buffer;
    g_imeContext.Attach(g_gameWindow.window);
    g_gameWindow.compositionEnded = 0;
    g_gameWindow.compositionResult = 0;
    g_gameWindow.onCompositionString = Ime_ReadCompositionString;
    g_gameWindow.onCompositionResult = Ime_ReadResultString;
    g_gameWindow.onCursorPos = Ime_ReadCursorPosition;
    g_gameWindow.onAttributes = Ime_ReadAttributes;
    return 1;
}
