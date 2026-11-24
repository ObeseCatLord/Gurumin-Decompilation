// Opening a message box (EN build). Ported from the JP source decomp/src/message_open.cpp (JP twin 0x10056260).
#include <string.h>

#include "anti_tamper.h"
#include "char.h"
#include "messages.h"
#include "name_paths.h"
#include "sound_effects.h"
#include "text_substitute.h"

// Game mode (g_guardedValue9) in which slot 0 is called "Popon".
const int MODE_POPON = 6;
// Placeholder name of other slots: shown as slot 0's name instead (Shift-JIS "Parin").
static const char PARIN_NAME[] = "\x83\x70\x83\x8a\x83\x93";

// Message_Open: opens message box `slot` with the text lines `line1` to `line4` (0 = empty line) spoken by character
// `slot`, playing the open sound when the first line has text. The names are expanded, then the '#' marker is
// stripped. Returns 1. Called by the script, the tutorials, the shops and the football minigame.
extern "C" int Message_Open(int slot, const char *line1, const char *line2, const char *line3, const char *line4, int extra)
{
    char text[0x100];
    if (line1 != 0 && line1[0] != 0)
        Sound_PlayEffect(SOUND_MESSAGE_OPEN, 0, 1.0f);
    g_messageKeyWait = 0;
    g_messageState[slot] = MESSAGE_TIMED_FADE_IN;
    g_messageFade[slot] = 0;
    g_messageTimer[slot] = 0;
    g_messageExtra[slot] = extra;
    char *text1 = g_messageText[slot].lines[0];
    if (line1 != 0)
        Names_Expand(text1, line1);
    else
        text1[0] = 0;
    char *text2 = g_messageText[slot].lines[1];
    if (line2 != 0)
        Names_Expand(text2, line2);
    else
        text2[0] = 0;
    char *text3 = g_messageText[slot].lines[2];
    if (line3 != 0)
        Names_Expand(text3, line3);
    else
        text3[0] = 0;
    char *text4 = g_messageText[slot].lines[3];
    if (line4 != 0)
        Names_Expand(text4, line4);
    else
        text4[0] = 0;
    if (slot == 0)
    {
        if (g_guardedValue9 == MODE_POPON)
            strcpy(text, "Popon");
        else
            strcpy(text, g_charRecords[0].name);
    }
    else
    {
        strcpy(text, g_charRecords[slot].name);
        if (strcmp(text, PARIN_NAME) == 0)
            strcpy(text, g_charRecords[0].name);
    }
    if (text[0] == TEXT_LABEL_MARKER)
        strcpy(g_messageText[slot].speaker, text + 1);
    else
        strcpy(g_messageText[slot].speaker, text);
    if (line1 != 0)
    {
        Text_CopyStripMarker(text, text1);
        strcpy(text1, text);
    }
    if (line2 != 0)
    {
        Text_CopyStripMarker(text, text2);
        strcpy(text2, text);
    }
    if (line3 != 0)
    {
        Text_CopyStripMarker(text, text3);
        strcpy(text3, text);
    }
    if (line4 != 0)
    {
        Text_CopyStripMarker(text, text4);
        strcpy(text4, text);
    }
    return 1;
}
