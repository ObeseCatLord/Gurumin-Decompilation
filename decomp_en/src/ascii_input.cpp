// ASCII text input of the EN build (no JP twin): typed letters are appended to a global buffer by a key press callback.
#include <string.h>

#include "ascii_input.h"
#include "ime.h"
#include "small_b11.h"

char g_asciiInputText[ASCII_INPUT_BUFFER_SIZE];
char g_asciiInputRepeatMark[ASCII_INPUT_BUFFER_SIZE];
char g_asciiInputInitial[ASCII_INPUT_BUFFER_SIZE];
int g_asciiInputM50;
int g_asciiInputRepeat;
int g_asciiInputKeyPressed;

// Distance in characters between lower case and upper case letters.
const int ASCII_CASE_OFFSET = 0x20;
// High bits of GetKeyState set while a key is down or a toggle key is on.
const int KEY_STATE_ACTIVE = 0xf000;

// AsciiInput_Begin: starts ASCII text input. Copies `initial` into the text buffer and into the initial buffer, clears
// the counters and installs AsciiInput_OnKey as the key press callback. Called by the name entry and menu code.
extern "C" int AsciiInput_Begin(const char *initial)
{
    g_textInputActive = 1;
    strcpy(g_asciiInputText, initial);
    strcpy(g_asciiInputInitial, initial);
    g_asciiInputM50 = 0;
    g_asciiInputRepeat = 0;
    g_textInputFlag = AsciiInput_OnKey;
    return 0;
}

// AsciiInput_OnKey: key press callback of ASCII input. Backspace and Delete remove the last character; a letter key
// appends the letter in lower case, or upper case when exactly one of Shift and Caps Lock is on. Does nothing while
// ASCII input is not active. Installed by AsciiInput_Begin.
extern "C" void AsciiInput_OnKey(WPARAM keyCode, LPARAM keyData)
{
    bool shiftDown = false;
    bool capsOn = false;
    if (!g_textInputActive)
        return;
    char *end = g_asciiInputText;
    if (GetKeyState(VK_SHIFT) & KEY_STATE_ACTIVE)
        shiftDown = true;
    if (GetKeyState(VK_CAPITAL) & KEY_STATE_ACTIVE)
        capsOn = true;
    if (keyCode == VK_BACK)
    {
        if (g_asciiInputText[0] != 0)
        {
            while (*++end != 0)
                ;
            if (end != g_asciiInputText)
                *--end = 0;
        }
    }
    else if (keyCode == VK_DELETE)
    {
        if (g_asciiInputText[0] != 0)
        {
            while (*++end != 0)
                ;
            if (end != g_asciiInputText)
                *--end = 0;
        }
    }
    g_asciiInputKeyPressed = 1;
    if (g_asciiInputRepeatMark[0] != 0)
        g_asciiInputRepeat = ASCII_INPUT_REPEAT_FRAMES;
    if (keyCode != VK_RETURN && g_asciiInputRepeat > 0)
        g_asciiInputRepeat--;
    if (strlen(g_asciiInputText) >= (unsigned)g_textInputLimit)
        return;
    char caseOffset;
    for (int letter = 'A'; letter <= 'Z'; letter++)
    {
        if (keyCode != letter)
            continue;
        if (!capsOn)
        {
            caseOffset = ASCII_CASE_OFFSET;
            if (shiftDown)
                caseOffset = 0;
        }
        else
        {
            caseOffset = 0;
            if (shiftDown)
                caseOffset = ASCII_CASE_OFFSET;
        }
        while (*end != 0)
            end++;
        *end = caseOffset + (char)letter;
        end++;
        *end = 0;
    }
}
