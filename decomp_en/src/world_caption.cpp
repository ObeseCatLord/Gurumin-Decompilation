// Per-frame screen overlays of the world renderer (EN build). Ported from decomp/src/world_caption.cpp.
#include <string.h>

#include "engine.h"
#include "font_renderer.h"
#include "world_renderer.h"

// Caption layout: captionSize is stored less this many percent; the gap between the two lines is this percent of the size.
const int CAPTION_SIZE_BASE = 25;
const int CAPTION_LINE_GAP_PERCENT = 17;
// Font scale of the caption: size in percent times this factor, divided by 100.
#define CAPTION_SCALE_FACTOR 0.9f
const int CAPTION_COLOUR = 0xffffff;
const int CAPTION_OUTLINE_COLOUR = 0x603913;
// Bytes of each caption line buffer.
const int CAPTION_LINE_MAX = 128;
// Shift-JIS opening corner bracket that starts the speech after the speaker name.
#define SPEAKER_OPEN_BRACKET "\x81\x75"
// Extra pixels between the two caption lines, added to the gap in percent of the size.
const int CAPTION_LINE_GAP_EXTRA = 4;

// Font scale of the caption for a size in percent.
static inline float CaptionScale(int size)
{
    return (float)size * CAPTION_SCALE_FACTOR / 100.0f;
}

// WorldRenderer::DrawScreenOverlays: draws the caption, then the cinema bars and fades (DrawOverlay). Called once per
// frame at the end of the world draw.
void WorldRenderer::DrawScreenOverlays()
{
    DrawCaption();
    DrawOverlay();
}

// WorldRenderer::DrawCaption. Draws the caption at (captionX, captionY), or centred when captionX is -1, in white over a
// dark outline (the text first drawn one pixel up/down/left/right in CAPTION_OUTLINE_COLOUR). A literal backslash-n in
// the text starts a second line. When the text also holds an opening bracket, the part up to and including the bracket
// is the speaker; the first line is then drawn a line gap above captionY and the second line (the speech) on captionY,
// after the speaker's width. The font's alternative glyph set is off while the caption is drawn (the JP build turns it on).
void WorldRenderer::DrawCaption()
{
    if (font == 0)
        return;
    font->useAlt = 0;
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
    int lineGap = size * CAPTION_LINE_GAP_PERCENT / 100 + CAPTION_LINE_GAP_EXTRA;
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
                width = width < speakerWidth ? speakerWidth : width;
            }
            else
            {
                lineGap = 0;
                width = font->TextWidth(first, CaptionScale(captionSize + CAPTION_SIZE_BASE));
            }
            x = (GAME_SCREEN_WIDTH - width) / 2;
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
