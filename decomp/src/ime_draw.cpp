// Drawing of the Japanese IME composition string (name entry and text input).

extern "C" int DAT_10f67000;
extern "C" int DAT_10f66bc4;
extern "C" int DAT_10f6699c;
extern "C" char DAT_10f66cd0[];
extern "C" char DAT_10f68008[];
extern "C" char DAT_10f66ab8[];
extern "C" int __stdcall FUN_1014fdd0(int x, int y, const char *text, int a, int b, int c, int d, int colour, int flags, float scaleX, float scaleY);

// Ime_DrawComposition: draws the text before the cursor, the clause being converted (selected or composing
// colour) and the rest of the composition string, each starting where the previous one ended. Returns 0 when no
// composition is active. Not byte-identical yet (see open.tsv).
extern "C" int FUN_1015fbf0(int unused, int x, int y, int a, int b, int c, int d, int colour, int selectedColour, int composingColour)
{
    if (DAT_10f67000 == 0)
        return 0;
    int width = FUN_1014fdd0(x, y, DAT_10f66cd0, a, b, c, d, colour, 0, 1.0f, 1.0f);
    if (DAT_10f66bc4 != 0)
    {
        width += FUN_1014fdd0(x + width, y, DAT_10f68008, a, b, c, d, selectedColour, 0, 1.0f, 1.0f);
    }
    else
    {
        width += FUN_1014fdd0(x + width, y, DAT_10f68008, a, b, c, d, composingColour, 0, 1.0f, 1.0f);
        if (DAT_10f6699c == 0)
        {
            FUN_1014fdd0(x + width, y, DAT_10f66ab8, a, b, c, d, composingColour, 0, 1.0f, 1.0f);
            return 1;
        }
        composingColour = selectedColour;
    }
    FUN_1014fdd0(x + width, y, DAT_10f66ab8, a, b, c, d, composingColour, 0, 1.0f, 1.0f);
    return 1;
}