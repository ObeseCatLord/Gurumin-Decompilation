// Start-up of the font renderer (EN build): the bitmap fonts, the Windows font and the text textures. Ported from the
// JP draft decomp/src/font_metrics.cpp (FontRenderer::Init, 0x101455d0, not matched there) and the EN bytes.
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "engine_memory.h"
#include "font_renderer.h"
#include "game_file.h"
#include "model_bank.h"
#include "render.h"
#include "texture_mgr.h"
#include "window.h"

// Bytes of a bitmap font file (the glyph tables of ddfont8.dat and ddfont8s.dat).
const int FONT_FILE_BYTES = 0x211080;
// Size of the Windows font's bitmap: wide enough for a line of text.
const int FONT_BITMAP_WIDTH = 640;
const int FONT_BITMAP_HEIGHT = 0x40;
// The Windows font: height in pixels, and a weight of 3500 (extra heavy; the face has no such weight and is drawn bold).
const int FONT_HEIGHT = 0x11;
const int FONT_WEIGHT = 0xdac;
// "ＭＳ Ｐゴシック" (MS P Gothic), Shift-JIS.
#define FONT_FACE_NAME "\x82\x6c\x82\x72\x20\x82\x6f\x83\x53\x83\x56\x83\x62\x83\x4e"
// Size of a text texture in pixels, and the bytes of its pixel buffer: three bytes a pixel in 16-bit texture mode, a
// larger buffer in 32-bit texture mode.
const int FONT_TEXT_TEXTURE_WIDTH = 640;
const int FONT_TEXT_TEXTURE_HEIGHT = 0x20;
const int FONT_TEXT_BUFFER_BYTES = 0xf000;
const int FONT_TEXT_BUFFER_BYTES_32BIT = 0x19000;
// Characters of a text texture's name ("tex00" to "tex31" and the terminator, with room to spare).
const int FONT_TEXTURE_NAME_SIZE = 0x80;
// Shift-JIS code of the character whose advance becomes the width of a digit (proportional text).
const unsigned short FONT_DIGIT_REFERENCE_CODE = 0x4f82;

// Loads the bitmap fonts: `path` (as given or in the data folder), else ddfont8s.dat in the data folder, as the main
// font with ddfont8.dat as the alternative; with neither of the first two, ddfont8.dat alone. Then creates the Windows
// font with its memory DC and bitmap, the 32 text textures "tex00".."tex31" with their pixel buffers, and measures the
// digit advance. `unused` is not read (the window comes from g_gameWindow). Returns 1. Called once by the game start-up.
int FontRenderer::Init(int unused, const char *path)
{
    useAlt = 0;
    GameFile file;
    FILE *opened;
    bool found = false;
    if (path != 0)
    {
        opened = file.Open(path, "rb");
        if (opened != 0)
            found = true;
        else
        {
            opened = file.Open(Path_Data(path, 0), "rb");
            if (opened != 0)
                found = true;
        }
    }
    if (!found)
    {
        g_folderPathCounter++;
        sprintf(g_folderPathBuffers[g_folderPathCounter & (FOLDER_PATH_BUFFERS - 1)], "%s\\%s", g_folderNames[0],
                "ddfont8s.dat");
        opened = file.Open(g_folderPathBuffers[g_folderPathCounter & (FOLDER_PATH_BUFFERS - 1)], "rb");
        if (opened != 0)
            found = true;
    }
    if (found)
    {
        glyphs = (unsigned short *)g_engineAlloc(FONT_FILE_BYTES, "m_lpFont");
        file.Read(glyphs, 1, FONT_FILE_BYTES);
        file.Close();
        altGlyphs = 0;
        bitmap = 0;
        dc = 0;
        font = 0;
        opened = file.Open(Path_Data("ddfont8.dat", 0), "rb");
        if (opened != 0)
        {
            altGlyphs = (unsigned short *)g_engineAlloc(FONT_FILE_BYTES, "m_lpFont2");
            file.Read(altGlyphs, 1, FONT_FILE_BYTES);
            file.Close();
        }
    }
    else
    {
        opened = file.Open(Path_Data("ddfont8.dat", 0), "rb");
        if (opened != 0)
        {
            glyphs = (unsigned short *)g_engineAlloc(FONT_FILE_BYTES, "m_lpFont");
            file.Read(glyphs, 1, FONT_FILE_BYTES);
            file.Close();
            altGlyphs = 0;
            bitmap = 0;
            dc = 0;
            font = 0;
        }
    }

    HWND gameWindow = g_gameWindow.window;
    HDC windowDc = GetDC(gameWindow);
    bitmap = CreateCompatibleBitmap(windowDc, FONT_BITMAP_WIDTH, FONT_BITMAP_HEIGHT);
    dc = CreateCompatibleDC(windowDc);
    SelectObject(dc, bitmap);
    ReleaseDC(gameWindow, windowDc);
    LOGFONTA logFont;
    memset(&logFont, 0, sizeof(logFont));
    strcpy(logFont.lfFaceName, FONT_FACE_NAME);
    logFont.lfPitchAndFamily = 0;
    logFont.lfCharSet = SHIFTJIS_CHARSET;
    logFont.lfHeight = FONT_HEIGHT;
    logFont.lfWeight = FONT_WEIGHT;
    logFont.lfItalic = 0;
    logFont.lfUnderline = 0;
    logFont.lfStrikeOut = 0;
    logFont.lfOutPrecision = 0;
    logFont.lfClipPrecision = 0;
    logFont.lfQuality = 0;
    font = CreateFontIndirectA(&logFont);
    SelectObject(dc, font);

    for (int i = 0; i < FONT_TEXT_SLOTS; i++)
    {
        alphas[i] = 0;
        recency[i] = 0;
        texts[i][0] = 0;
        if (g_textures32Bit != 0)
        {
            buffers[i] = (unsigned short *)g_engineAlloc(FONT_TEXT_BUFFER_BYTES_32BIT, "m_fontbuff");
            memset(buffers[i], 0, FONT_TEXT_BUFFER_BYTES_32BIT);
        }
        else
        {
            buffers[i] = (unsigned short *)g_engineAlloc(FONT_TEXT_BUFFER_BYTES, "m_fontbuff");
            memset(buffers[i], 0, FONT_TEXT_BUFFER_BYTES);
        }
        char name[FONT_TEXTURE_NAME_SIZE];
        sprintf(name, "tex%02d", i);
        int saved32Bit = g_textures32Bit;
        int savedSharedLoading = g_sharedTextureLoading;
        g_sharedTextureLoading = 1;
        g_textures32Bit = 0;
        // The original hands the address of the buffer table (not a buffer) as the pixels and as the alpha mode.
        textures[i] = Texture_CreateFromPixels(name, FONT_TEXT_TEXTURE_WIDTH, FONT_TEXT_TEXTURE_HEIGHT,
                                               (unsigned char *)buffers, (int)buffers, 0);
        g_sharedTextureLoading = savedSharedLoading;
        g_textures32Bit = saved32Bit;
    }
    digitAdvance = GlyphWidth(0, 0, 0, FONT_DIGIT_REFERENCE_CODE, 0, 0);
    return 1;
}
