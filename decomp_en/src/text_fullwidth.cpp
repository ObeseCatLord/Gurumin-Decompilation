// Converting text to full-width characters (Shift-JIS), EN build. Ported from the matched JP source
// decomp/src/text_fullwidth.cpp: every single-byte character becomes its full-width form from a 256-entry table;
// double-byte characters are kept, except the full-width space, which becomes the full-width underscore.

#include <string.h>

// Text_ToFullWidth: converts `text` in place (it must have room for twice its length). Returns 0.
extern "C" int Text_ToFullWidth(char *text)
{
    // full-width form of each byte (lead bytes and unused codes map to "")
    char fullWidth[256][4] = {
        "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a",   // 0x00
        "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a",   // 0x08
        "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a",   // 0x10
        "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a",   // 0x18
        "\x81Q", "\x81I", "\x81h", "\x81\x94", "\x81\x90", "\x81\x93", "\x81\x95", "\x81" "f",   // 0x20
        "\x81i", "\x81j", "\x81\x96", "\x81{", "\x81" "C", "\x81|", "\x81" "D", "\x81^",   // 0x28
        "\x82O", "\x82P", "\x82Q", "\x82R", "\x82S", "\x82T", "\x82U", "\x82V",   // 0x30
        "\x82W", "\x82X", "\x81" "F", "\x81G", "\x81\x83", "\x81\x81", "\x81\x84", "\x81H",   // 0x38
        "\x81\x97", "\x82`", "\x82" "a", "\x82" "b", "\x82" "c", "\x82" "d", "\x82" "e", "\x82" "f",   // 0x40
        "\x82g", "\x82h", "\x82i", "\x82j", "\x82k", "\x82l", "\x82m", "\x82n",   // 0x48
        "\x82o", "\x82p", "\x82q", "\x82r", "\x82s", "\x82t", "\x82u", "\x82v",   // 0x50
        "\x82w", "\x82x", "\x82y", "\x81m", "\x81\x8f", "\x81n", "\x81O", "\x81Q",   // 0x58
        "\x81" "e", "\x82\x81", "\x82\x82", "\x82\x83", "\x82\x84", "\x82\x85", "\x82\x86", "\x82\x87",   // 0x60
        "\x82\x88", "\x82\x89", "\x82\x8a", "\x82\x8b", "\x82\x8c", "\x82\x8d", "\x82\x8e", "\x82\x8f",   // 0x68
        "\x82\x90", "\x82\x91", "\x82\x92", "\x82\x93", "\x82\x94", "\x82\x95", "\x82\x96", "\x82\x97",   // 0x70
        "\x82\x98", "\x82\x99", "\x82\x9a", "\x81o", "\x81" "b", "\x81p", "\x81`", "\x87\x8a",   // 0x78
        "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a",   // 0x80
        "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a",   // 0x88
        "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a",   // 0x90
        "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a",   // 0x98
        "\x81Q", "\x81" "B", "\x81u", "\x81v", "\x81" "A", "\x87\x8a", "\x83\x92", "\x83@",   // 0xa0
        "\x83" "B", "\x83" "D", "\x83" "F", "\x83H", "\x83\x83", "\x83\x85", "\x83\x87", "\x83" "b",   // 0xa8
        "\x81[", "\x83" "A", "\x83" "C", "\x83" "E", "\x83G", "\x83I", "\x83J", "\x83L",   // 0xb0
        "\x83N", "\x83P", "\x83R", "\x83T", "\x83V", "\x83X", "\x83Z", "\x83\x5c",   // 0xb8
        "\x83^", "\x83`", "\x83" "c", "\x83" "e", "\x83g", "\x83i", "\x83j", "\x83k",   // 0xc0
        "\x83l", "\x83m", "\x83n", "\x83q", "\x83t", "\x83w", "\x83z", "\x83}",   // 0xc8
        "\x83~", "\x83\x80", "\x83\x81", "\x83\x82", "\x83\x84", "\x83\x86", "\x83\x88", "\x83\x89",   // 0xd0
        "\x83\x8a", "\x83\x8b", "\x83\x8c", "\x83\x8d", "\x83\x8f", "\x83\x93", "\x81J", "\x81K",   // 0xd8
        "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a",   // 0xe0
        "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a",   // 0xe8
        "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a",   // 0xf0
        "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a", "\x87\x8a",   // 0xf8
    };
    unsigned char copy[0x100];
    strcpy((char *)copy, text);
    int length = strlen(text);
    int out = 0;
    for (int i = 0; i < length; i++)
    {
        unsigned char c = copy[i];
        if ((c >= 0x7f && c < 0xa0) || c >= 0xe0)
        {
            // a double-byte character: kept, except that the full-width space becomes the full-width underscore
            if (*(unsigned short *)&copy[i] == 0x4081)
            {
                *(unsigned short *)&text[out] = 0x5181;
                out += 2;
                i++;
            }
            else
            {
                text[out] = c;
                text[out + 1] = copy[i + 1];
                out += 2;
                i++;
            }
        }
        else
        {
            // single-byte: ASCII or half-width katakana
            text[out] = fullWidth[c][0];
            text[out + 1] = fullWidth[c][1];
            out += 2;
        }
    }
    text[out] = 0;
    return 0;
}
