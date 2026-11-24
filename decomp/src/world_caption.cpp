// The world's caption: a line or two of character speech drawn with an outline over the 3D scene.

#include <string.h>

#include "world_renderer.h"

#define CAPTION_SIZE_BASE 25         // captionSize is stored less this many percent
#define CAPTION_LINE_GAP_PERCENT 17  // gap between the two lines, in percent of the size
#define CAPTION_SCALE_FACTOR 0.9f
#define CAPTION_COLOUR 0xffffff
#define CAPTION_OUTLINE_COLOUR 0x603913
#define CAPTION_LINE_MAX 128
#define SCREEN_WIDTH 640
#define SPEAKER_OPEN_BRACKET "\x81\x75"

// Font scale of the caption for a size in percent.
static inline float CaptionScale(int size)
{
    return (float)size * CAPTION_SCALE_FACTOR / 100.0f;
}

// 0x101687e0 (DrawCaption). Draws the caption at (captionX, captionY), or centred when captionX is -1, in white over a
// dark outline (the text first drawn one pixel up/down/left/right in CAPTION_OUTLINE_COLOUR). A literal backslash-n in
// the text starts a second line. When the text also holds an opening bracket, the part up to and including the bracket
// is the speaker; the first line is then drawn a line gap above captionY and the second line (the speech) on captionY,
// after the speaker's width. The font's alternative glyph set is on while the caption is drawn.
void WorldRenderer::DrawCaption()
{
    if (font == 0)
        return;
    font->useAlt = 1;
    char first[CAPTION_LINE_MAX];
    char second[CAPTION_LINE_MAX];
    char speaker[CAPTION_LINE_MAX];
    first[0] = 0;
    second[0] = 0;
    speaker[0] = 0;
    int split = 0;
    int length = (int)strlen(caption);
    strcpy(first, caption);
    for (int i = 1; i < length - 2; i++)
    {
        if (caption[i] == '\\' && caption[i + 1] == 'n')
        {
            split = 1;
            first[i] = 0;
            strcpy(second, caption + i + 2);
        }
    }
    if (split)
    {
        char *bracket = strstr(caption, SPEAKER_OPEN_BRACKET);
        if (bracket != 0)
        {
            strcpy(speaker, caption);
            speaker[bracket - caption + 2] = 0;
        }
    }
    int size = captionSize + CAPTION_SIZE_BASE;
    int lineGap = size * CAPTION_LINE_GAP_PERCENT / 100 + 4;
    int x = 0;
    if (captionVisible != 0)
    {
        if (captionX == -1)
        {
            int width;
            if (split)
            {
                int speakerWidth = font->TextWidth(speaker, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                speakerWidth += font->TextWidth(second, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                width = font->TextWidth(first, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                if (width < speakerWidth)
                    width = speakerWidth;
            }
            else
            {
                lineGap = 0;
                width = font->TextWidth(first, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            }
            x = (SCREEN_WIDTH - width) / 2;
            font->DrawText(x, captionY - lineGap, first, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            font->DrawText(x - 1, captionY - lineGap - 1, first, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            font->DrawText(x + 1, captionY - lineGap - 1, first, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            font->DrawText(x - 1, captionY - lineGap + 1, first, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            font->DrawText(x + 1, captionY - lineGap + 1, first, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            font->DrawText(x, captionY - lineGap, first, CAPTION_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
        }
        else
        {
            font->DrawText(captionX, captionY - lineGap, first, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            font->DrawText(captionX - 1, captionY - lineGap - 1, first, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            font->DrawText(captionX + 1, captionY - lineGap - 1, first, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            font->DrawText(captionX - 1, captionY - lineGap + 1, first, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            font->DrawText(captionX + 1, captionY - lineGap + 1, first, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            font->DrawOutlined(captionX, captionY - lineGap, first, CAPTION_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
        }
        if (split)
        {
            int speechX;
            if (captionX == -1)
            {
                speechX = font->TextWidth(speaker, CaptionScale(captionSize + CAPTION_SIZE_BASE)) + x;
                font->DrawText(speechX, captionY, second, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                font->DrawText(speechX - 1, captionY - 1, second, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                font->DrawText(speechX + 1, captionY - 1, second, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                font->DrawText(speechX - 1, captionY + 1, second, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                font->DrawText(speechX + 1, captionY + 1, second, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                font->DrawText(speechX, captionY, second, CAPTION_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            }
            else
            {
                int speechWidth = font->TextWidth(speaker, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                font->DrawText(captionX + speechWidth, captionY, second, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                font->DrawText(captionX - 1 + speechWidth, captionY - 1, second, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                font->DrawText(captionX + 1 + speechWidth, captionY - 1, second, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                font->DrawText(captionX - 1 + speechWidth, captionY + 1, second, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                font->DrawText(captionX + 1 + speechWidth, captionY + 1, second, CAPTION_OUTLINE_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
                font->DrawText(captionX + speechWidth, captionY, second, CAPTION_COLOUR, 0, 1.0f, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            }
        }
    }
    font->useAlt = 0;
}

// 0x10184160 (DrawScreenOverlays). Draws the caption, then the cinema bars and fades (DrawOverlay).
void WorldRenderer::DrawScreenOverlays()
{
    DrawCaption();
    DrawOverlay();
}
