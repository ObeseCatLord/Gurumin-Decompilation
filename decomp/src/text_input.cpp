// Per-frame update of text input (the name entry screen): IME open/close and mode switching, candidate lines,
// Delete/Backspace repeat and the split of the composition string. The IME state itself is in ime.cpp.

#include <windows.h>
#include <imm.h>
#include <stdio.h>
#include <string.h>
#include <mbctype.h>

#include "ime.h"
#include "render_window.h"
#include "render_vars.h"
#include "ui_vars.h"
#include "win_vars.h"

// Direct-input label shown while the IME is off (Shift-JIS).
#define LABEL_DIRECT_INPUT "\x92\xbc\x90\xda\x93\xfc\x97\xcd"
#define KEY_STATE_DOWN 0xf000
#define COMPOSITION_BLINK_MASK 0x1f
#define COMPOSITION_BLINK_ON 0xfu
#define KEY_REPEAT_DELAY 5
#define KEY_IDLE_FRAMES 10
#define MAX_CANDIDATES_SHOWN 9
#define COMPOSITION_MAX_LENGTH 0x30
#define IME_CMODE_ROMAN_ONLY 0x10

// 0x1015f4e0 (TextInput_Update). Per-frame update of text input. With input off it restores the IME mode and
// closes the IME, returning 0. With input on it switches the IME on (toggling between direct and romaji input
// when asked), reads the candidate page into the candidate lines, mirrors the entered text into the caller's
// buffer, handles Delete/Backspace repeat (removing a whole two-byte character when needed) and splits the
// composition string around the clause cursor into the converted, active and remaining parts. Returns 1 while
// input is running, 0 when there is no buffer or input is off. When the IME is closed the result is whatever
// the last call left in eax (the original never returns a value on that path).
extern "C" int FUN_1015f4e0(void)
{
    if (DAT_10f67000 == 0)
    {
        if (DAT_10f68124.IsOpen())
        {
            if (DAT_10f66bc8 != 0)
            {
                DAT_10f68124.SetConversion(&DAT_10f66dd8, &DAT_10f66bb8);
                DAT_10f66bc8 = 0;
            }
            DAT_10f68124.SetOpen(0);
        }
        return 0;
    }
    if (!DAT_10f68124.IsOpen())
    {
        if (DAT_10f66998 != 0)
        {
            if (DAT_10f67000 != 0)
                DAT_10f68124.SetOpen(1);
        }
        else if (DAT_10f67000 != 0)
        {
            DAT_10f68124.SetOpen(1);
            if (DAT_10f66dd4 != 0)
            {
                DAT_10f6810c = 0;
                DAT_10f66bc8 = 1;
                DAT_10f68124.GetConversion(&DAT_10f66dd8, &DAT_10f66bb8);
                DAT_10f66dd4 = 0;
            }
            else
            {
                DAT_10f6810c = (DAT_10f6810c == 0);
                if (DAT_10f6810c != 0)
                {
                    char unused[8];                    // never used; its presence gives the original its security cookie
                    DWORD conversion;
                    DWORD sentence;
                    DAT_10f66bc8 = 1;
                    DAT_10f68124.GetConversion(&DAT_10f66dd8, &DAT_10f66bb8);
                    conversion = IME_CMODE_ROMAN_ONLY;
                    sentence = 0;
                    DAT_10f68124.SetConversion(&conversion, &sentence);
                }
                else
                {
                    DAT_10f68124.SetConversion(&DAT_10f66dd8, &DAT_10f66bb8);
                }
            }
        }
    }
    if (DAT_10f66ef8 == 0)
        return 0;
    if (DAT_10f68124.IsOpen())
    {
        if (DAT_10f68110 != 0)
        {
            DWORD conversion;
            DWORD sentence;
            DAT_10f68124.GetConversion(&conversion, &sentence);
            sentence = 0;
            DAT_10f68124.SetConversion(&conversion, &sentence);
        }
        DAT_10f68124.LoadCandidates();
        DAT_10f66dd0 = DAT_10f68124.GetPageStart();
        DAT_10f66aac = DAT_10f68124.GetCount();
        DAT_10f66ab0 = DAT_10f68124.GetPageSize();
        DAT_10f68114 = DAT_10f68124.GetSelection();
        DAT_10f66de8 = 0;
        if (DAT_10f66ab0 != 0 && DAT_10f68114 >= DAT_10f66ab0)
            DAT_10f66de8 = DAT_10f68114 / DAT_10f66ab0;
        MenuText *entry = DAT_10f67008;
        for (int line = 0; ; line++, entry++)
        {
            int shown = DAT_10f66aac - DAT_10f66de8 * DAT_10f66ab0 - DAT_10f66dd0;
            shown = (shown < MAX_CANDIDATES_SHOWN) ? shown : MAX_CANDIDATES_SHOWN;
            if (line >= shown)
                break;
            sprintf(entry->text, "%s",
                    DAT_10f68124.GetCandidate(DAT_10f66de8 * DAT_10f66ab0 + DAT_10f66dd0 + line));
            if (DAT_10203428 == 1)
                FUN_1015d520(entry->text);
        }
        DAT_10f66de0 = 1;
        if (DAT_10f66bd0[0] != 0)
            DAT_10f669a0 = KEY_IDLE_FRAMES;
        if ((GetKeyState(VK_RETURN) & KEY_STATE_DOWN) == 0 && DAT_10f669a0 > 0)
            DAT_10f669a0--;
        strcpy(DAT_10f66ef8, DAT_10f66df8);
        DAT_10f65698.compositionEnded = 0;
        if (DAT_10f66998 == 0 && DAT_10f6810c != 0)
            strcpy(DAT_10f66f00, LABEL_DIRECT_INPUT);
        else
            DAT_10f68124.GetModeText(DAT_10f66f00);
        if (DAT_10f66bd0[0] == 0)
        {
            if (DAT_10f6811c < KEY_IDLE_FRAMES)
                DAT_10f6811c++;
        }
        else
        {
            DAT_10f6811c = 0;
        }
        if ((GetKeyState(VK_DELETE) & KEY_STATE_DOWN) == 0 && (GetKeyState(VK_BACK) & KEY_STATE_DOWN) == 0
            && DAT_10f6811c > KEY_REPEAT_DELAY)
            DAT_10f66aa8 = 0;
        if (DAT_10f66aa8 == 0)
        {
            if (((GetKeyState(VK_DELETE) & KEY_STATE_DOWN) != 0 || (GetKeyState(VK_BACK) & KEY_STATE_DOWN) != 0)
                && DAT_10f6811c > KEY_REPEAT_DELAY)
            {
                DAT_10f66aa8 = 1;
                int length = strlen(DAT_10f66df8);
                if (length >= 1)
                {
                    if (length == 1)
                        DAT_10f66df8[0] = 0;
                    else if (_ismbblead((unsigned char)DAT_10f66df8[length - 2])
                             && _ismbbtrail((unsigned char)DAT_10f66df8[length - 1]))
                        DAT_10f66df8[length - 2] = 0;
                    else
                        DAT_10f66df8[length - 1] = 0;
                }
            }
        }
        DAT_10f6699c = 0;
        if (DAT_10f66bd0[0] == 0)
        {
            DAT_10f66bc4 = 0;
            strcpy(DAT_10f66cd0, DAT_10f66df8);
            if ((DAT_10f65504 & COMPOSITION_BLINK_MASK) < COMPOSITION_BLINK_ON)
            {
                if (DAT_10f6810c != 0)
                {
                    strcat(DAT_10f66cd0, "\x82\x68");
                    DAT_10f68008[0] = 0;
                    DAT_10f66ab8[0] = 0;
                }
                else
                {
                    strcat(DAT_10f66cd0, "\x81\xa1");
                    DAT_10f68008[0] = 0;
                    DAT_10f66ab8[0] = 0;
                }
            }
            else
            {
                DAT_10f68008[0] = 0;
                DAT_10f66ab8[0] = 0;
            }
        }
        else
        {
            strcpy(DAT_10f66cd0, DAT_10f66df8);
            int start = DAT_10f66ddc;
            int cursor = DAT_10f66de4;
            if (start == cursor)
            {
                DAT_10f66bc4 = 0;
                strncpy(DAT_10f68008, DAT_10f66bd0, start);
                DAT_10f68008[start] = 0;
                if ((int)strlen(DAT_10f66bd0) > cursor)
                {
                    DAT_10f6699c = 1;
                    strcpy(DAT_10f66ab8, DAT_10f66bd0 + cursor);
                }
                else
                {
                    DAT_10f66ab8[0] = 0;
                }
            }
            else
            {
                DAT_10f66bc4 = 1;
                strncpy(DAT_10f68008, DAT_10f66bd0, cursor);
                DAT_10f68008[cursor] = 0;
                strcpy(DAT_10f66ab8, DAT_10f66bd0 + cursor);
            }
        }
        if (strlen(DAT_10f66bd0) > COMPOSITION_MAX_LENGTH)
        {
            if (_ismbblead((unsigned char)DAT_10f66bd0[COMPOSITION_MAX_LENGTH - 2])
                && _ismbbtrail((unsigned char)DAT_10f66bd0[COMPOSITION_MAX_LENGTH - 1]))
                DAT_10f66bd0[COMPOSITION_MAX_LENGTH - 2] = 0;
            else
                DAT_10f66bd0[COMPOSITION_MAX_LENGTH - 1] = 0;
        }
        if (DAT_10203428 == 1)
        {
            FUN_1015d520(DAT_10f66cd0);
            FUN_1015d520(DAT_10f68008);
            FUN_1015d520(DAT_10f66ab8);
        }
    }
}
