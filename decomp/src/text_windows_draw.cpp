// The three text windows and the story card (state DAT_10255960).

#include "anim_model.h"
#include "camera_vars.h"
#include "font_renderer.h"
#include "game.h"
#include "game_vars.h"
#include "map_vars.h"
#include "render_state.h"
#include "render_vars.h"
#include "sound_vars.h"
#include "texture_object.h"
#include "text_window.h"
#include "ui_vars.h"

extern "C" int FUN_10143e00(int left, int top, int right, int bottom, unsigned int colour, float z, float alpha);

#define TITLE_SCALE 1.15f           // the title line of window 2 (DAT_1020da50 line 0)
#define CARD_TITLE_SCALE 1.3f
#define TEXT_SCALE 0.9f
#define FIRST_LINE_SCALE 1.1f
#define WINDOW_MARGIN 10            // the frame reaches this far outside the contents
#define FRAME_ALPHA_SCALE 0.7f
#define PERCENT 0.01f
#define LINE_HEIGHT 20
#define TEXT_COLOUR 0xffffff
#define SCREEN_CENTRE_X 320
#define CARD_FADE_STEP 0.04f        // card opacity per frame of `progress`
#define CARD_FADE_FRAMES 0x1a
#define CARD_FADE_PERCENT 50.0f
#define SCREEN_WIDTH_PX 640
#define SCREEN_HEIGHT_PX 480
#define BOOK_TILES_PER_ROW 8
#define BOOK_TILE_SIZE 64
#define LAYER_CARD_FIRST 0x6a
#define LAYER_CARD_LAST 0x70
#define CARD_FACE_X 0xe0
#define CARD_FACE_Y 0x48
#define CARD_FACE_RIGHT 0x1a0
#define CARD_FACE_BOTTOM 0x94
#define CARD_MOTION_LAYER 1
#define CARD_MOTION_BLEND 5

// 0x1007d0a0 (TextWindows_Draw). While the map is loaded (DAT_104c0350), first runs the overlay step when the map
// timer runs without an overlay model. Then draws each of the three windows by kind. A framed window (kind 1)
// shows a picture (styles 2 and 3: the PictureRecord rectangle as texture coordinates) or, in the other styles, a
// panel (style 0 the game's panel, else a tinted frame, the latter at 70 % opacity); with the camera shot set
// (DAT_10c63574) it also draws its text block: 7 lines (window 2, its title DAT_1020da50 centred below) or 8, at
// x + 8, or at x + 0x90 beside a picture when the shot kind (DAT_10c63570) is 1 or 3. A story card (kind 3) makes
// the player start its card motion, fades the screen by half the card opacity, draws the screen copy twice, then
// shows its lines from the frames 0x19, 0x23, 0x2d, 0x37 and, from 0x50, the yellow hint and the picture tile.
// Returns 0.
extern "C" int FUN_1007d0a0(void)
{
    if (DAT_10c6505c != 0 && DAT_104c03c8 == 0)
        FUN_10052480();
    if (DAT_104c0350 != 0)
    {
        int i = 0;
        do
        {
            int titleWidth = 0;
            if (i == 2)
                titleWidth = DAT_1020e538.TextWidth(DAT_1020da50[0], TITLE_SCALE);
            if (DAT_10255960[i].kind == TEXT_WINDOW_FRAMED)
            {
                int style = DAT_10255960[i].style;
                int left = DAT_10255960[i].x;
                int top = DAT_10255960[i].y;
                int frameLeft = left - WINDOW_MARGIN;
                int frameTop = top - WINDOW_MARGIN;
                float u0, v0, u1, v1;
                int frameRight, frameBottom;
                if (style == TEXT_WINDOW_STYLE_PICTURE_FULL)
                {
                    const PictureRecord &rec = DAT_1049efd8[DAT_10255960[i].picture];
                    float width = (float)rec.right;
                    u0 = ((float)rec.left + 0.5f) / width;
                    float height = (float)rec.bottom;
                    v0 = ((float)rec.top + 0.5f) / height;
                    u1 = (width - 0.5f) / width;
                    v1 = (height - 0.5f) / height;
                }
                else if (style == TEXT_WINDOW_STYLE_PICTURE_RECT)
                {
                    float size = (float)DAT_10255960[i].tile;
                    const PictureRecord &rec = DAT_1049efd8[DAT_10255960[i].picture];
                    u0 = ((float)rec.left + 0.5f) / size;
                    v0 = ((float)rec.top + 0.5f) / size;
                    u1 = ((float)rec.right - 0.5f) / size;
                    v1 = ((float)rec.bottom - 0.5f) / size;
                }
                else if (style != TEXT_WINDOW_STYLE_PLAIN)
                {
                    frameRight = DAT_10255960[i].right + WINDOW_MARGIN;
                    frameBottom = DAT_10255960[i].bottom + WINDOW_MARGIN;
                }
                else
                {
                    frameRight = (DAT_10255960[i].right - left) + 2 * WINDOW_MARGIN;
                    frameBottom = (DAT_10255960[i].bottom - top) + 2 * WINDOW_MARGIN;
                }
                float alpha = (float)DAT_10255960[i].progress * PERCENT;
                if (style == TEXT_WINDOW_STYLE_PICTURE_RECT || style == TEXT_WINDOW_STYLE_PICTURE_FULL)
                {
                    DAT_118d0ba8[DAT_1049efd8[DAT_10255960[i].picture].slot].DrawTinted(
                        left, top, DAT_10255960[i].right, DAT_10255960[i].bottom, u0, v0, u1, v1, 0, alpha, 0, 0);
                }
                else
                {
                    if (style != TEXT_WINDOW_STYLE_PLAIN)
                    {
                        DAT_118d0ba8[0].DrawFrameAltTinted(frameLeft, frameTop, frameRight, frameBottom, 0,
                                                           alpha * FRAME_ALPHA_SCALE, 0, 0, 0);
                        if (i == 2)
                            DAT_118d0ba8[0].DrawFrameAltTinted(300 - (titleWidth >> 1), frameTop - 0x26,
                                                               (titleWidth >> 1) + 0x154, frameTop + 7, 0,
                                                               alpha * FRAME_ALPHA_SCALE, 0, 0, 0);
                    }
                    else
                    {
                        DAT_104b7200.DrawPanelAlt(frameLeft, frameTop, frameRight, frameBottom, 0, alpha);
                        if (i == 2)
                            DAT_104b7200.DrawPanelAlt(300 - (titleWidth >> 1), frameTop - 0x26,
                                                      (titleWidth >> 1) + 0x154, frameTop + 7, 0, alpha);
                    }
                    if (DAT_10c63574 != 0)
                    {
                        if (DAT_10c63570 == 1 || DAT_10c63570 == 3)
                        {
                            int middle;
                            if (DAT_10255960[i].style == TEXT_WINDOW_STYLE_PLAIN)
                                middle = frameBottom;
                            else
                                middle = (frameBottom - frameTop) + WINDOW_MARGIN;
                            middle = (middle >> 1) - 0x40 + frameTop;
                            if (DAT_10255960[i].picture >= 0)
                                DAT_118d0ba8[DAT_1049efd8[DAT_10255960[i].picture].slot].DrawTinted(
                                    frameLeft + 0x12, middle, frameLeft + 0x92, middle + 0x80, 0, 0, 1.0f, 1.0f, 0, alpha, 0, 0);
                            if (i == 2)
                            {
                                for (int line = 0; line < 7; line++)
                                {
                                    if (line == 0)
                                    {
                                        if (DAT_10c6356c == 0)
                                            DAT_1020e538.DrawOutlined(frameLeft + 0x9a, frameTop + 0x12, DAT_1020da50[1], TEXT_COLOUR, 0, alpha, FIRST_LINE_SCALE);
                                        else
                                            DAT_1020e538.DrawOutlined(frameLeft + 0x9a, frameTop + 0x12, DAT_1020da50[1], TEXT_COLOUR, 0, alpha, TEXT_SCALE);
                                    }
                                    else
                                    {
                                        DAT_1020e538.DrawOutlined(frameLeft + 0x9a, DAT_10c6356c != 0 ? frameTop + (line * 5 + 5) * 4 : frameTop + 0x16 + line * LINE_HEIGHT, DAT_1020da50[1 + line], TEXT_COLOUR, 0, alpha, TEXT_SCALE);
                                    }
                                }
                                DAT_1020e538.DrawOutlined(SCREEN_CENTRE_X - (titleWidth >> 1), frameTop - 0x1c,
                                                          DAT_1020da50[0], TEXT_COLOUR, 0, alpha, TITLE_SCALE);
                            }
                            else
                            {
                                for (int line = 0; line < 8; line++)
                                {
                                    if (line == 0)
                                    {
                                        if (DAT_10c6356c == 0)
                                            DAT_1020e538.DrawOutlined(frameLeft + 0x9a, frameTop + 0x12, DAT_1020da50[0], TEXT_COLOUR, 0, alpha, FIRST_LINE_SCALE);
                                        else
                                            DAT_1020e538.DrawOutlined(frameLeft + 0x9a, frameTop + 0x12, DAT_1020da50[0], TEXT_COLOUR, 0, alpha, TEXT_SCALE);
                                    }
                                    else
                                    {
                                        DAT_1020e538.DrawOutlined(frameLeft + 0x9a, DAT_10c6356c != 0 ? frameTop + (line * 5 + 5) * 4 : frameTop + 0x16 + line * LINE_HEIGHT, DAT_1020da50[line], TEXT_COLOUR, 0, alpha, TEXT_SCALE);
                                    }
                                }
                            }
                        }
                        else if (i == 2)
                        {
                            for (int line = 0; line < 7; line++)
                            {
                                if (line == 0)
                                {
                                    if (DAT_10c6356c == 0)
                                        DAT_1020e538.DrawOutlined(frameLeft + 0x12, frameTop + 0x12, DAT_1020da50[1], TEXT_COLOUR, 0, alpha, FIRST_LINE_SCALE);
                                    else
                                        DAT_1020e538.DrawOutlined(frameLeft + 0x12, frameTop + 0x12, DAT_1020da50[1], TEXT_COLOUR, 0, alpha, TEXT_SCALE);
                                }
                                else
                                {
                                    DAT_1020e538.DrawOutlined(frameLeft + 0x12, DAT_10c6356c != 0 ? frameTop + (line * 5 + 5) * 4 : frameTop + 0x16 + line * LINE_HEIGHT, DAT_1020da50[1 + line], TEXT_COLOUR, 0, alpha, TEXT_SCALE);
                                }
                            }
                            DAT_1020e538.DrawOutlined(SCREEN_CENTRE_X - (titleWidth >> 1), frameTop - 0x1c,
                                                      DAT_1020da50[0], TEXT_COLOUR, 0, alpha, TITLE_SCALE);
                        }
                        else
                        {
                            for (int line = 0; line < 8; line++)
                            {
                                if (line == 0)
                                {
                                    if (DAT_10c6356c == 0)
                                        DAT_1020e538.DrawOutlined(frameLeft + 0x12, frameTop + 0x12, DAT_1020da50[0], TEXT_COLOUR, 0, alpha, FIRST_LINE_SCALE);
                                    else
                                        DAT_1020e538.DrawOutlined(frameLeft + 0x12, frameTop + 0x12, DAT_1020da50[0], TEXT_COLOUR, 0, alpha, TEXT_SCALE);
                                }
                                else
                                {
                                    DAT_1020e538.DrawOutlined(frameLeft + 0x12, DAT_10c6356c != 0 ? frameTop + 0x12 + line * LINE_HEIGHT : frameTop + 0x16 + line * LINE_HEIGHT, DAT_1020da50[line], TEXT_COLOUR, 0, alpha, TEXT_SCALE);
                                }
                            }
                        }
                    }
                }
            }
            else if (DAT_10255960[i].kind == TEXT_WINDOW_CARD)
            {
                Playback *motion = DAT_10db6ee8[0].motion;
                if (motion->blending == 0 && (motion->layer < LAYER_CARD_FIRST || motion->layer > LAYER_CARD_LAST))
                    DAT_10db6ee8[0].PlayMotion(DAT_10c66c98, CARD_MOTION_LAYER, CARD_MOTION_BLEND, 1.0f);
                float fade;
                if (DAT_10255960[i].progress < CARD_FADE_FRAMES)
                    fade = (float)DAT_10255960[i].progress * CARD_FADE_STEP;
                else
                    fade = 1.0f;
                float savedFade = DAT_104bf8f0;
                DAT_104bf8f0 = fade * CARD_FADE_PERCENT;
                DAT_104bf8ec = 0;
                FUN_10143e00(0, 0, SCREEN_WIDTH_PX, SCREEN_HEIGHT_PX, 0, 0, DAT_104bf8f0 / 100.0f);
                DAT_104bf8f0 = savedFade;
                DAT_1020e538.TextWidth(DAT_1020da50[0], CARD_TITLE_SCALE);
                if (DAT_10c64f8c == 0)
                {
                    FUN_1013f070(0, 1, 0);
                    DAT_118d0ba8[DAT_10202478].DrawTinted(CARD_FACE_X, CARD_FACE_Y, CARD_FACE_RIGHT, CARD_FACE_BOTTOM,
                                                          1.0f / 512, 1.0f / 256, (float)(DAT_10f08af0 - 2) / 1024.0f,
                                                          (float)(DAT_10f08af2 - 2) / 512.0f, 0, fade, 0, 0);
                    DAT_118d0ba8[DAT_10202478].DrawTinted(CARD_FACE_X, CARD_FACE_Y, CARD_FACE_RIGHT, CARD_FACE_BOTTOM,
                                                          1.0f / 512, 1.0f / 256, (float)(DAT_10f08af0 - 2) / 1024.0f,
                                                          (float)(DAT_10f08af2 - 2) / 512.0f, 0, fade, 0, 0);
                    FUN_1013f070(0, 0, 0);
                }
                if (DAT_10255960[i].progress > 0x19)
                    DAT_1020e538.DrawOutlined(0xbe, 0x9c, DAT_1020da50[1], TEXT_COLOUR, 0, fade, 1.0f);
                if (DAT_10255960[i].progress > 0x23)
                    DAT_1020e538.DrawOutlined(0xbe, 0xba, DAT_1020da50[2], TEXT_COLOUR, 0, fade, 1.0f);
                if (DAT_10255960[i].progress > 0x2d)
                    DAT_1020e538.DrawOutlined(0xbe, 0xd8, DAT_1020da50[4], TEXT_COLOUR, 0, fade, 1.0f);
                if (DAT_10255960[i].progress > 0x37)
                    DAT_1020e538.DrawOutlined(0xbe, 0xf6, DAT_1020da50[5], TEXT_COLOUR, 0, fade, 1.0f);
                if (DAT_10255960[i].progress >= 0x50)
                {
                    DAT_1020e538.DrawOutlined(0xdc, 0x119, DAT_1020da50[6], 0xffff00, 0, fade, 1.2f);
                    int tile = DAT_10255960[i].tile;
                    if (tile != 0 && DAT_10c64f8c == 0)
                    {
                        float tileX = (float)(tile % BOOK_TILES_PER_ROW * BOOK_TILE_SIZE);
                        float tileY = (float)(tile / BOOK_TILES_PER_ROW * BOOK_TILE_SIZE);
                        DAT_118d0ba8[DAT_104b7200.texWaku04].DrawTinted(
                            0x130, 0x140, 0x150, 0x160, (tileX + 1.0f) / 512.0f, (tileY + 1.0f) / 512.0f,
                            (tileX + 63.0f) / 512.0f, (tileY + 63.0f) / 512.0f, 0, 1.0f, 0, 0);
                    }
                }
            }
            i++;
        } while (i < TEXT_WINDOW_COUNT);
    }
    return 0;
}
