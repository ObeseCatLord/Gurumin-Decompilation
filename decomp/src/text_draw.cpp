// Text renderer: drawing a string through the texture cache (the 640x480 screen-space variant).

#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "font_renderer.h"
#include "texture_object.h"

#define TEXT_TEXTURE_COUNT      0x20
#define TEXT_TEXTURE_KEY_LENGTH 0x80    // bytes of the text kept for the cache comparison
#define TEXT_SHADOW_SCALE       2.0f    // y offset per unit of scale
#define TEXEL_CENTRE            0.5f    // texture coordinates start half a texel in
#define TEXT_END_PADDING        4       // pixels of colour written after the text
#define TEXT_TEXTURE_PITCH      0x400   // bytes per row of the text textures (512 pixels)
#define TEXT_TEXTURE_WIDTH      512.0f
#define TEXT_TEXTURE_HEIGHT     32.0f
#define BITMAP_FONT_LINE_HEIGHT 18      // rows of a text rendered with the bitmap font
#define PIXEL_WHITE             0xffff  // background of the Windows-font bitmap
#define PIXEL_OPAQUE            0xf000

// 0x101501f0 (Font_DrawText). Draws `text` as a texture quad at (x, y + 2 * scale) in 640x480 screen coordinates,
// `scale` times its size, tinted by DrawTinted. Like DrawTextClipped the 32 text textures form a cache keyed by
// text, colour and alpha; a miss picks a random slot and renders the text there, with the bitmap font when one is
// loaded or otherwise with the Windows font (white background becomes transparent, the rest opaque). Every draw
// marks its slot as the newest and ages the rest. Returns the width of the text in pixels at `scale`.
int FontRenderer::DrawText(int x, int y, const char *text, int colour, int a, float alpha, float scale)
{
    y += (int)(scale * TEXT_SHADOW_SCALE);
    unsigned short *font = glyphs;
    if (useAlt == 1 && altGlyphs != 0)
        font = altGlyphs;
    int slot = 0;
    if (font == 0)
    {
        for (slot = 0; slot < TEXT_TEXTURE_COUNT; slot++)
        {
            if (alphas[slot] == alpha && colours[slot] == colour && strcmp(text, texts[slot]) == 0)
                goto drawnWindows;
        }
        slot = rand() % TEXT_TEXTURE_COUNT;
        strncpy(texts[slot], text, TEXT_TEXTURE_KEY_LENGTH);
        alphas[slot] = alpha;
        colours[slot] = colour;
        SIZE extent;
        GetTextExtentPoint32A(dc, text, strlen(text), &extent);
        TextOutA(dc, 0, 0, text, strlen(text));
        unsigned short colour16 = (colour >> 4 & 0xf) | (colour >> 12 & 0xf) << 4 | (colour >> 12 & 0xff00);
        unsigned short opaque = colour16 | PIXEL_OPAQUE;
        GetBitmapBits(bitmap, extent.cy << 10, buffers[slot]);
        int rowOffset = 0;
        for (int row = 0; row < extent.cy; row++)
        {
            int offset = rowOffset;
            for (int column = 0; column < extent.cx; column++)
            {
                unsigned short *pixel = (unsigned short *)((char *)buffers[slot] + offset);
                *pixel = *pixel == PIXEL_WHITE ? colour16 : opaque;
                offset += 2;
            }
            rowOffset += TEXT_TEXTURE_PITCH;
        }
        widths[slot] = extent.cx;
        heights[slot] = extent.cy;
        DAT_118d0ba8[textures[slot]].Write16(0, 0, widths[slot] + TEXT_END_PADDING, extent.cy, TEXT_TEXTURE_PITCH,
                                             buffers[slot]);
    drawnWindows:
        float textWidth = (float)widths[slot];
        float textHeight = (float)heights[slot];
        DAT_118d0ba8[textures[slot]].DrawTinted(
            x, y, (int)(textWidth * scale + (float)x), (int)(textHeight * scale + (float)y), 0, 0,
            textWidth / TEXT_TEXTURE_WIDTH, textHeight / TEXT_TEXTURE_HEIGHT, a, alpha, 0, 0);
        recency[slot] = TEXT_TEXTURE_COUNT;
        for (int other = 0; other < TEXT_TEXTURE_COUNT; other++)
        {
            if (other != slot && recency[slot] != 0)
                recency[slot]--;
        }
        return (int)((float)widths[slot] * scale);
    }
    else
    {
        for (slot = 0; slot < TEXT_TEXTURE_COUNT; slot++)
        {
            if (alphas[slot] == alpha && colours[slot] == colour && strcmp(text, texts[slot]) == 0)
                goto drawnFont;
        }
        slot = rand() % TEXT_TEXTURE_COUNT;
        strncpy(texts[slot], text, TEXT_TEXTURE_KEY_LENGTH);
        colours[slot] = colour;
        alphas[slot] = alpha;
        widths[slot] = DrawStringToPixels(buffers[slot], 0, 0, text,
                                          (colour >> 4 & 0xf) | (colour >> 12 & 0xf) << 4 | (colour >> 12 & 0xff00), 0);
        heights[slot] = BITMAP_FONT_LINE_HEIGHT;
        DAT_118d0ba8[textures[slot]].Write16(0, 0, widths[slot] + TEXT_END_PADDING, BITMAP_FONT_LINE_HEIGHT,
                                             TEXT_TEXTURE_PITCH, buffers[slot]);
    drawnFont:
        float textWidth = (float)widths[slot];
        float textHeight = (float)heights[slot];
        DAT_118d0ba8[textures[slot]].DrawTinted(
            x, y, (int)(textWidth * scale + (float)x), (int)(textHeight * scale + (float)y),
            TEXEL_CENTRE / TEXT_TEXTURE_WIDTH, TEXEL_CENTRE / TEXT_TEXTURE_HEIGHT,
            (textWidth - TEXEL_CENTRE) / TEXT_TEXTURE_WIDTH, (textHeight - TEXEL_CENTRE) / TEXT_TEXTURE_HEIGHT,
            a, alpha, 0, 0);
        recency[slot] = TEXT_TEXTURE_COUNT;
        for (int other = 0; other < TEXT_TEXTURE_COUNT; other++)
        {
            if (other != slot && recency[slot] != 0)
                recency[slot]--;
        }
        return (int)((float)widths[slot] * scale);
    }
}
