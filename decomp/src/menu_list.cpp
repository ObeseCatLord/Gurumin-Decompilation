// IME candidate list layout.

struct MenuText
{
    char text[0x80];
};

struct TextRenderer
{
    int M_1014fd30(const MenuText *line, float scale);
};

extern "C" int DAT_10f66aac;
extern "C" int DAT_10f66ab0;
extern "C" int DAT_10f66de8;
extern "C" int DAT_10f66dd0;
extern "C" MenuText DAT_10f67008[];

// Number of candidates shown on the current page (at most 9).
inline int VisibleLines()
{
    int remaining = DAT_10f66aac - DAT_10f66ab0 * DAT_10f66de8 - DAT_10f66dd0;
    if (remaining >= 9)
        remaining = 9;
    return remaining;
}

// Returns the number of visible candidates.
extern "C" int FUN_1015fa90(void)
{
    return VisibleLines();
}

// Returns the width in pixels of the widest visible candidate.
extern "C" int FUN_1015fac0(TextRenderer *renderer)
{
    int widest = 0;
    for (int i = 0; i < VisibleLines(); i++)
    {
        int width = renderer->M_1014fd30(&DAT_10f67008[i], 1.0f);
        if (widest < width)
            widest = width;
    }
    return widest;
}

