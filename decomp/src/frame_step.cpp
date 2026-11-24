// The frame step Screenshot_Capture 0x10137b10: runs once per frame (callers: the script, the game modes and
// startup). Pumps the messages, clamps the player's health against tampering, resets the per-frame counters,
// runs the current game mode, presents the frame and saves a screenshot when the clipboard holds a picture.

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "char_slot.h"
#include "dinput_min.h"
#include "font_renderer.h"
#include "item_dictionary.h"
#include "texture_manager.h"
#include "tracker.h"

#include "frame_vars.h"
#include "game_vars.h"
#include "render_vars.h"
#include "script_vars.h"
#include "sound_vars.h"
#include "ui_vars.h"
#include "win_vars.h"

// Highest health the player can have.
#define PLAYER_HEALTH_LIMIT 100
// Anti-tamper rolls: a limit breach closes the game when rand() % 50 is below the threshold.
#define TAMPER_ROLL_RANGE 50
#define TAMPER_ROLL_WINDOW_THRESHOLD 20
#define TAMPER_ROLL_QUIT_THRESHOLD 10
// Game modes (DAT_104bf8c0) that have a per-frame update here.
#define MODE_PLAY 1
#define MODE_5 5
#define MODE_10 10
#define MODE_15 0xf
// Size of the stamp picture shape.dat and its place in the bottom right corner of a screenshot.
#define STAMP_FILE_SIZE 0x3080
#define STAMP_HEADER_SIZE 0x36
#define STAMP_WIDTH 128
#define STAMP_HEIGHT 32
#define STAMP_ROW_BYTES (STAMP_WIDTH * 3)
#define STAMP_MARGIN_X 0xa0
#define STAMP_MARGIN_Y 0x24
#define SCREENSHOT_JPEG_QUALITY 100

// Copies pixel `offset` of the current group of four of the stamp (bytes ending at lastByte) to the screenshot
// unless it is black.
#define STAMP_COPY_PIXEL(offset)     do     {         if (lastByte[(offset) * 3 - 2] != 0 || lastByte[(offset) * 3 - 1] != 0 || lastByte[(offset) * 3] != 0)         {             pixels[(top * width + x + left + (offset)) * 3] = lastByte[(offset) * 3 - 2];             pixels[(top * width + x + left + (offset)) * 3 + 1] = lastByte[(offset) * 3 - 1];             pixels[(top * width + x + left + (offset)) * 3 + 2] = lastByte[(offset) * 3];         }     } while (0)

extern "C" int FUN_1000e750(void);
extern "C" int Game_LoadData(void);
int TitleScreen_Update();                    // 0x10114950 (title_screen.cpp)
extern "C" void FUN_1011d810(void);
#include "name_entry.h"
#include "field_update.h"
extern "C" int FUN_1013f070(int command, int value, int extra);
extern "C" int FUN_101439e0(void);
extern "C" const char *FUN_1013f8a0(const char *format, ...);
extern "C" unsigned char *FUN_1013e9d0(int *width, int *height);

// Sets the quit flag when any entry of the anti-tamper noise table DAT_104c0270 (16 entries, scanned in two rows of
// eight) is non-zero.
static __forceinline void QuitIfNoiseTableUsed()
{
    for (int first = 0; first < 16; first += 8)
    {
        if (DAT_104c0270[first + 0] != 0)
            DAT_10f6552c = 1;
        if (DAT_104c0270[first + 1] != 0)
            DAT_10f6552c = 1;
        if (DAT_104c0270[first + 2] != 0)
            DAT_10f6552c = 1;
        if (DAT_104c0270[first + 3] != 0)
            DAT_10f6552c = 1;
        if (DAT_104c0270[first + 4] != 0)
            DAT_10f6552c = 1;
        if (DAT_104c0270[first + 5] != 0)
            DAT_10f6552c = 1;
        if (DAT_104c0270[first + 6] != 0)
            DAT_10f6552c = 1;
        if (DAT_104c0270[first + 7] != 0)
            DAT_10f6552c = 1;
    }
}

// 0x10137b10 (Screenshot_Capture). The frame step: pumps the window messages (waiting while the device is lost),
// limits the player's health (a breach of the limit closes the window with some probability), clears the per-frame
// counters, runs the update of the current game mode, paces and presents the frame, and when the clipboard holds a
// picture (the screenshot key) saves the back buffer as capture<N>.jpg with the shape.dat stamp in the corner.
// Returns 1 to go on, 0 when the game should end.
extern "C" int FUN_10137b10(void)
{
    if (DAT_10f6552c != 0)
        return 0;
    if (DAT_10f656a0 == 0)
    {
        DAT_10836c70.PumpMessages();
        return 1;
    }
    int limit;
    if (DAT_104bf8c0 == MODE_PLAY && (limit = DAT_10daaae8[0].stats[STAT_MAX_HEALTH]) > PLAYER_HEALTH_LIMIT)
    {
        if (rand() % TAMPER_ROLL_RANGE < TAMPER_ROLL_WINDOW_THRESHOLD)
            PostMessageA(DAT_10f6569c, WM_CLOSE, 0, 0);
        limit = PLAYER_HEALTH_LIMIT;
        DAT_10daaae8[0].stats[STAT_MAX_HEALTH] = limit;
        FUN_1000e750();
        DAT_104c0344++;
    }
    if (DAT_104bf8c0 == MODE_PLAY && DAT_10daaae8[0].stats[STAT_HEALTH] > limit)
    {
        DAT_10daaae8[0].stats[STAT_HEALTH] = limit;
        FUN_1000e750();
    }
    if (DAT_10836c70.PumpAndRecover() == 0)
    {
        if (DAT_10f6552c != 0)
            DAT_1049eae4 = 1;
        return 0;
    }
    if (DAT_104bf8c0 == MODE_PLAY && (limit = DAT_10daaae8[0].stats[STAT_MAX_HEALTH]) > PLAYER_HEALTH_LIMIT)
    {
        if (rand() % TAMPER_ROLL_RANGE < TAMPER_ROLL_QUIT_THRESHOLD)
            PostMessageA(DAT_10f6569c, WM_CLOSE, 0, 0);
        limit = PLAYER_HEALTH_LIMIT;
        DAT_10daaae8[0].stats[STAT_MAX_HEALTH] = limit;
        FUN_1000e750();
        DAT_104c0348++;
    }
    if (DAT_104bf8c0 == MODE_PLAY && DAT_10daaae8[0].stats[STAT_HEALTH] > limit)
    {
        DAT_10daaae8[0].stats[STAT_HEALTH] = limit;
        FUN_1000e750();
    }
    if (DAT_102026d0 != 0 && Game_LoadData() == 0)
        return 0;
    FUN_1013f070(7, 1, 0);
    DAT_104bfd71 = 1;
    DAT_11b044d4 = 0;
    DAT_11b044d8 = 0;
    DAT_10f623c8 = 0;
    DAT_10f1fcf4 = 0;
    DAT_10f141d4 = 0;
    DAT_11b07528 = 0;
    DAT_11b002b0 = 0;
    DAT_10f0d5c8 = 0;
    DAT_10ff1dec = 0;
    DAT_10f623b8 = 0;
    DAT_10f623bc = 0;
    DAT_10f623c0 = 0;
    DAT_10f623c4 = 0;
    DAT_1101d858 = 0;
    DAT_110175f8 = 0;
    if (DAT_104bfec0 == 0)
    {
        QuitIfNoiseTableUsed();
        if (DAT_104bf8c0 == 0)
            TitleScreen_Update();
        else if (DAT_104bf8c0 == MODE_PLAY)
            FUN_1011d810();
        else if (DAT_104bf8c0 == MODE_5)
            ItemDictionary_Update();
        else if (DAT_104bf8c0 == MODE_10)
            NameEntry_Update();
        else if (DAT_104bf8c0 == MODE_15)
            Field_Update();
    }
    if (DAT_104bf8c0 == MODE_PLAY && DAT_10daaae8[0].stats[STAT_MAX_HEALTH] > PLAYER_HEALTH_LIMIT)
    {
        if (rand() % TAMPER_ROLL_RANGE < TAMPER_ROLL_QUIT_THRESHOLD)
            DAT_10f6552c = 1;
        DAT_10daaae8[0].stats[STAT_MAX_HEALTH] = PLAYER_HEALTH_LIMIT;
        FUN_1000e750();
        DAT_104c0344++;
        if (rand() % TAMPER_ROLL_RANGE < TAMPER_ROLL_QUIT_THRESHOLD)
            DAT_10f6552c = 1;
    }
    if (DAT_104bf8c0 == MODE_PLAY && DAT_10daaae8[0].stats[STAT_HEALTH] > DAT_10daaae8[0].stats[STAT_MAX_HEALTH])
    {
        DAT_10daaae8[0].stats[STAT_HEALTH] = DAT_10daaae8[0].stats[STAT_MAX_HEALTH];
        FUN_1000e750();
    }
    DAT_10836c70.PaceFrame();
    if (DAT_104bfec0 != 0)
    {
        DAT_10f08008.Update();
        DAT_10210f30 = DAT_10f08008.ReadControls(0);
        FUN_101439e0();
        FUN_1013f070(4, 0xf7941d, 0);
        DAT_1020e538.DrawText(0x8d, 0xc1, &DAT_104bfec0, 0, 0, 1.0f, 1.2f);
        DAT_10836c70.PresentFrame();
        DAT_1020d9e0.M_10001550();
        char button = 0;
        DAT_1020d9e0.M_10001110(0, &button, 0, 0, 0, 0);
        if (DAT_10210f30 != 0 || button != 0)
            return 0;
    }
    else
        DAT_10836c70.PresentSimple();
    if (OpenClipboard(DAT_10f6569c))
    {
        if (GetClipboardData(CF_DIB) != 0)
        {
            char path[0x100];
            FILE *file;
            do
            {
                sprintf(path, "%s\\capture\\capture%05d.jpg", DAT_10f627d0, DAT_104c00c4);
                file = fopen(path, "rb");
                if (file != 0)
                    fclose(file);
                DAT_104c00c4++;
            } while (file != 0);
            file = fopen(path, "wb");
            if (file != 0)
            {
                fclose(file);
                int width;
                int height;
                unsigned char *pixels = FUN_1013e9d0(&width, &height);
                file = fopen(FUN_1013f8a0("shape.dat", 0), "rb");
                if (file != 0)
                {
                    unsigned char *stamp = new unsigned char[STAMP_FILE_SIZE];
                    fread(stamp, 1, STAMP_FILE_SIZE, file);
                    fclose(file);
                    int left = width - STAMP_MARGIN_X;
                    int top = height - STAMP_MARGIN_Y;
                    unsigned char *rowEnd = stamp + STAMP_HEADER_SIZE + (STAMP_HEIGHT - 1) * STAMP_ROW_BYTES + 2;
                    for (int row = 0; row < STAMP_HEIGHT; row++)
                    {
                        unsigned char *lastByte = rowEnd;
                        for (int x = 0; x < STAMP_WIDTH; x += 4)
                        {
                            STAMP_COPY_PIXEL(0);
                            STAMP_COPY_PIXEL(1);
                            STAMP_COPY_PIXEL(2);
                            STAMP_COPY_PIXEL(3);
                            lastByte += 12;
                        }
                        rowEnd -= STAMP_ROW_BYTES;
                        top++;
                    }
                    free(stamp);
                    DAT_11b1bb80(path, pixels, width, height, SCREENSHOT_JPEG_QUALITY, 0);
                }
                free(pixels);
            }
            EmptyClipboard();
        }
        CloseClipboard();
    }
    QuitIfNoiseTableUsed();
    return 1;
}
