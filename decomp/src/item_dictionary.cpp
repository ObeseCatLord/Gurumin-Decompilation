// The item dictionary (game mode 5, titled "item dictionary" in Shift-JIS): lists the items the player has obtained
// in an item window, turns the model of the selected one and lets the mouse tilt it. C++ rather than extern "C": the
// function keeps an exception frame for its Matrix4 and Vec4 locals.

#include <math.h>
#include <stdlib.h>
#include <windows.h>

#include "vec4.h"

#include "matrix4.h"
#include "anim_model.h"
#include "camera_setup.h"
#include "camera_vars.h"
#include "char_vars.h"
#include "doppler_loops.h"
#include "dinput_min.h"
#include "game.h"
#include "frame_vars.h"
#include "game_vars.h"
#include "input_state.h"
#include "item_dictionary.h"
#include "render_state.h"
#include "render_vars.h"
#include "texture_object.h"
#include "tracker.h"
#include "ui_vars.h"

// Shift-JIS "item dictionary" (the window title) and six full-width question marks (the entry of an item not yet
// obtained).
#define ITEM_DICTIONARY_TITLE "\x83\x41\x83\x43\x83\x65\x83\x80\x8e\xab\x93\x54"
#define ITEM_DICTIONARY_UNKNOWN "\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48\x81\x48"

#define ITEM_COUNT 0x60
#define ITEM_WINDOW_ID 0x218
#define DICTIONARY_SOUND 0x357
#define SAVE_RECORD_OBTAINED 0x81
// The background scroll restarts when its counter passes BACKGROUND_SCROLL_LAST; the dictionary can be closed once
// it is past EARLIEST_CLOSE_FRAME.
#define BACKGROUND_SCROLL_LAST 0x137
#define BACKGROUND_SCROLL_RESTART 5
#define EARLIEST_CLOSE_FRAME 5
#define BUTTON_PRESSED 2
#define STATE_OPENING 1
#define STATE_OPEN 2
#define STATE_FADE_START 10
#define LIST_FADE_FULL 0x14
#define PROMPT_SIZE_MAX 10
#define PREVIEW_FADE_FULL 15
#define BACKGROUND_SCROLL_SPEED 0.02f
#define BACKGROUND_WIDTH_UVS 5.0f
#define BACKGROUND_HEIGHT_UVS 3.75f
#define BACKGROUND_Z 0.991f
#define SPIN_SPEED 0.03f
#define TILT_LIMIT 1.5
#define CAMERA_DISTANCE 400.0
#define CAMERA_X -20.0f
#define CAMERA_FOCUS_RISE 50.0f

// Adds item `first + n` to the list when it has a dictionary entry and is not hidden.
#define LIST_ITEM(n)                                                                                \
    do                                                                                              \
    {                                                                                               \
        if (DAT_1088e650[first + (n)].hidden == 0 && DAT_1088e650[first + (n)].caption[0] != 0)     \
            list[count++] = first + (n);                                                            \
    } while (0)

// 0x10115cd0 (ItemDictionary_Update). The first frame opens the item window (items sorted by their level flag, the
// obtained ones from the save, the others as "??????") and sets the scene up. Every frame draws the scrolling
// background from the orbit camera (which turns with the mouse tilt DAT_104c02fc), and once a new engine tick has
// started reads the mouse and keys, updates the windows, the prompt size and the fades, lets a held left button turn
// (DAT_104c02f8) and tilt the model, and closes the dictionary on Escape, a key or a right click. Returns 0.
int ItemDictionary_Update()
{
    int list[0x80];
    int obtained[4];
    int pos[2];
    char left;
    char right;
    if (DAT_104bf8bc == 0)
    {
        DAT_104bf8bc = 1;
        DAT_104bfd14 = 1;
        DAT_1032d67c[0] = 0;
        DAT_10f6553c = DrawItemPreview;
        DAT_10dc4a70 = 0.5f;
        DAT_10dc4a74 = 0.5f;
        DAT_10db6ee8[5].PlayMotion(2, 0, 5, 1.0f);
        FUN_1003ad20(DICTIONARY_SOUND, 0, 1.0f);
        FUN_1000cb30(ITEM_DICTIONARY_TITLE);
        FUN_1000cb90("");
        DAT_104c02fc = 0.3f;
        FUN_101603c0(DAT_10256110, SAVE_RECORD_OBTAINED, &obtained[0]);
        FUN_101603c0(DAT_10256110, SAVE_RECORD_OBTAINED + 1, &obtained[1]);
        FUN_101603c0(DAT_10256110, SAVE_RECORD_OBTAINED + 2, &obtained[2]);
        int count = 0;
        for (int first = 0; first < ITEM_COUNT; first += 8)
        {
            LIST_ITEM(0);
            LIST_ITEM(1);
            LIST_ITEM(2);
            LIST_ITEM(3);
            LIST_ITEM(4);
            LIST_ITEM(5);
            LIST_ITEM(6);
            LIST_ITEM(7);
        }
        qsort(list, count, sizeof(int), FUN_10017a60);
        DAT_104b7200.OpenWindow(ITEM_WINDOW_ID, 0x20, 0x50, 0xb4, 0xf0);
        int slot = DAT_104b7200.FindSlot(ITEM_WINDOW_ID);
        if (slot != 3)
            DAT_104b7200.m_011c[slot] = 9;
        slot = DAT_104b7200.FindSlot(ITEM_WINDOW_ID);
        if (slot != 3)
            DAT_104b7200.m_0128[slot] = 0x14;
        DAT_104b7200.m_0690 = FUN_1000c200;
        for (int entry = 0; entry < count; entry++)
        {
            int item = list[entry];
            if (obtained[item / 32] >> (item % 32) & 1)
            {
                DAT_1042e2a8[entry] = item;
                DAT_104b7200.AddKind5Entry(ITEM_WINDOW_ID, DAT_1088e650[item].caption, &DAT_1030c6c0);
            }
            else
            {
                DAT_1042e2a8[entry] = -1;
                DAT_104b7200.AddFlagEntry(ITEM_WINDOW_ID, ITEM_DICTIONARY_UNKNOWN, &DAT_1030c6c0);
            }
        }
    }
    Matrix4 viewProjection;
    Matrix4 spareA;
    Matrix4 view;
    Matrix4 projection;
    Matrix4 spareB;
    float scroll = (float)DAT_104c0328 * BACKGROUND_SCROLL_SPEED;
    Vec4 eye;
    Vec4 target;
    FUN_1013f070(4, 0xedba67, 0);
    FUN_1013f070(0, 0, 0);
    FUN_1013f070(2, 0, 0);
    FUN_1013f070(7, 1, 0);
    DAT_118d0ba8[DAT_1020283c].DrawTinted(0, 0, 0x280, 0x1e0, scroll, scroll, scroll + BACKGROUND_WIDTH_UVS,
                                          scroll + BACKGROUND_HEIGHT_UVS, BACKGROUND_Z, 1.0f, 0, 0);
    eye.Set(CAMERA_X, (float)(cos(DAT_104c02fc) * -CAMERA_DISTANCE), (float)(sin(DAT_104c02fc) * CAMERA_DISTANCE), 0);
    target.Set(CAMERA_X, 0, 0, 0);
    DAT_10f65688.Copy(eye);
    DAT_10f668e8.Copy(target);
    DAT_10f66938.Copy(target);
    DAT_10f66908.Copy(target);
    DAT_10f65590 = target.z + CAMERA_FOCUS_RISE;
    Camera_SetMatrices(&eye, &target, &view, &projection, 0);
    viewProjection = view * projection;
    DAT_108392d8 = viewProjection;
    if (DAT_10f65504 != DAT_104bf8b8)
    {
        DAT_104c0328++;
        DAT_1020d9e0.M_10001110(0, 0, 0, 0, 0, 0);
        DAT_10f08008.Update();
        DAT_10210f30 = DAT_10f08008.ReadControls(0);
        DAT_1020d9e0.M_10001110(pos, &left, 0, &right, 0, 0);
        DAT_1020d9e0.M_10001550();
        DAT_1065cb28.SetButtons(DAT_10210f30);
        DAT_104bf890++;
        DAT_104bf8b8 = DAT_10f65504;
        DAT_104b7200.UpdateWindows();
        DAT_104b7200.MoveCursor();
        FUN_1003a9d0();
        DAT_1032d67c[0]++;
        if (DAT_1032d67c[0] > BACKGROUND_SCROLL_LAST)
            DAT_1032d67c[0] = BACKGROUND_SCROLL_RESTART;
        else if (DAT_1032d67c[0] > EARLIEST_CLOSE_FRAME &&
                 (DAT_1065cb31 != 0 || (GetKeyState(VK_ESCAPE) & 0xf000) != 0 || right == BUTTON_PRESSED) &&
                 (DAT_104bfd14 == STATE_OPENING || DAT_104bfd14 == STATE_OPEN))
        {
            DAT_104bfd14 = 1;
            DAT_104bf8c0 = 0;
            DAT_104bf890 = 0;
            DAT_1030c50c = 1;
            DAT_10f6553c = FUN_10038010;
            DAT_104c0324 = 0;
            DAT_104b7200.CloseWindow(ITEM_WINDOW_ID);
            return 0;
        }
        DAT_104bf88c += DAT_10202348;
        if (DAT_104bf88c == PROMPT_SIZE_MAX || DAT_104bf88c == 0)
            DAT_10202348 = -DAT_10202348;
        if (DAT_104bf8d4 != 0)
        {
            if (DAT_104bf8d4 == 1)
            {
                if (DAT_104bf8d8 < PREVIEW_FADE_FULL)
                    DAT_104bf8d8++;
            }
            else if (DAT_104bf8d4 == 2 && DAT_104bf8d8 > 0)
                DAT_104bf8d8--;
            if (DAT_104bf8d8 == 0)
                DAT_104bf8d4 = 0;
        }
        FUN_1015f4e0();
        FUN_10112140();
        if (DAT_104c02f4 != 0)
        {
            DAT_104c02f8 = DAT_104c02f8 - (float)(pos[0] - DAT_1021fb40) * SPIN_SPEED;
            DAT_104c02fc = (float)(pos[1] - DAT_1021fb44) * SPIN_SPEED + DAT_104c02fc;
            if (DAT_104c02fc > TILT_LIMIT)
                DAT_104c02fc = 1.5f;
            if (DAT_104c02fc < 0.0)
                DAT_104c02fc = 0;
            if (left == 0)
                DAT_104c02f4 = 0;
        }
        else
        {
            DAT_104c02f8 += SPIN_SPEED;
            if (left == BUTTON_PRESSED)
                DAT_104c02f4 = 1;
        }
        DAT_1021fb40 = pos[0];
        DAT_1021fb44 = pos[1];
        if (DAT_104bfd14 > STATE_FADE_START)
            DAT_104bfd14++;
        if (DAT_104bfd14 >= STATE_FADE_START + 1)
        {
            if (DAT_104c0232 < LIST_FADE_FULL)
                DAT_104c0232++;
        }
        else if (DAT_104bfd14 == STATE_FADE_START)
        {
            if (DAT_104c0232 > STATE_FADE_START)
                DAT_104c0232 = STATE_FADE_START;
            else if (DAT_104c0232 < STATE_FADE_START)
                DAT_104c0232++;
        }
        else if (DAT_104bfd14 < STATE_FADE_START && DAT_104c0232 > 0)
            DAT_104c0232--;
        FUN_1000cdf0();
    }
    return 0;
}
