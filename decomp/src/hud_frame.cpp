// The frame drawing: the scene passes, then the HUD of the field (stamina gauge, strip of tick marks, target ring,
// effect numbers, health, coins), the menu windows, the message panel and the overlays. Coordinates are 640x480
// screen pixels; the 2D parts are shifted by (DAT_10f654b4, DAT_10f654b8), the margins of a wider back buffer.

#include <math.h>

#include "anim_model.h"
#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "script_vars.h"
#include "debris.h"
#include "render_device.h"
#include "scene_callback.h"
#include "shaga.h"
#include "text_window.h"
#include "tracker.h"
#include "world_renderer.h"
#include "dodge_game.h"
#include "field_item_menu.h"
#include "font_renderer.h"
#include "game.h"
#include "game_load_data.h"
#include "game_vars.h"
#include "hud_digits.h"
#include "hud_frame.h"
#include "message_labels.h"
#include "item_info.h"
#include "matrix4.h"
#include "model_bank.h"
#include "node.h"
#include "render_state.h"
#include "action_select_vars.h"
#include "effect_manager.h"
#include "shop_menu.h"
#include "field_item_menu_draw.h"
#include "render_vars.h"
#include "scene_draw_vars.h"
#include "script_system.h"
#include "texture_load.h"
#include "texture_object.h"
#include "ui_vars.h"
#include "vec4.h"

// The texture slots of the HUD sheets, all fields of the Game object.
#define HUD_TEXTURE(field) DAT_118d0ba8[DAT_104b7200.field]

// Layout of the strip of tick marks.
#define STRIP_LENGTH 480
#define STRIP_TICK_SPACING 15
#define STRIP_ARC_COUNT 15

#define GAUGE_SMALL_CAPACITY 300
#define GAUGE_LARGE_CAPACITY 400

// 0x1008c680 (Hud_DrawFrame). Draws the whole frame: the margin bars of a wide back buffer, the IME panel, the scene
// passes and, while the HUD is shown (DAT_104bfe6c == 0 and no cinema bars), the stamina gauge, the opening bars of
// the counter, the strip of tick marks with the target ring, the effect numbers, the helmet and costume icons, health,
// pickups, coins and gems; then the shop and field menus, the windows, the held item model, the cinema bars, the
// movie frame, the fade and the pictures. Returns 1.
int Hud_DrawFrame(void)
{
    if (DAT_10f654b4 != 0)
    {
        int marginX = DAT_10f654b4 * 640 / DAT_10202b2c;
        int marginY = DAT_10f654b8 * 480 / DAT_10202b30;
        FUN_1013f070(RENDER_CLEAR_DEPTH, 0xffff, 0);
        HUD_TEXTURE(texMenu2).DrawTinted(marginX + 640, 0, 640, 480, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f, 1.0f, 0.0f, 0.0f);
        HUD_TEXTURE(texMenu2).DrawTinted(0, marginY + 480, marginX + 640, 480, 0.0f, 0.0f, 0.0f, 0.0f, 0.7f, 1.0f,
                                         0.0f, 0.0f);
    }
    if (DAT_104c00d0 >= 2000 && DAT_104c00d0 <= 2040)
        FUN_1013f070(RENDER_CLEAR_DEPTH, 0xffff, 0);
    if (DAT_10f65484 != 0)
        FUN_1000cf80(&DAT_1065ca08[1]);
    if (DAT_104c030a != 0 && FUN_1015d510() != 0)
    {
        unsigned int elapsed = DAT_104c030c - DAT_104bf890;
        unsigned int seconds = elapsed / 60;
        int blink = (int)(elapsed - seconds * 60) - 50;
        DAT_104b7200.DrawPanelAlt(0x127, 0xcf, 0x40, 0x40, 0.0f, 1.0f);
        float grow = (float)(blink < 0 ? 0 : blink) / 10.0f;
        FUN_10012990(0x140, 0xe6, seconds, 1.0f, 1, grow, grow);
        DAT_104b7200.DrawPanelAlt(0xe2, 0x110, 0xc0, 0x34, 0.0f, 1.0f);
        FUN_1015fbf0(&DAT_1020e538, 0xf6, 0x120, 0xe2, 0x110, 0x192, 0x144, 0xffffff, 0xff2020, 0x2020ff);
        int candidates = FUN_1015fa90();
        if (candidates != 0)
        {
            int width = FUN_1015fac0(&DAT_1020e538);
            DAT_104b7200.DrawPanelAlt(0xe2, 0xf8 - candidates * 16, width + 0x28, candidates * 16 + 0x24, 0.0f, 1.0f);
            FUN_1015fb20(&DAT_1020e538, 0xf6, 0x108 - candidates * 16, 0xffffff, 0xff2020);
        }
        DAT_1020e538.DrawText(0x244, 0x1cc, DAT_10f66f00, 0xffffff, 0.0f, 1.0f, 0.6f);
    }
    DAT_10202b34 += DAT_10f654b4;
    DAT_10202b38 += DAT_10f654b8;
    if (DAT_10f65484 == 0)
        FUN_1000cf80(&DAT_1065ca08[1]);
    if (DAT_104bfd78 == 0)
    {
        FUN_101999a0(&DAT_1065ca08[0], &DAT_1065ca08[1], DAT_10c67ef0, 2);
        DAT_10daaa80.Draw(DAT_1065ca08[1], &DAT_1065ca08[2], 4);
    }
    DAT_10202b34 -= DAT_10f654b4;
    DAT_10202b38 -= DAT_10f654b8;
    if (DAT_104bfe6c == 0 && DAT_104c02e6 == 0)
    {
        if (DAT_10c652cc != 0)
        {
            int rows = (DAT_10c64ad0 != 0) + 2;
            int stamina = DAT_10c64ad8;
            if (rows == 2)
            {
                if (stamina >= GAUGE_SMALL_CAPACITY)
                    stamina = GAUGE_SMALL_CAPACITY;
            }
            else if (rows == 3 && stamina >= GAUGE_LARGE_CAPACITY)
                stamina = GAUGE_LARGE_CAPACITY;
            int frameX = (rows == 3) ? -349 : -329;
            HUD_TEXTURE(texWaku03).DrawTinted(frameX + 0x1fe, 1, frameX + 0x216, 0x17, 0.625f, 0.0625f, 0.75f,
                                              0.15625f, 0.7f, 0.7f, 0.0f, 0.0f);
            HUD_TEXTURE(texWaku03).DrawTinted(frameX + 0x202, 3, frameX + 0x212, 0x13,
                                              (float)((DAT_10c64ad4 + 10) * 16) / 256.0f, 0.1875f,
                                              (float)((DAT_10c64ad4 + 11) * 16) / 256.0f, 0.25f, 0.7f, 0.9f, 0.0f,
                                              0.0f);
            int barX = (rows == 3) ? -343 : -319;
            int barLeft = barX + 0x21c;
            HUD_TEXTURE(texWaku03).DrawTinted(barX + 0x1fc, 5, barLeft, 0x14, 0.125f, 0.12890625f, 0.25f, 0.1875f,
                                              0.7f, 0.7f, 0.0f, 0.0f);
            HUD_TEXTURE(texWaku03).DrawTinted(barLeft, 5, barX + 0x24e, 0x14, 0.25f, 0.12890625f, 0.37109375f,
                                              0.1875f, 0.7f, 0.7f, 0.0f, 0.0f);
            HUD_TEXTURE(texWaku03).DrawTinted(barX + 0x24e, 5, barX + 0x280, 0x14, 0.25f, 0.12890625f, 0.37109375f,
                                              0.1875f, 0.7f, 0.7f, 0.0f, 0.0f);
            HUD_TEXTURE(texWaku03).DrawTinted(barX + 0x280, 5, barX + 0x2b2, 0x14, 0.25f, 0.12890625f, 0.37109375f,
                                              0.1875f, 0.7f, 0.7f, 0.0f, 0.0f);
            bool full;
            if (rows == 2)
            {
                HUD_TEXTURE(texWaku03).DrawTinted(barX + 0x2b2, 5, barX + 0x2d2, 0x14, 0.375f, 0.12890625f, 0.5f,
                                                  0.1875f, 0.7f, 0.7f, 0.0f, 0.0f);
                full = stamina >= GAUGE_SMALL_CAPACITY;
            }
            else
            {
                HUD_TEXTURE(texWaku03).DrawTinted(barX + 0x2b2, 5, barX + 0x2e4, 0x14, 0.25f, 0.12890625f,
                                                  0.37109375f, 0.1875f, 0.7f, 0.7f, 0.0f, 0.0f);
                HUD_TEXTURE(texWaku03).DrawTinted(barX + 0x2e4, 5, barX + 0x304, 0x14, 0.375f, 0.12890625f, 0.5f,
                                                  0.1875f, 0.7f, 0.7f, 0.0f, 0.0f);
                full = rows == 3 && stamina >= GAUGE_LARGE_CAPACITY;
            }
            if (full)
            {
                HUD_TEXTURE(texWaku03).DrawTinted(
                    barLeft, 7, (int)((float)((DAT_10c64ad4 + 1) * 5000) / 100.0f + (float)barLeft), 0xf,
                    0.00390625f, 0.6328125f, 0.00390625f, 0.63671875f, 0.71f, 0.7f, 0.0f,
                    (float)DAT_104bf88c * 0.049999997f);
            }
            else
            {
                int filledRight = (int)((float)((DAT_10c64ad4 + 1) * 5000) / 100.0f + (float)barLeft);
                HUD_TEXTURE(texWaku03).DrawTinted(barLeft, 7, filledRight, 0xf, 0.00390625f, 0.6328125f,
                                                  0.00390625f, 0.63671875f, 0.71f, 0.7f, 0.0f,
                                                  (float)DAT_104bf88c * 0.049999997f);
                HUD_TEXTURE(texWaku03).DrawTinted(filledRight, 7, (int)((float)(stamina * 50) / 100.0f + (float)barLeft),
                                                  0xf, 0.46875f, 0.9453125f, 0.46875f, 0.9453125f, 0.71f, 0.7f, 0.0f,
                                                  0.0f);
            }
        }
        HUD_TEXTURE(texWaku03).DrawTinted(0x9d, 0x17, 0xbd, 0x36, 0.125f, 0.00390625f, 0.25f, 0.125f, 0.7f, 0.5f,
                                          0.0f, 0.0f);
        HUD_TEXTURE(texWaku03).DrawTinted(0xbd, 0x17, 0x1bb, 0x36, 0.25f, 0.00390625f, 0.375f, 0.125f, 0.7f, 0.5f,
                                          0.0f, 0.0f);
        HUD_TEXTURE(texWaku03).DrawTinted(0x1bb, 0x17, 0x1db, 0x36, 0.375f, 0.00390625f, 0.5f, 0.125f, 0.7f, 0.5f,
                                          0.0f, 0.0f);

        // The bars of the counter in the middle open from the centre over 60 frames, 145 pixels to each side.
        if (DAT_104c0108 < 70)
        {
            int opening = DAT_104c0108 - 10;
            int openFrames = opening < 1 ? 0 : opening;
            float rightSlide = ((1.0f - (float)pow((double)(1.0f - (float)openFrames / 60.0f), 2.0)) * 256.0f) / 256.0f;
            HUD_TEXTURE(texWaku03).DrawTinted(0x13c, 0x18,
                                              (int)((1.0f - (float)pow((double)(1.0f - (float)openFrames / 60.0f), 2.0)) *
                                                        145.0f + 316.0f),
                                              0x35, 0.00390625f, 0.6328125f, rightSlide, 0.63671875f, 0.7f, 0.7f, 0.0f,
                                              0.0f);
            opening = DAT_104c0108 - 10;
            openFrames = opening < 1 ? 0 : opening;
            float leftSlide = ((1.0f - (float)pow((double)(1.0f - (float)openFrames / 60.0f), 2.0)) * 256.0f) / 256.0f;
            HUD_TEXTURE(texWaku03).DrawTinted((int)(316.0f - (1.0f - (float)pow((double)(1.0f - (float)openFrames / 60.0f), 2.0)) *
                                                                145.0f),
                                              0x18, 0x13c, 0x35, leftSlide, 0.6328125f, 0.00390625f, 0.63671875f,
                                              0.7f, 0.7f, 0.0f, 0.0f);
        }
        else
        {
            float grey = (float)DAT_104bf88c * 0.049999997f;
            int opening = DAT_104c0108 - 10;
            int openFrames = opening < 1 ? 0 : opening;
            float rightSlide = ((1.0f - (float)pow((double)(1.0f - (float)openFrames / 60.0f), 2.0)) * 256.0f) / 256.0f;
            HUD_TEXTURE(texWaku03).DrawTinted(0x13c, 0x18,
                                              (int)((1.0f - (float)pow((double)(1.0f - (float)openFrames / 60.0f), 2.0)) *
                                                        145.0f + 316.0f),
                                              0x35, 0.00390625f, 0.7109375f, rightSlide, 0.71484375f, 0.7f, 0.7f, 0.0f,
                                              grey);
            opening = DAT_104c0108 - 10;
            openFrames = opening < 1 ? 0 : opening;
            float leftSlide = ((1.0f - (float)pow((double)(1.0f - (float)openFrames / 60.0f), 2.0)) * 256.0f) / 256.0f;
            HUD_TEXTURE(texWaku03).DrawTinted((int)(316.0f - (1.0f - (float)pow((double)(1.0f - (float)openFrames / 60.0f), 2.0)) *
                                                                145.0f),
                                              0x18, 0x13c, 0x35, leftSlide, 0.7109375f, 0.00390625f, 0.71484375f,
                                              0.7f, 0.7f, 0.0f, grey);
        }

        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);

        // The strip of tick marks along an arc from x = 443 down to 175 (mirrored on the right). The marks in the
        // middle of the strip are collected for the target ring.
        int arc[STRIP_ARC_COUNT];
        int markerOffsets[16];
        int markerCount;
        arc[0] = 0;
        arc[3] = 0x1b;
        arc[11] = 0x1b;
        markerCount = 0;
        arc[1] = 0xb;
        arc[2] = 0x14;
        arc[4] = 0x20;
        arc[5] = 0x23;
        arc[6] = 0x25;
        arc[7] = 0x26;
        arc[8] = 0x25;
        arc[9] = 0x23;
        arc[10] = 0x20;
        arc[12] = 0x14;
        arc[13] = 0xb;
        arc[14] = 0;
        int step = -1;
        int leftX = 443;
        int rightX = 203;
        do
        {
            int phase = step + 450 + DAT_1032d6a0;
            if (leftX < 383)
            {
                if ((phase % STRIP_TICK_SPACING == 0 || DAT_104c0128 != 0) && leftX < 324)
                {
                    HUD_TEXTURE(texWaku03).DrawTinted(rightX - 8, 0x26, rightX - 6, 0x28, 0.0625f, 0.1875f, 0.0625f,
                                                      0.1875f, 0.7f, 0.5f, 0.0f, 0.0f);
                    HUD_TEXTURE(texWaku03).DrawTinted(leftX - 8, 0x26, leftX - 6, 0x28, 0.0625f, 0.1875f, 0.0625f,
                                                      0.1875f, 0.7f, 0.5f, 0.0f, 0.0f);
                    if (DAT_104c0128 != 0)
                    {
                        HUD_TEXTURE(texWaku03).DrawTinted(rightX - 0x10, 0x1e, rightX, 0x2e, 0.0f, 0.125f, 0.125f,
                                                          0.25f, 0.7f, 0.5f, 0.0f, 0.0f);
                        HUD_TEXTURE(texWaku03).DrawTinted(leftX - 0x10, 0x1e, leftX, 0x2e, 0.0f, 0.125f, 0.125f,
                                                          0.25f, 0.7f, 0.5f, 0.0f, 0.0f);
                    }
                }
                if (leftX < 383 && leftX < 0x1f8)
                {
                    StripMarker *marker = DAT_108d9758;
                    do
                    {
                        if (marker > DAT_108d9758 && marker->phase == 0)
                            break;
                        if (marker->phase == phase % STRIP_LENGTH)
                        {
                            if (leftX < 324)
                            {
                                HUD_TEXTURE(texWaku03).DrawTinted(rightX - 0x10, 0x1e, rightX, 0x2e, 0.0f, 0.125f,
                                                                  0.125f, 0.25f, 0.7f, 0.5f, 0.0f, 0.0f);
                                HUD_TEXTURE(texWaku03).DrawTinted(leftX - 0x10, 0x1e, leftX, 0x2e, 0.0f, 0.125f,
                                                                  0.125f, 0.25f, 0.7f, 0.5f, 0.0f, 0.0f);
                            }
                            if (leftX < 0x14c && leftX > 0xbf)
                                markerOffsets[markerCount++] = step - 30;
                        }
                        marker++;
                    } while (marker < DAT_108d9758 + 32);
                }
            }
            step++;
            leftX -= 2;
            rightX += 2;
        } while (leftX > 0xaf);

        DAT_10202b38 += DAT_10f654b8;
        DAT_10202b34 += DAT_10f654b4;

        // The ring around what the player aims at: the target (or the player), projected to the screen.
        Vec4 screen;
        if (DAT_102024ac < 1)
        {
            screen.Copy(DAT_10db9210.GetTranslation());
            screen.z += 120.0f;
        }
        else
        {
            screen.Copy(DAT_10db6ee8[DAT_102024ac].matrix.GetTranslation());
            screen.z += 50.0f;
        }
        screen.Copy(Project(screen, DAT_108392d8));
        screen.x = (screen.x + 1.0f) * 320.0f;
        screen.y = (2.0f - (screen.y + 1.0f)) * 240.0f;
        int ringX = (int)screen.x;
        int ringY = (int)screen.y;
        if (DAT_102024ac > 0)
        {
            if (DAT_104c0124 == 0 || DAT_104c0131 > 1 || DAT_104c0154 == 0 || DAT_10db712a < 4)
            {
                Vec4 aimed;
                aimed.Copy(DAT_10db6ee8[DAT_102024ac].matrix.GetTranslation());
                aimed.z += 50.0f;
                aimed.Copy(Project(aimed, DAT_108392d8));
                aimed.x = (aimed.x + 1.0f) * 320.0f;
                aimed.y = (2.0f - (aimed.y + 1.0f)) * 240.0f;
                int aimedX = (int)aimed.x;
                int aimedY = (int)aimed.y;
                float fade = (float)DAT_104c0124 / 15.0f;
                if (1.0f <= fade)
                    fade = 1.0f;
                HUD_TEXTURE(texWaku04).DrawTinted(aimedX - 0x40, aimedY - 0x40, aimedX + 0x40, aimedY + 0x40, 0.0f,
                                                  0.0f, 1.0f, 1.0f, 0.7f,
                                                  ((float)(0x26 - arc[(DAT_1032d6a0 + STRIP_LENGTH) % STRIP_ARC_COUNT]) *
                                                       0.00005f + 0.3f) * fade,
                                                  0.0f, 0.0f);
            }
            else
            {
                float fade = (float)DAT_104c0124 / 15.0f;
                if (1.0f <= fade)
                    fade = 1.0f;
                for (int index = 0; index < markerCount; index++)
                {
                    int offset = markerOffsets[index];
                    float shrink = (float)(-offset + 64) / 64.0f;
                    int half = -(offset + 64);
                    int height = 0x26 - arc[(DAT_1032d6a0 + STRIP_LENGTH) % STRIP_ARC_COUNT];
                    HUD_TEXTURE(texWaku05).DrawTinted(ringX + half, ringY + half, ringX - half, ringY - half, 0.0f,
                                                      0.0f, 1.0f, 1.0f, 0.7f,
                                                      ((float)height * 0.00015f + 0.4f) * shrink * fade, 0.0f, 0.0f);
                    if (offset > 2 && DAT_104c0128 == 0)
                    {
                        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
                        float turn = (float)offset / 32.0f;
                        float angle = 3.1415f - turn;
                        int sideX = (int)((float)sin((double)angle) * 58.0f);
                        int sideY = (int)((float)cos((double)angle) * 58.0f);
                        float alpha = shrink * fade * 0.9f;
                        HUD_TEXTURE(texWaku03).DrawTinted(ringX + sideX - 12, ringY + sideY - 12, ringX + sideX + 12,
                                                          ringY + sideY + 12, 0.001953125f, 0.376953125f,
                                                          0.123046875f, 0.498046875f, 0.7f, alpha, turn, 0.0f);
                        angle = turn + 3.1415f;
                        sideX = (int)((float)sin((double)angle) * 58.0f);
                        sideY = (int)((float)cos((double)angle) * 58.0f);
                        HUD_TEXTURE(texWaku03).DrawTinted(ringX + sideX - 12, ringY + sideY - 12, ringX + sideX + 12,
                                                          ringY + sideY + 12, 0.001953125f, 0.376953125f,
                                                          0.123046875f, 0.498046875f, 0.7f, alpha, turn, 0.0f);
                        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
                    }
                }
                int pulseHeight = 0x26 - arc[(DAT_1032d6a0 + STRIP_LENGTH) % STRIP_ARC_COUNT];
                HUD_TEXTURE(texWaku05).DrawTinted(ringX - 0x40, ringY - 0x40, ringX + 0x40, ringY + 0x40, 0.0f, 0.0f,
                                                  1.0f, 1.0f, 0.7f, ((float)pulseHeight * 0.00005f + 0.3f) * fade,
                                                  0.0f, 0.0f);
            }
        }
        DAT_10202b34 -= DAT_10f654b4;
        DAT_10202b38 -= DAT_10f654b8;
        int pulse = arc[(DAT_1032d6a0 + STRIP_LENGTH) % STRIP_ARC_COUNT] / 2;
        HUD_TEXTURE(texWaku03).DrawTinted(0x12b, 0x185 - 376 - pulse, 0x14b, 0x1a5 - 376 - pulse, 0.0f, 0.0f, 0.125f,
                                          0.125f, 0.7f, 0.5f, 0.0f, 0.0f);
        HUD_TEXTURE(texWaku03).DrawTinted(0x13b, 0x18, 0x13c, 0x35, 1.0f, 0.7109375f, 1.0f, 0.71484375f, 0.7f, 0.7f,
                                          0.0f, 0.0f);
        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
        if (DAT_104c011b != 0)
        {
            HUD_TEXTURE(texWaku01).DrawTinted(0x2a, 0x3b, 0x7a, 0x53, 0.6875f, 0.25f, 0.9375f, 0.3125f, 0.0f,
                                              (float)DAT_104c011b, 0.0f, 0.0f);
            FUN_10012990(0x16, 0x40, DAT_104c0114, ((float)DAT_104c011b / 60.0f) * 0.9f, 2,
                         (float)DAT_104c011a / 10.0f + 0.5f, (float)DAT_104c011a / 10.0f);
        }
        int numberWidth = 0;
        for (int slot = 0; slot < 0x20; slot++)
        {
            if (DAT_10255040[slot] != 0)
            {
                int rise = (int)((float)(DAT_1020d5c8[slot] * DAT_1020d5c8[slot]) * 0.1f);
                if (DAT_102196f0[slot] >= 0)
                    numberWidth = FUN_10012990(0x11c, rise + 0x20, DAT_102196f0[slot],
                                               (float)(0x1e - DAT_1020d5c8[slot]) / 30.0f, DAT_10255040[slot], 0.0f,
                                               0.0f);
                float alpha = (float)(0x1e - DAT_1020d5c8[slot]);
                int top = rise + 0x20;
                int bottom = rise + 0x30;
                if (DAT_10255040[slot] == 1)
                    HUD_TEXTURE(texWaku01).DrawTinted(numberWidth + 0x11c, top, numberWidth + 0x17c, bottom, 0.0f,
                                                      0.0625f, 0.375f, 0.125f, 0.0f, alpha, 0.0f, 0.0f);
                if (DAT_10255040[slot] == 2)
                    HUD_TEXTURE(texWaku01).DrawTinted(numberWidth + 0x11c, top, numberWidth + 0x15c, bottom, 0.0f,
                                                      0.125f, 0.25f, 0.1875f, 0.0f, alpha, 0.0f, 0.0f);
                if (DAT_10255040[slot] == 3)
                    HUD_TEXTURE(texWaku01).DrawTinted(numberWidth + 0x11c, top, numberWidth + 0x1cc, bottom, 0.0f,
                                                      0.1875f, 0.6875f, 0.25f, 0.0f, alpha, 0.0f, 0.0f);
                if (DAT_10255040[slot] == 4)
                    HUD_TEXTURE(texWaku01).DrawTinted(numberWidth + 0x11c, top, numberWidth + 0x16c, bottom, 0.5f,
                                                      0.0625f, 0.8125f, 0.125f, 0.0f, alpha, 0.0f, 0.0f);
                if (DAT_10255040[slot] == 5)
                    HUD_TEXTURE(texWaku01).DrawTinted(numberWidth + 0x11c, top, numberWidth + 0x16c, bottom, 0.625f,
                                                      0.0f, 0.9375f, 0.0625f, 0.0f, alpha, 0.0f, 0.0f);
                if (DAT_10255040[slot] == 6)
                    HUD_TEXTURE(texWaku01).DrawTinted(numberWidth + 0x11c, top, numberWidth + 0x1ac, bottom, 0.4375f,
                                                      0.125f, 1.0f, 0.1875f, 0.0f, alpha, 0.0f, 0.0f);
            }
        }
    }
    if (DAT_10f654cc != 0 && DAT_104c008c != 0 && DAT_104bfe6c == 0 && DAT_104c02e6 == 0)
        DAT_118d0ba8[DAT_108360fc].DrawFrameAltStretched(0, 0x17c, 0x80, 0x1e0, 0.2f, 0.5f, 1, 0);
    if (DAT_104bfe1c != 0)
    {
        if (DAT_104bfe6c == 0 && DAT_104c02e6 == 0)
        {
            FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
            float scale = 1.0f;
            int barX = 0x104;
            for (float *level = DAT_104bff24; level < DAT_104bff24 + 25; level++)
            {
                float height = scale * *level * 0.2f;
                if (height < 0.0f)
                    height = 0.0f;
                else if (64.0f < height)
                    height = 64.0f;
                int steps = (int)(height / 3.0f);
                HUD_TEXTURE(texWaku03).DrawTinted(barX, steps * -3 + 0x160, barX + 8, 0x160, 0.50390625f,
                                                  (float)(steps * -3 + 0x40) / 256.0f, 0.52734375f, 0.25f, 0.8f, 1.0f,
                                                  0.0f, 0.0f);
                scale += 0.13f;
                barX += 8;
            }
            scale = 1.0f;
            barX = 0xe4;
            for (float *level = DAT_104bffa4; level < DAT_104bffa4 + 25; level++)
            {
                float height = scale * *level * 0.2f;
                if (height < 0.0f)
                    height = 0.0f;
                else if (64.0f < height)
                    height = 64.0f;
                int steps = (int)(height / 3.0f);
                HUD_TEXTURE(texWaku03).DrawTinted(barX, steps * -3 + 0x160, barX + 8, 0x160, 0.50390625f,
                                                  (float)(steps * -3 + 0x40) / 256.0f, 0.52734375f, 0.25f, 0.8f, 1.0f,
                                                  0.0f, 0.0f);
                scale += 0.13f;
                barX -= 8;
            }
            FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
        }
    }
    if (DAT_104bfe6c == 0 && DAT_104c02e6 == 0)
    {
        {
            if (DAT_102028d8 == -1)
                DAT_102028d8 = LoadTexture("ico01.bmp", "ico01a.bmp");
            HUD_TEXTURE(texWaku01).DrawFrameAltPlain(0x22a, 5, 0x25e, 0x39, 0.2f, 0.5f, 1, 0, 0);
            HUD_TEXTURE(texWaku01).DrawFrameAltPlain(0x1fa, 5, 0x22e, 0x39, 0.2f, 0.5f, 1, 0, 0);
            HUD_TEXTURE(texWaku01).DrawTinted(0x202, 5, 0x222, 0x15, 0.375f, 0.4375f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
                                              0.0f);
            HUD_TEXTURE(texWaku01).DrawTinted(0x232, 5, 0x252, 0x15, 0.5f, 0.4375f, 0.625f, 0.5f, 0.0f, 1.0f, 0.0f,
                                              0.0f);
            if (DAT_10daabb4 > 0)
            {
                int icon = DAT_1088e6b0[DAT_10daabb4].icon;
                float cellU = (float)(icon % 8) * 0.125f;
                float cellV = (float)(icon / 8) * 0.125f;
                HUD_TEXTURE(texIcon01).DrawTinted(0x234, 0xf, 0x254, 0x2f, cellU + 0.002f, cellV + 0.002f,
                                                  cellU + 0.125f - 0.002f, cellV + 0.125f - 0.002f, 0.0f, 1.0f, 0.0f,
                                                  0.0f);
            }
            if (DAT_10daabb8 > 0)
            {
                int icon = DAT_1088e6b0[DAT_10daabb8].icon;
                float cellU = (float)(icon % 8) * 0.125f;
                float cellV = (float)(icon / 8) * 0.125f;
                HUD_TEXTURE(texIcon01).DrawTinted(0x204, 0xf, 0x224, 0x2f, cellU + 0.002f, cellV + 0.002f,
                                                  cellU + 0.125f - 0.002f, cellV + 0.125f - 0.002f, 0.0f, 1.0f, 0.0f,
                                                  0.0f);
            }
        }
    }
    if (DAT_104bfd28 != 0)
        ShopMenu_Draw();
    if (DAT_104bfe6c == 0 && DAT_104c02e6 == 0)
        FieldItemMenu_Draw();
    if (DAT_104bfd78 != 0)
        FUN_1000f4d0();
    DAT_104b7200.DrawWindows(DAT_104bf88c);
    DAT_104b7200.DrawWindowMarks(DAT_104bf88c);
    int menuMode = DAT_104bfd74;
    DAT_10202b34 += DAT_10f654b4;
    DAT_10202b38 += DAT_10f654b8;
    if (DAT_104bfd74 == 0)
    {
        if (DAT_1020245c != 0)
        {
            if (DAT_104bfe1c == 0)
            {
                if (DAT_104bf8f8[0] == 0 || DAT_1021d138[0] != 8)
                {
                    DAT_104bf8f8[0] = 1;
                    DAT_1021d138[0] = 8;
                }
                DAT_1021f740[0] = 0x1e;
            }
            DAT_104bfe7c = 0x1e;
        }
        DAT_1020245c = 0;
    }
    else if (DAT_104bfd74 != 4)
    {
        if (DAT_104bfd74 == 2)
        {
            if (DAT_1020245c != 10)
                DAT_104bfe7c = 0x1e;
            DAT_1020245c = 10;
        }
        else if (DAT_104bfd74 == 3)
        {
            if (DAT_1020245c != 7)
            {
                if (DAT_104bfe1c == 0)
                {
                    if (DAT_104bf8f8[0] == 0 || DAT_1021d138[0] != 0xb)
                    {
                        DAT_104bf8f8[0] = 1;
                        DAT_1021d138[0] = 0xb;
                    }
                    DAT_1021f740[0] = 0x1e;
                }
                DAT_104bfe7c = 0x1e;
            }
            DAT_1020245c = 7;
        }
        else if (DAT_104bfd74 == 1)
        {
            if (DAT_1020245c != 8)
            {
                if (DAT_104bfe1c == 0)
                    FUN_1000d620(0, 10, 0x1e);
                DAT_104bfe7c = 0x1e;
            }
            DAT_1020245c = 8;
        }
    }
    DAT_104bfde0 = menuMode;

    // The numbers and labels floating over the field: slot i is alive for DAT_104463f8[i] frames (30 at most).
    for (int slot = 0; slot < 0x60; slot++)
    {
        if (DAT_1020d660[slot] != 0 && DAT_104463f8[slot] != 0)
        {
            int frames = DAT_104463f8[slot];
            int value = DAT_10468230[slot];
            if (value == -100000)
            {
                FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
                HUD_TEXTURE(texWaku02).DrawTinted(DAT_10253c58[slot] - 0x4c, DAT_10256218[slot] - 0x30,
                                                  DAT_10253c58[slot] + 0x50, DAT_10256218[slot] - 0x10, 0.0f, 0.25f,
                                                  0.625f, 0.375f, 0.0f, 1.0f - (float)frames / 30.0f, 0.0f, 0.0f);
                FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            }
            else if (value == -100001)
            {
                FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
                HUD_TEXTURE(texWaku02).DrawTinted(DAT_10253c58[slot] - 0x4c, DAT_10256218[slot] - 0x30,
                                                  DAT_10253c58[slot] + 0x50, DAT_10256218[slot] - 0x10, 0.0f, 0.375f,
                                                  0.625f, 0.5f, 0.0f, 1.0f - (float)frames / 30.0f, 0.0f, 0.0f);
                FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            }
            else if (value == -100002)
            {
                FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
                HUD_TEXTURE(texWaku02).DrawTinted(DAT_10253c58[slot] - 0x4c, DAT_10256218[slot] - 0x30,
                                                  DAT_10253c58[slot] + 0x50, DAT_10256218[slot] - 0x10, 0.0f, 0.5f,
                                                  0.625f, 0.625f, 0.0f, 1.0f - (float)frames / 30.0f, 0.0f, 0.0f);
                FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
            }
            else if (DAT_1021f640[slot] == 3)
            {
                FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
                if (frames < 8)
                {
                    DAT_104b7200.DrawLabelledNumber(value, DAT_10253c58[slot], DAT_10256218[slot], 0.0f,
                                                    (float)frames * 0.125f, (float)frames / 30.0f + 0.7f, 0.0f);
                    FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
                }
                else
                {
                    float fade = frames < 0x17 ? 1.0f : (float)(0x1e - frames) * 0.125f;
                    DAT_104b7200.DrawLabelledNumber(value, DAT_10253c58[slot], DAT_10256218[slot], 0.0f, fade,
                                                    (float)frames / 30.0f + 0.7f, 0.0f);
                    FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
                }
            }
            else if (DAT_1021f640[slot] == 2)
            {
                float fade;
                float scale = (float)frames / 30.0f + 0.7f;
                float alpha;
                if (frames < 8)
                {
                    fade = (float)frames * 0.125f;
                    alpha = (float)(8 - frames) * 0.4f;
                }
                else if (frames > 0x16)
                {
                    fade = (float)(0x1e - frames) * 0.125f;
                    alpha = (float)(frames - 0x16) * 0.4f;
                }
                else
                {
                    fade = 1.0f;
                    alpha = 0.0f;
                }
                DAT_104b7200.DrawNumber(value, DAT_10253c58[slot], DAT_10256218[slot], 0.0f, fade, scale, alpha);
            }
            else
            {
                float fade;
                float scale;
                float alpha;
                if (DAT_1021f640[slot] == 1)
                    scale = (float)frames / 20.0f + 1.2f;
                else
                    scale = (float)frames / 30.0f + 0.7f;
                if (frames < 8)
                {
                    fade = (float)frames * 0.125f;
                    alpha = (float)(8 - frames) * 0.4f;
                }
                else if (frames > 0x16)
                {
                    fade = (float)(0x1e - frames) * 0.125f;
                    alpha = (float)(frames - 0x16) * 0.4f;
                }
                else
                {
                    fade = 1.0f;
                    alpha = 0.0f;
                }
                DAT_104b7200.DrawNumberAlt(value, DAT_10253c58[slot], DAT_10256218[slot], 0.0f, fade, scale, alpha);
            }
        }
    }
    if (DAT_104bfe6c == 0 && DAT_104c02e6 == 0)
    {
        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
        unsigned char timer = (unsigned char)DAT_104c00bd;
        if (timer < 0x1e)
        {
            int widthLeft = timer * 4;
            int heightHalf = timer / 2;
            if (DAT_104c01b4 == 0)
                HUD_TEXTURE(texWaku02).DrawTinted(0x140 - widthLeft, 0xc8 - heightHalf, 0x140 + widthLeft,
                                                  0xc8 + heightHalf, 0.625f, 0.5625f, 1.0f, 0.625f, 0.0f, 1.0f, 0.0f,
                                                  0.0f);
            else
                HUD_TEXTURE(texWaku02).DrawTinted(0x140 - widthLeft, 0xc8 - heightHalf, 0x140 + widthLeft,
                                                  0xc8 + heightHalf, 0.625f, 0.625f, 1.0f, 0.6875f, 0.0f, 1.0f, 0.0f,
                                                  0.0f);
        }
        else
        {
            float alpha = (float)(0x3c - timer) / 30.0f;
            if (DAT_104c01b4 == 0)
                HUD_TEXTURE(texWaku02).DrawTinted(0xc8, 0xb9, 0x1b8, 0xd7, 0.625f, 0.5625f, 1.0f, 0.625f, 0.0f,
                                                  alpha, 0.0f, 0.0f);
            else
                HUD_TEXTURE(texWaku02).DrawTinted(0xc8, 0xb9, 0x1b8, 0xd7, 0.625f, 0.625f, 1.0f, 0.6875f, 0.0f,
                                                  alpha, 0.0f, 0.0f);
        }
        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    }
    DAT_10202b34 -= DAT_10f654b4;
    DAT_10202b38 -= DAT_10f654b8;
    if (DAT_104bf8d4 != 0)
        FUN_1000cc30();
    if (DAT_104bfe6c == 0 && DAT_104c02e6 == 0)
    {
        if (DAT_104bfe78 == 0)
        {
            if (DAT_104c0232 != 0)
                HUD_TEXTURE(texWaku03).DrawTinted(0x251, 0x1b1, 0x281, 0x1e1, 0.8125f, 0.3125f, 0.99609375f,
                                                  0.49609375f, 0.0f, 1.0f, 0.0f, 0.0f);
            else
                HUD_TEXTURE(texWaku03).DrawTinted(0x250, 0x1b0, 0x280, 0x1e0, 0.8125f, 0.3125f, 0.99609375f,
                                                  0.49609375f, 0.0f, 1.0f, 0.0f, 0.0f);
        }
        if (DAT_104bfe6c == 0 && DAT_104c02e6 == 0)
        {
            CharSlot *watched = &DAT_10daaae8[DAT_104c0110];
            if (DAT_104c0110 != 0 && watched->stats[STAT_MAX_HEALTH] != 0)
            {
                if (watched->stats[STAT_HEALTH] < 0)
                    watched->stats[STAT_HEALTH] = 0;
                HUD_TEXTURE(texWaku01).DrawTinted(0x168, 0x1c8, 0x248, 0x1d8, 0.001953125f, 0.376953125f,
                                                  0.873046875f, 0.435546875f, 0.0f, 0.8f, 0.0f, 0.0f);
                HUD_TEXTURE(texWaku03).DrawTinted(0x198, 0x1cd,
                                                  watched->stats[STAT_HEALTH] * 0xa3 /
                                                          watched->stats[STAT_MAX_HEALTH] + 0x198,
                                                  0x1d3, 0.15625f, 0.2734375f, 0.15625f, 0.2734375f, 0.0f, 0.9f, 0.0f,
                                                  0.0f);
                HUD_TEXTURE(texWaku03).DrawTinted(watched->stats[STAT_HEALTH] * 0xa3 /
                                                          watched->stats[STAT_MAX_HEALTH] + 0x198,
                                                  0x1cd, 0x23b, 0x1d3, 0.9375f, 0.546875f, 0.9375f, 0.546875f, 0.0f,
                                                  0.9f, 0.0f, 0.0f);
            }
            HUD_TEXTURE(texWaku03).DrawTinted(0x10, 0x2b, 0xa0, 0x34, 0.314453125f, 0.255859375f, 0.873046875f,
                                              0.287109375f, 0.7f, 0.8f, 0.0f, 0.0f);
            HUD_TEXTURE(texWaku03).DrawTinted(0x14, 0x2e,
                                              DAT_10daaae8[0].stats[STAT_HEALTH] * 0x7e /
                                                      DAT_10daaae8[0].stats[STAT_MAX_HEALTH] + 0x14,
                                              0x31, 0.01953125f, 0.28125f, 0.01953125f, 0.28125f, 0.7f, 0.8f, 0.0f,
                                              (float)DAT_104bf88c * 0.049999997f);
            HUD_TEXTURE(texWaku02).DrawTinted(0x24, 0x1e, 0x40, 0x2c, 0.0f, 0.0f, 0.125f, 0.0625f, 0.7f, 1.0f, 0.0f,
                                              0.0f);
            HUD_TEXTURE(texWaku02).DrawTinted(0x64, 0x1c, 0x74, 0x2c, 0.25f, 0.0f, 0.3125f, 0.0625f, 0.7f, 1.0f,
                                              0.0f, 0.0f);
            if (DAT_10daaae8[0].stats[STAT_HEALTH] < DAT_10daaae8[0].stats[STAT_MAX_HEALTH] / 4)
            {
                int health = DAT_10daaae8[0].stats[STAT_HEALTH];
                if (health < 0)
                {
                    health = 0;
                    DAT_10daaae8[0].stats[STAT_HEALTH] = 0;
                    FUN_1000e750();
                }
                float shake = (float)DAT_104c011d / 10.0f;
                FUN_10012d90(100, 0x1c, health, 1.0f, 2, shake, shake);
            }
            else if (DAT_10daaae8[0].stats[STAT_HEALTH] < DAT_10daaae8[0].stats[STAT_MAX_HEALTH] / 2)
            {
                float shake = (float)DAT_104c011d / 10.0f;
                FUN_10012d90(100, 0x1c, DAT_10daaae8[0].stats[STAT_HEALTH], 1.0f, 1, shake, shake);
            }
            else
            {
                float shake = (float)DAT_104c011d / 10.0f;
                FUN_10012d90(100, 0x1c, DAT_10daaae8[0].stats[STAT_HEALTH], 1.0f, 0, shake, shake);
            }
            FUN_10012d90(0x90, 0x1c, DAT_10daaae8[0].stats[STAT_MAX_HEALTH], 1.0f, 0, 0.0f, 0.0f);
            int pickupX = 0;
            if (DAT_10c64ad0 != 0)
                pickupX = 0x10;
            HUD_TEXTURE(texWaku01).DrawTinted(pickupX + 0x190, 4, pickupX + 0x1b0, 0x14, 0.1875f, 0.4375f, 0.3125f,
                                              0.5f, 0.0f, 1.0f, 0.0f, 0.0f);
            float pickupShake = (float)DAT_104c0119 / 10.0f;
            FUN_10012990(pickupX + 0x1b0, 4, DAT_10daaae8[0].stats[STAT_PICKUPS], 1.0f, 3, pickupShake, pickupShake);
            if (DAT_10c655f0 == 7)
                DAT_1020e538.DrawOutlined(0x26, 6, DAT_10daaae8[0].name, 0xffffff, 0.7f, 1.0f, 0.8f);
            else if (DAT_10c655f0 == 6)
                DAT_1020e538.DrawOutlined(0x26, 6, "\x83|\x83|\x83\x93", 0xffffff, 0.7f, 1.0f, 0.8f);
            else
                DAT_1020e538.DrawOutlined(0x26, 6, DAT_10daaae8[0].name, 0xffffff, 0.7f, 1.0f, 0.8f);
            HUD_TEXTURE(texMenu1).DrawTinted(4, 2, 0x24, 0x22, 0.0f, 0.5f, 0.1875f, 0.6875f, 0.0f, 1.0f, 0.0f, 0.0f);
            HUD_TEXTURE(texWaku02).DrawTinted(0x86, 0x1c2, 0xb6, 0x1da, 0.25f, 0.875f, 0.5f, 1.0f, 0.0f, 1.0f, 0.0f,
                                              0.0f);
            int coins = DAT_10daaae8[0].coins;
            if (coins > 99999)
            {
                coins = 99999;
                DAT_10daaae8[0].coins = 99999;
                FUN_1000e750();
            }
            float coinShake = (float)DAT_104c011e / 10.0f;
            FUN_100131b0(0xac, 0x1c9, coins, 1.0f, 0, coinShake, coinShake);
            HUD_TEXTURE(texWaku02).DrawTinted(0x106, 0x1c2, 0x136, 0x1da, 0.0f, 0.875f, 0.25f, 1.0f, 0.0f, 1.0f, 0.0f,
                                              0.0f);
            int gems = DAT_10c6670c;
            if (gems > 999)
            {
                gems = 999;
                DAT_10c6670c = 999;
                FUN_1000e750();
            }
            float gemShake = (float)DAT_104c011f / 10.0f;
            FUN_100131b0(300, 0x1c9, gems, 1.0f, 0, gemShake, gemShake);
            FUN_100236c0();
        }
    }
    if (DAT_104bf8c8 != 0)
        FUN_1000eed0();
    DAT_10202b34 += DAT_10f654b4;
    DAT_10202b38 += DAT_10f654b8;
    Matrix4 scratch;
    FUN_1013f070(RENDER_CULL_MODE, 0, 0);
    if (DAT_104bfe6c == 0 && DAT_104c02e6 == 0)
        FUN_1007d0a0();
    if (DAT_104c00d0 > 0x7d9 && DAT_104c00d0 < 0x80d)
    {
        // The drill model of the cutscene: scaled up by 1.5, turned 0.1 radian a frame and placed at the left hand,
        // lifted by 20.
        Matrix4 placement;
        Node *hand = DAT_10db6ee8[0].root->Find("Bip01 L Hand", 0);
        placement = DAT_10db9210;
        placement = Scaling(1.5f, 1.5f, 1.5f) * RotationZ((float)DAT_104c00d0 * 0.1f) * placement;
        if (hand != 0)
            placement.SetTranslation(hand->worldMatrix.GetTranslation());
        placement.m[14] += 20.0f;
        Vec4 colour;
        colour.Fill(1.0f);
        colour.w = (float)(DAT_104c00d0 - 0x7da) / 10.0f;
        DAT_104d2a08[DAT_104c0198].StartProximityPass(placement.m, &DAT_108392d8, &DAT_108392d8, 0, 0);
        DAT_104687b0 = 1;
        DAT_104d2a08[DAT_104c0198].QueueDraw(&placement, &DAT_108392d8,
                                             DAT_10804bf8.LightDirection(0, &DAT_118e69e8, &DAT_118e69e8), -1,
                                             &colour, 0);
    }
    if (DAT_104c02e6 != 0)
    {
        FUN_10143e00(0, 0, 640, 0x22, 0, 0.0f, 1.0f);
        FUN_10143e00(0, 0x1be, 640, 480, 0, 0.0f, 1.0f);
    }
    if (DAT_104bfe6c == 0 ? DAT_104c0088 != 0 : (DAT_104bfe6c != 2 && DAT_104bfe6c != 3))
    {
        DAT_10c68000.DrawPictures(DAT_108392d8);
        DAT_10c68000.DrawScreenOverlays();
    }
    if (DAT_104bfe6c == 3)
    {
        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
        int left = (640 - DAT_10836eb8) / 2;
        int top = (480 - DAT_10836ebc) / 2;
        DAT_118d0ba8[DAT_10202478].DrawTinted(left, top, DAT_10836eb8 + left, top + DAT_10836ebc,
                                              0.00048828125f, 0.0009765625f, ((float)DAT_10836eb8 - 0.5f) / 1024.0f,
                                              ((float)DAT_10836ebc - 0.5f) / 512.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    }
    else if (DAT_104bfe6c == 2)
    {
        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
        int left = (640 - DAT_11b1bb58) / 2;
        int top = (480 - DAT_11b1bb64) / 2;
        DAT_118d0ba8[DAT_10202478].DrawTinted(left, top, DAT_11b1bb58 + left, top + DAT_11b1bb64,
                                              0.00048828125f, 0.0009765625f, ((float)DAT_11b1bb58 - 0.5f) / 1024.0f,
                                              ((float)DAT_11b1bb64 - 0.5f) / 512.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    }
    else if (DAT_104bfe70 != 0 && DAT_10f08afb != 0)
    {
        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
        float screenU = (float)(DAT_10f08af0 - 2) / 1024.0f;
        float screenV = (float)(DAT_10f08af2 - 2) / 512.0f;
        DAT_118d0ba8[DAT_10202478].DrawTinted(0x50, 0x36, 0x230, 0xae, 0.001953125f, 0.00390625f, screenU, screenV,
                                              0.0f, 1.0f, 0.0f, 0.0f);
        DAT_118d0ba8[DAT_10202478].DrawTinted(0x50, 0x36, 0x230, 0xae, 0.001953125f, 0.00390625f, screenU, screenV,
                                              0.0f, 1.0f, 0.0f, 0.0f);
        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    }
    if (0.1f < DAT_104bf8f0)
        FUN_1000dba0();
    if (DAT_104bfe78 != 0 && DAT_10f08afb != 0)
    {
        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ADD, 0);
        int pictureWidth = DAT_10f08af0;
        int pictureHeight = DAT_10f08af2;
        int left = 0x140 - pictureWidth / 2;
        int top = 0xbe - pictureHeight / 2;
        DAT_118d0ba8[DAT_10202478].DrawTinted(left, top, left + pictureWidth, top + pictureHeight, 0.001953125f,
                                              0.00390625f, (float)(pictureWidth - 2) / 1024.0f,
                                              (float)(pictureHeight - 2) / 512.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        FUN_1013f070(RENDER_BLEND_MODE, BLEND_ALPHA, 0);
    }
    DAT_10202b34 -= DAT_10f654b4;
    DAT_10202b38 -= DAT_10f654b8;
    DAT_1020d9e0.M_100014b0();
    return 1;
}
