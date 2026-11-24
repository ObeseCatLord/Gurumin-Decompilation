// The title screen (game mode 0): the logo pieces and the menu frame over a scrolling background, with the main menu
// (Menu_Main) running on top. C++ rather than extern "C": the function keeps an exception frame for its Matrix4 locals.

#include <stdio.h>
#include <windows.h>

#include "vec4.h"

#include "matrix4.h"

#include "node.h"

#include "anim_model.h"

#include "model_bank.h"

#include "tracker.h"
#include "world_renderer.h"

#include "bgm_spectrum.h"
#include "bgm_track.h"
#include "camera_refresh_vars.h"
#include "char_vars.h"
#include "dinput_min.h"
#include "doppler_loops.h"
#include "field_item_menu.h"
#include "frame_vars.h"
#include "game.h"
#include "char_hikari.h"
#include "sound_vars.h"
#include "game_load_data.h"
#include "game_vars.h"
#include "input_state.h"
#include "item_dictionary.h"
#include "map_vars.h"
#include "menu_save_load.h"
#include "menu_scene.h"
#include "render_state.h"
#include "render_vars.h"
#include "texture_load.h"
#include "texture_object.h"
#include "title_screen.h"
#include "ui_vars.h"

Matrix4 IdentityMatrix();                                // 0x101b0ba0

// Engine command numbers of Render_Command (render_state.h) that the frame uses by value.
#define CLEAR_DEPTH_FULL 0xffff
#define ALPHA_TEST_REFERENCE 0xf
#define OPAQUE_FOG_COLOUR 0xedba67
// The title lasts: the music starts at frame MUSIC_START_FRAME, the fade-in ends at FADE_FRAME, the logo pieces start
// to flicker after FLICKER_START_FRAME; the menu opens (state 1) on a key and the pieces slide away for SLIDE_FRAMES.
#define MUSIC_START_FRAME 0x20
#define MUSIC_TRACK 8
#define FADE_FRAME 0x1e
#define FLICKER_START_FRAME 0x3c
#define RISE_FRAMES 0xf
#define SLIDE_SPEED 0x18
#define FLICKER_ODDS 100
#define FLICKER_MAX 10
#define FLICKER_STEP 2
#define MOUSE_PRESSED 2
#define START_SOUND 0x357
#define FADE_AMOUNT 100.0f
#define FADE_STEP_FRAMES 15
// The title frame state DAT_104bfd14: 0 = waiting for a key, 1 = the key was pressed (the menu slides in).
#define TITLE_WAITING 0
#define TITLE_KEY_PRESSED 1
// The scrolling background moves this many texture widths per frame and repeats five by 3.75 times.
#define BACKGROUND_SCROLL_SPEED 0.02f
#define BACKGROUND_WIDTH_UVS 5.0f
#define BACKGROUND_HEIGHT_UVS 3.75f
#define BACKGROUND_Z 0.991f
#define PIECE_UV_MIN 0.002f
#define PIECE_UV_MAX 0.998f
#define SCREEN_WIDTH 0x280
#define SCREEN_HEIGHT 0x1e0
#define ANIM_MODEL_COUNT 124

// Shows the logo piece `slot` with its flicker `flick`: its top edge is `top - flick / 2`.
#define PIECE(slot, left, top, right, bottom)                                                                      \
    DAT_118d0ba8[DAT_10202808[slot]].DrawTinted(left, (int)((float)(top) - (float)DAT_10202888[slot] / 2.0f), right,   \
                                                bottom, PIECE_UV_MIN, PIECE_UV_MIN, PIECE_UV_MAX, PIECE_UV_MAX,   \
                                                0, 1.0f, 0, 0)

// 0x10114950 (TitleScreen_Update). The first frame loads the title textures, clears the scene and starts the fade
// from the silent title. Every frame draws the scrolling background, the logo pieces (they rise in over the first
// frames and slide out when a key opens the menu) and the menu frame, and once a new engine tick has started reads
// the controls, updates the world and the menu, flickers the logo pieces, starts the music and waits for a key or
// mouse click. Returns 0 when the mode ends or the window is closing, 1 otherwise.
int TitleScreen_Update()
{
    char fileName[0x100];
    char alphaName[0x100];
    char left;
    char right;
    if (DAT_10f6552c != 0)
        return 0;
    if (DAT_104bf8bc == 0)
    {
        DAT_104bfd14 = 0;
        DAT_10f6553c = FUN_10038010;
        DAT_104bfd18 = -1;
        DAT_104bf8d4 = 0;
        DAT_1032d67c[0] = 0;
        DAT_104bf8bc = 1;
        DAT_104bf890 = 0;
        if (DAT_104b7210 >= 0)
            FUN_10174c70(DAT_104b7210, 0);
        DAT_104b7210 = -1;
        DAT_104b7210 = FUN_10173550(FUN_1013f8a0("menu3.bmp", 0), FUN_1013f8a0("menu3a.bmp", 0), 0);
        DAT_104c0096 = 0;
        DAT_104bf8c0 = 0;
        DAT_104c0324 = 1;
        DAT_104c0328 = 0;
        for (int piece = 0; piece < TITLE_PIECE_COUNT; piece++)
        {
            DAT_10202848[piece] = 0;
            DAT_10202888[piece] = 0;
            if (DAT_10202808[piece] < 0)
            {
                sprintf(fileName, "tit%03d.bmp", piece + 1);
                sprintf(alphaName, "tit%03da.bmp", piece + 1);
                DAT_10202808[piece] = FUN_10173550(FUN_1013f8a0(fileName, 0), FUN_1013f8a0(alphaName, 0), 0);
            }
        }
        DAT_1020283c = FUN_10173550(FUN_1013f8a0("menu6.bmp", 0), 0, 0);
        DAT_10202840 = FUN_10173550(FUN_1013f8a0("menu7.bmp", 0), FUN_1013f8a0("menu7a.bmp", 0), 0);
        DAT_10da90d0.Set(0.9f, 0.9f, 0.9f, 1.0f);
        DAT_1042da68 = 0;
        DAT_10db6ee8[0].SetPosition(-200000.0f, 0, 0);
        DAT_104c012c = 0;
        Bgm_Play(0, 0);
        DAT_104bf8f0 = FADE_AMOUNT;
        DAT_104bf8e8 = 0;
        DAT_104bf8ec = 0;
        DAT_1032d4b0 = -6.6666665f;
        DAT_104bf8e4 = FADE_STEP_FRAMES;
        FUN_1013faf0();
        DAT_104bfd50 = 1;
    }
    DAT_104c00dc = 0;
    FUN_1013f070(0, 0, 0);
    FUN_1013f070(2, 0, 0);
    FUN_1013f070(7, 1, 0);
    FUN_1013f070(3, 1, ALPHA_TEST_REFERENCE);
    FUN_10199860();
    Matrix4 spareA;
    Matrix4 spareB;
    Matrix4 spareC;
    Matrix4 spareD;
    Matrix4 spareE;
    if (DAT_10f65504 != DAT_104bf8b8)
    {
        DAT_104c0328++;
        DAT_1020d9e0.M_10001110(0, 0, 0, 0, 0, 0);
        DAT_104bf8b8 = DAT_10f65504;
        DAT_10f08008.Update();
        DAT_10210f30 = DAT_10f08008.ReadControls(0);
        DAT_1020d9e0.M_10001550();
        FUN_1005c680();
        if (DAT_104bff00 != 0)
        {
            if (DAT_104bff0c < 0xf)
                DAT_104bff0c++;
        }
        else if (DAT_104bff0c > 0)
            DAT_104bff0c--;
        DAT_104bf8a4 = DAT_10d4ee30;
        DAT_10c68000.AdvanceFrame();
        DAT_1020d9e0.M_10001450();
        Camera_Refresh();
        DAT_10db9240 = 0;
        DAT_10db9244 = 0;
        DAT_10db9248 = -300.0f;
        Matrix4 spareF;
        Matrix4 spareG;
        spareG.SetIdentity();
        if (DAT_10255f08 != 0)
            DAT_10255f08->StartProximityPass(DAT_10836ec8.m, &DAT_108392d8, &DAT_10836330, DAT_104bf8a4, 0);
        if (DAT_10255f0c != 0)
            DAT_10255f0c->StartProximityPass(DAT_10836ec8.m, &DAT_10836cd8, &DAT_10836330, DAT_104bf8a4, 0);
        DAT_108654f8.StartProximityPass(DAT_10836ec8.m, &DAT_108392d8, &DAT_10836330, 0, 0);
        for (int index = 0; index < ANIM_MODEL_COUNT; index++)
            DAT_10db6ee8[index].UpdateFrame(&DAT_108392d8, 1, 1, 0, 0);
        DAT_1065cb28.SetButtons(DAT_10210f30);
        FUN_1003a9d0();
        if (DAT_104bf8c8 != 0)
            FUN_1000ee60();
        DAT_104bf890++;
        DAT_104bf88c += DAT_10202348;
        if (DAT_104bf88c == FLICKER_MAX || DAT_104bf88c == 0)
            DAT_10202348 = -DAT_10202348;
        DAT_1032d67c[0]++;
        DAT_104b7200.UpdateWindows();
        DAT_104b7200.MoveCursor();
        if (DAT_104bf8d4 == 1)
        {
            if (DAT_104bf8d8 < 0xf)
                DAT_104bf8d8++;
        }
        else if (DAT_104bf8d4 == 2 && DAT_104bf8d8 > 0)
            DAT_104bf8d8--;
        if (DAT_104bf8d8 == 0)
            DAT_104bf8d4 = 0;
        FUN_1015f4e0();
        FUN_10112140();
        if (DAT_104bf8c0 != 0)
            return 0;
        if (DAT_104bfd14 == TITLE_WAITING)
        {
            if (DAT_104bf890 == MUSIC_START_FRAME)
            {
                DAT_104c012c = MUSIC_TRACK;
                Bgm_Play(MUSIC_TRACK, 0);
            }
            if (DAT_104bfd14 == TITLE_WAITING && DAT_104bf890 == FADE_FRAME)
            {
                DAT_104bf8f0 = FADE_AMOUNT;
                DAT_104bf8e8 = 0;
                DAT_104bf8ec = 100;
                DAT_1032d4b0 = -20.0f;
                DAT_104bf8e4 = 5;
            }
        }
        if (DAT_104bfd14 != TITLE_WAITING || DAT_104bf890 != FADE_FRAME)
        {
            if (DAT_104bf890 > FLICKER_START_FRAME)
            {
                for (int piece = 0; piece < TITLE_PIECE_COUNT; piece++)
                {
                    if (DAT_10202888[piece] == 0 && FUN_1013f880() % FLICKER_ODDS == 0)
                    {
                        DAT_10202888[piece] = 1;
                        DAT_10202848[piece] = 1;
                    }
                    if (DAT_10202888[piece] != 0 && DAT_10202848[piece] != 0)
                        DAT_10202888[piece] += FLICKER_STEP;
                    if (DAT_10202888[piece] != 0 && DAT_10202848[piece] == 0)
                        DAT_10202888[piece] -= FLICKER_STEP;
                    if (DAT_10202888[piece] > FLICKER_MAX && DAT_10202848[piece] != 0)
                    {
                        DAT_10202888[piece] = FLICKER_MAX;
                        DAT_10202848[piece] = 0;
                    }
                }
                if (DAT_104bf890 > FLICKER_START_FRAME && DAT_104bfd14 == TITLE_WAITING)
                {
                    DAT_1020d9e0.M_10001110(0, &left, 0, &right, 0, 0);
                    if (left == MOUSE_PRESSED || right == MOUSE_PRESSED || DAT_1065cb38 != 0 || DAT_1065cb39 != 0 ||
                        DAT_1065cb36 != 0 || DAT_1065cb37 != 0 || DAT_1065cb34 != 0 || DAT_1065cb35 != 0 ||
                        DAT_1065cb31 != 0 || DAT_1065cb30 != 0 || DAT_1065cb32 != 0 || DAT_1065cb33 != 0)
                    {
                        DAT_1030c50c = 1;
                        DAT_104bfd14 = TITLE_KEY_PRESSED;
                        DAT_104bf890 = 0;
                        FUN_1003ad20(START_SOUND, 0, 1.0f);
                    }
                }
            }
            if (DAT_104bfd14 == TITLE_KEY_PRESSED && DAT_104c0324 == 1 && DAT_104bf890 > FADE_FRAME)
                DAT_104c0324 = 0;
        }
        DAT_10804bf8.PrepareFrame(&DAT_10836ec8, &DAT_108392d8, DAT_1032d67c[0], 1);
        if ((DAT_104bf890 & 1) != 0 && DAT_10255f08 != 0)
            DAT_10255f08->M_10194ee0();
    }
    DAT_10836ec8 = IdentityMatrix();
    FUN_10199860();
    FUN_1013f070(3, 1, ALPHA_TEST_REFERENCE);
    FUN_1013f070(4, OPAQUE_FOG_COLOUR, 0);
    float scroll = (float)DAT_104c0328 * BACKGROUND_SCROLL_SPEED;
    if (DAT_104bfd14 == TITLE_WAITING || DAT_104bf890 < FADE_FRAME)
        DAT_118d0ba8[DAT_104b7214].DrawTinted(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, scroll, scroll,
                                              scroll + BACKGROUND_WIDTH_UVS, scroll + BACKGROUND_HEIGHT_UVS,
                                              BACKGROUND_Z, 1.0f, 0, 0);
    if (DAT_104bfd14 == TITLE_KEY_PRESSED)
    {
        float progress = (float)DAT_104bf890 / 30.0f;
        DAT_118d0ba8[DAT_1020283c].DrawTinted(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, scroll, scroll,
                                              scroll + BACKGROUND_WIDTH_UVS, scroll + BACKGROUND_HEIGHT_UVS,
                                              BACKGROUND_Z, progress > 1.0 ? 1.0 : progress, 0, 0);
    }
    if (DAT_10255f08 != 0)
        FUN_10038c10(0);
    FUN_1013f070(0xe, 0, 0);
    FUN_1013f070(0, 5, 0);
    FUN_1013f070(0, 0, 0);
    FUN_1013f070(0xe, 0, 0);
    FUN_1013f070(0xb, CLEAR_DEPTH_FULL, 0);
    int slide = 0;
    if (DAT_104bfd14 == TITLE_KEY_PRESSED)
        slide = DAT_104bf890 * SLIDE_SPEED;
    int rise = 0;
    if (DAT_104bfd14 == TITLE_KEY_PRESSED)
    {
        if (DAT_104bf890 < RISE_FRAMES)
            rise = (RISE_FRAMES - DAT_104bf890) * SLIDE_SPEED;
        if (DAT_104bf890 < FADE_FRAME && DAT_104c0324 != 0)
        {
            unsigned int step = FADE_FRAME - DAT_104bf890;
            if (step >= FADE_FRAME)
                step = FADE_FRAME;
            DAT_118d0ba8[DAT_10202840].DrawTinted(0x50, 0, 0x230, SCREEN_HEIGHT, 0.002f, 0.002f, 0.998f, 0.998f, 0,
                                                  (float)step / 30.0f, 0, 0);
        }
    }
    if (DAT_104bfd14 == TITLE_WAITING)
    {
        unsigned int step = DAT_104bf890;
        if (DAT_104bf890 >= FADE_FRAME)
            step = FADE_FRAME;
        DAT_118d0ba8[DAT_10202840].DrawTinted(0x50, 0, 0x230, SCREEN_HEIGHT, 0.002f, 0.002f, 0.998f, 0.998f, 0,
                                              (float)step / 30.0f, 0, 0);
    }
    if (DAT_104bfd14 == TITLE_KEY_PRESSED)
    {
        PIECE(7, 0x17e, rise + 0xc4, 0x27e, rise + 0x2c4);
        PIECE(11, 0xcd, rise + 0x142, 0x14d, rise + 0x242);
        PIECE(10, 5, rise + 0x14d, 0x105, rise + 0x1cd);
        PIECE(9, 0x32, rise + 0x146, 0x132, rise + 0x246);
        PIECE(8, 0x97, rise + 0x15b, 0x117, rise + 0x1db);
        PIECE(8, 0x145, rise + 0x110, 0x245, rise + 0x210);
    }
    if (DAT_104c0324 != 0)
    {
        if (DAT_104bfd14 == TITLE_WAITING || DAT_104bf890 < FADE_FRAME)
        {
            PIECE(5, 3 - slide, 148, 0x103 - slide, 0x194);
            PIECE(4, 0xb4 - slide, 195, 0x1b4 - slide, 0x143);
            PIECE(6, 0x3a - slide, 228, 0xba - slide, 0x164);
            PIECE(3, 0x9e - slide, 204, 0x11e - slide, 0x1cc);
            PIECE(2, 0x54 - slide, 218, 0x154 - slide, 0x1da);
            PIECE(1, 0xbd - slide, 254, 0x1bd - slide, 0x1fe);
            DAT_118d0ba8[DAT_10202808[0]].DrawTinted(slide + 0x121, 0x91 - DAT_10202888[0], slide + 0x321, 0x291,
                                                     PIECE_UV_MIN, PIECE_UV_MIN, PIECE_UV_MAX, PIECE_UV_MAX, 0, 1.0f,
                                                     0, 0);
        }
        if (DAT_104bfd14 == TITLE_KEY_PRESSED)
        {
            if (DAT_104bf890 < FADE_FRAME)
            {
                DAT_118d0ba8[DAT_104b7210].DrawTinted(-slide, 0x1a0, 300 - slide, SCREEN_HEIGHT, 0, 0.75f, 0.5859375f,
                                                      1.0f, 0, 1.0f, 0, 0);
                DAT_118d0ba8[DAT_104b7210].DrawTinted(0x40, -slide, 0x240, 200 - slide, 0, 0.03125f, 1.0f, 0.75f, 0,
                                                      1.0f, 0, 0);
            }
        }
    }
    if (DAT_104bfd14 == TITLE_WAITING)
    {
        DAT_118d0ba8[DAT_104b7210].DrawTinted(0, 0x1a0, 300, SCREEN_HEIGHT, 0, 0.75f, 0.5859375f, 1.0f, 0, 1.0f, 0, 0);
        if (DAT_104bf890 > FADE_FRAME)
            DAT_118d0ba8[DAT_104b7210].DrawTinted(0x40, 0, 0x240, 200, 0, 0.03125f, 1.0f, 0.75f, 0, 1.0f, 0, 0);
        if (DAT_104bf890 < FADE_FRAME)
        {
            int grow = (FADE_FRAME - DAT_104bf890) * 8;
            DAT_118d0ba8[DAT_104b7210].DrawTinted(0x40 - grow, -grow, grow + 0x240, grow + 200, 0, 0.03125f, 1.0f,
                                                  0.75f, 0, (float)DAT_104bf890 * 0.005f,
                                                  (float)(int)(DAT_104bf890 - FADE_FRAME) * 0.2f, 0);
        }
        else if (DAT_104bf890 < FLICKER_START_FRAME)
        {
            int grow = (DAT_104bf890 - FADE_FRAME) * 8;
            DAT_118d0ba8[DAT_104b7210].DrawTinted(0x40 - grow, -grow, grow + 0x240, grow + 200, 0, 0.03125f, 1.0f,
                                                  0.75f, 0, 0.2f - (float)(DAT_104bf890 - FADE_FRAME) * 0.01f,
                                                  (float)(int)(DAT_104bf890 - FADE_FRAME) * 0.2f, 0);
        }
        if (DAT_104bf890 > FLICKER_START_FRAME)
        {
            float half = (float)DAT_104bf88c * 0.5f;
            DAT_118d0ba8[DAT_104b7210].DrawTinted(0xe5, (int)(336.0f - half), 0x18f, (int)(half + 364.0f),
                                                  0.6484375f, 0.875f, 1.0f, 1.0f, 0, 1.0f, 0, 0);
        }
    }
    FUN_1013f070(0xb, CLEAR_DEPTH_FULL, 0);
    return 1;
}
