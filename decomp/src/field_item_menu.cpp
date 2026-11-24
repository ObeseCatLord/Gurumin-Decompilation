// The field item menu: every frame of the field outside the cutscenes. Reads the mouse and the pad, walks the player
// towards the pointer, opens and closes the list of items in the lower right corner of the screen (two pages of
// fifteen rows, scrolled by the wheel or by holding the pointer at the edge), picks an entry with the mouse or the
// pad and uses the item. Coordinates are 640x480 screen pixels.

// Game::TestScriptFlag stays a call here: the functions defined inside a class are not expanded.
#pragma auto_inline(off)

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vec4.h"
#include "matrix4.h"
#include "node.h"
#include "engine_memory.h"
#include "camera_vars.h"
#include "char_slot.h"
#include "char_tables.h"
#include "char_vars.h"
#include "game.h"
#include "tracker.h"
#include "game_vars.h"
#include "ui_vars.h"
#include "render_vars.h"
#include "doppler_loops.h"
#include "dodge_game.h"
#include "minigame_hud.h"
#include "menu_scene.h"
#include "menu_save_load.h"
#include "item_dictionary.h"
#include "item_info.h"
#include "map_records.h"
#include "path_walk.h"
#include "map_vars.h"
#include "script_vars.h"
#include "texture_load.h"
#include "tutorial_book.h"
#include "field_item_menu.h"
#include "item_use_texts.h"

#pragma auto_inline(on)

#define SOUND_MENU_OPEN 0x361
#define SOUND_MENU_CLOSE 0x362
#define SOUND_MENU_SCROLL 0x363

#define INPUT_LOCK_FRAMES 10
#define STICK_DEAD_ZONE 4
#define STICK_MENU_THRESHOLD 50
#define STICK_REPEAT_LIMIT 5
#define STICK_REPEAT_RAMP 9
#define STICK_IDLE_LIMIT 25
#define STICK_IDLE_STEP 5

#define WALK_MAX_LENGTH 127.0f
#define WALK_MIN_LENGTH 3.0f

#define BUTTON_PRESSED 2

#define CORNER_SUM 0x410
#define PAGE_ROWS 15
#define ROW_HEIGHT 0x16
#define LIST_TOP 0x4f
#define FADE_STEPS 8
#define FADE_DIVISOR 32
#define MENU_ANIM_FRAMES 15

#define WINDOW_ID_SLOT_LIST 0x22c

// Entries of DAT_104485e8 that are commands, not items.
#define ENTRY_SAVE 0xa1
#define ENTRY_COMMAND_A 0xa2
#define ENTRY_COMMAND_B 0xa3
#define ENTRY_PAGE_COMMAND 0xa4
#define ENTRY_QUIT 0xa5



// Closes the item menu: the close sound, the menu windows and caption, the overlay model with its texture slots,
// then the menu state. Does nothing past the sound when the menu is not open.
__forceinline void CloseItemMenu()
{
    FUN_1003ad20(SOUND_MENU_CLOSE, 0, VOLUME_FULL);
    DAT_104bfe90 = 0;
    if (DAT_104bfe1c != 0)
    {
        DAT_1030c500[0x13] = 0;
        DAT_104bfd78 = 0;
        DAT_104b7200.CloseWindow(WINDOW_ID_SLOT_LIST);
        FUN_100106a0();
        FUN_1000cb30("");
        FUN_1000cb90("");
        DAT_104bf8d4 = 1;
        if (DAT_104c03c8 != 0)
        {
            DAT_104c03dc = 0;
            DAT_104c03c8->Release();
            DAT_11b1ba40(DAT_104c03c8);
            DAT_104c03c8 = 0;
            DAT_104c03cc = 0;
            DAT_104c03d0 = 0;
            DAT_104c03d4 = 0;
            DAT_104c03d8 = 0;
            DAT_104c03e0 = 0;
            DAT_104c03e4 = 0;
            for (int slot = 0; slot < 0x22; slot++)
            {
                if (DAT_1030b870[slot] > -1)
                {
                    FUN_10174c70(DAT_1030b870[slot], 1);
                    DAT_1030b870[slot] = -1;
                }
            }
        }
        FUN_100106a0();
        DAT_104bfe1c = 0;
        memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
    }
}

// Picks the entry `entry` of the list: the commands set their flag, an item becomes the selected one and is to be used.
__forceinline void PickEntry(int entry, int &useItem, int &selected)
{
    if (entry == ENTRY_SAVE)
        DAT_1030c540 = 1;
    else if (entry == ENTRY_COMMAND_A)
        DAT_1030c558 = 1;
    else if (entry == ENTRY_COMMAND_B)
        DAT_1030c55c = 1;
    else if (entry == ENTRY_PAGE_COMMAND)
        DAT_1030c510 = 1;
    else if (entry == ENTRY_QUIT)
        DAT_1030c574 = 1;
    else
    {
        useItem = 1;
        DAT_10202464[0] = entry;
        selected = entry;
    }
}

// 0x100ba2a0 (FieldItemMenu_Update). One frame of the field. Returns 0.
int FieldItemMenu_Update()
{
    int moving = 0;
    char left = 0;
    char right = 0;
    int mouse[2];
    int wheel;
    int stickX;
    int stickY;
    int selected = DAT_10202464[0];
    int useItem = 0;
    int closedByRight = 0;
    if (TutorialBook_Update() != 0)
    {
        DAT_1020c354 = INPUT_LOCK_FRAMES;
        return 0;
    }
    if (DAT_104b7200.IsAnyWindowBusy() != 0)
        DAT_1020c354 = INPUT_LOCK_FRAMES;
    if (DAT_1020c304 == 0 && DAT_1020c354 <= 0)
    {
        stickX = DAT_1065cb48;
        stickY = DAT_1065cb4c;
        if (DAT_104c0148 != 0)
            DAT_104c0148--;
        if (DAT_1065cb90 == 0)
        {
            if (abs(stickX) > STICK_DEAD_ZONE || abs(stickY) > STICK_DEAD_ZONE)
                DAT_1020d9e0.M_100014a0();
            DAT_1020d9e0.M_10001450();
            DAT_1020d9e0.M_10001110(mouse, &left, 0, &right, &wheel, 0);
            if (left != 0 && DAT_1020c2b0 == 0)
                right = 0;
            int open = DAT_104bfe1c;
            int pointerX = mouse[0];
            int pointerY = mouse[1];
            if (DAT_104b7200.IsAnyWindowBusy() == 0 && DAT_104c03c8 == 0 && open != 0 &&
                DAT_104b7200.PointInRect(pointerX, pointerY, 0x1e4, -0x52, 0x286, 0x19e) != 0)
            {
                if (left == BUTTON_PRESSED)
                {
                    if (DAT_104c02bc == 1)
                    {
                        if (DAT_104b7200.PointInRect(pointerX, pointerY, 0x1e2, 0x36, 0x220, 0x4e) != 0)
                        {
                            DAT_104c02bc = 0;
                            DAT_104c0048 = 0;
                        }
                    }
                    else if (DAT_104c02bc == 0)
                    {
                        if (DAT_104b7200.PointInRect(pointerX, pointerY, 0x220, 0x36, 0x26a, 0x4e) != 0)
                        {
                            DAT_104c02bc = 1;
                            DAT_104c0048 = 0;
                        }
                    }
                }
                goto list_input;
            }
            if (left != 0 && DAT_104b7200.IsAnyWindowBusy() == 0 && open == 0)
            {
                Vec4 step;
                step.z = 0;
                step.x = (float)mouse[0] - (float)DAT_1020c318;
                step.y = (float)mouse[1] - (float)DAT_1020c31c;
                float length = step.Length2D();
                if (length > WALK_MAX_LENGTH)
                {
                    length = WALK_MAX_LENGTH;
                    step.Normalized3();
                }
                else if (length > WALK_MIN_LENGTH)
                    step.Normalized3();
                else
                    step.Fill(0.0f);
                step.Copy(step * length);
                stickX = (int)step.x;
                stickY = (int)step.y;
            }
            if (DAT_104c03c8 != 0)
                goto list_idle;
            open = DAT_104bfe1c;
        list_input:
            if (open != 0 && DAT_104b7200.PointInRect(pointerX, pointerY, 0x1e4, -0x52, 0x29c, 0x1ca) != 0 &&
                DAT_104b7200.IsAnyWindowBusy() == 0 && DAT_1020d9e0.M_10001440() != 0)
            {
                int row = (mouse[1] - LIST_TOP) / ROW_HEIGHT;
                if (row >= 0 && row < PAGE_ROWS)
                    DAT_10202464[0] = DAT_104485e8[row];
                if (wheel > 0 && DAT_104c0048 > 0)
                    DAT_104c0048--;
                else if (wheel < 0 && DAT_104c0044 - DAT_104c0048 > 15 &&
                         DAT_104b7200.PointInRect(mouse[0], mouse[1], 0x1e4, -0x3c, 0x286, 0x1b6) != 0)
                    DAT_104c0048++;
                else if (row < 0 && DAT_104c0048 > 0)
                {
                    if (wheel == 0)
                        DAT_104bfe90 = 0;
                    else
                        DAT_104bfe90--;
                    if (DAT_104c004c > 0)
                        DAT_104c004c--;
                    else
                    {
                        DAT_104c0048--;
                        DAT_104c004c = 5 - DAT_104c0050;
                        if (DAT_104c0050 < 5)
                            DAT_104c0050++;
                        if (DAT_104c004c != 0)
                            DAT_104c004c--;
                    }
                }
                else if (row > 15 && DAT_104c0044 - DAT_104c0048 > 15 &&
                         DAT_104b7200.PointInRect(mouse[0], mouse[1], 0x1e4, -0x3c, 0x286, 0x1e0) != 0 &&
                         mouse[0] + mouse[1] < CORNER_SUM)
                {
                    if (wheel == 0)
                        DAT_104bfe90 = PAGE_ROWS - 1;
                    else
                        DAT_104bfe90--;
                    if (DAT_104c004c > 0)
                        DAT_104c004c--;
                    else
                    {
                        DAT_104c0048++;
                        DAT_104c004c = 5 - DAT_104c0050;
                        if (DAT_104c0050 < 5)
                            DAT_104c0050++;
                        if (DAT_104c004c != 0)
                            DAT_104c004c--;
                    }
                }
                else
                {
                    DAT_104c0050 = 0;
                    if (row < PAGE_ROWS && row >= 0 && DAT_104485e8[row] != 0)
                        DAT_104bfe90 = row;
                }
            }
            else
            {
            list_idle:
                DAT_104c0050 = 0;
            }
        list_done:
            if (left == 0)
                DAT_104c0097 = 0;
            if (DAT_104c03c8 == 0)
            {
                if (DAT_104bfdf0 == 0)
                {
                    if (DAT_104b7200.IsAnyWindowBusy() == 0 && DAT_104c0028 == 0 && left != 0 &&
                        mouse[0] + mouse[1] >= CORNER_SUM)
                    {
                        if (left == BUTTON_PRESSED)
                        {
                            stickX = 0;
                            stickY = 0;
                            DAT_104c0232 = 4;
                            if (DAT_104bfe1c == 0)
                            {
                                FUN_1003ad20(SOUND_MENU_OPEN, 0, VOLUME_FULL);
                                DAT_104bfe1c = 1;
                            }
                            else
                                CloseItemMenu();
                            DAT_104bfd74 = 1;
                            DAT_104bfe90 = 0;
                            DAT_104c0048 = 0;
                            DAT_102024b0 = 1;
                            DAT_1020249c = 1;
                            DAT_1030b868 = 2;
                            DAT_104bfeb9 = 2;
                            selected = DAT_10202464[0];
                            goto entry_done;
                        }
                    }
                    else if (DAT_104b7200.IsAnyWindowBusy() == 0 && left == BUTTON_PRESSED && DAT_104bfe1c != 0 &&
                             DAT_104c0099 > 14)
                    {
                        int clickX = mouse[0];
                        int clickY = mouse[1];
                        if (DAT_104b7200.PointInRect(clickX, clickY, 0x1e4, 0x4f, 0x26e, 0x194) == 0)
                        {
                            if (clickX < 0x1e0 && clickY < 0x168)
                            {
                                CloseItemMenu();
                                DAT_104bfd74 = 1;
                                goto entry_done;
                            }
                        }
                        else if (DAT_104c0028 == 0)
                        {
                            int row;
                            if (DAT_1020d9e0.M_10001440() == 0)
                                row = DAT_104bfe90;
                            else
                                row = (mouse[1] - LIST_TOP) / ROW_HEIGHT;
                            int entry = DAT_104485e8[row];
                            if (entry != 0)
                            {
                                DAT_104c0148 = 2;
                                PickEntry(entry, useItem, selected);
                            }
                        entry_done:
                            if (DAT_104c03c8 != 0)
                                goto input_done;
                        }
                    }
                }
                if (DAT_104c0148 == 0 && DAT_104bfe1c != 0 && DAT_104c0099 > 14 && DAT_1032d69c != 0 &&
                    DAT_104b7200.IsAnyWindowBusy() == 0)
                {
                    if (DAT_1065cb30 == 0)
                    {
                        if (DAT_1065cb31 != 0 || right == BUTTON_PRESSED)
                        {
                            closedByRight = 1;
                            CloseItemMenu();
                            DAT_104bfd74 = 1;
                            DAT_1065cb31 = 0;
                        }
                    }
                    else
                        PickEntry(DAT_104485e8[DAT_104bfe90], useItem, selected);
                }
            }
        }
    input_done:
        if (abs(stickX) < STICK_DEAD_ZONE && abs(stickY) < STICK_DEAD_ZONE)
            ;
        else
            moving = 1;
        if (DAT_10c63518 != 0)
        {
            if (DAT_10210fc0[DAT_10c64a5c].points[0].x != 0)
                DAT_1020c2bc = 0;
            DAT_10c63518 = 0;
        }
        if (DAT_104bfe1c == 0 && DAT_1020c2bc == 0 && DAT_104c03dc == 0)
        {
            if (moving == 0 || DAT_102028dc != 0 || (left != 0 && mouse[0] + mouse[1] >= CORNER_SUM))
            {
                DAT_1020c2b0 = 0;
                DAT_1020c310 = 0;
            }
            else
            {
                DAT_1020c2b0 = 1;
                Player_WalkAlongPath(stickX, stickY);
            }
        }
        else
            DAT_1020c2bc = 0;
        Map_SelectMovie();
        if (DAT_104b7200.IsAnyWindowBusy() == 0)
        {
            if (DAT_104c03c8 != 0)
                goto open_done;
            if (DAT_104bfe1c != 0)
                goto book_check;
            if (((right != BUTTON_PRESSED || closedByRight != 0 || DAT_1065cb90 != 0) &&
                 (DAT_1065cb30 == 0 && DAT_1065cb39 == 0 && DAT_1020c2e8 != 1)) ||
                DAT_1020c2d8 != 0 || DAT_102028dc != 0)
                goto open_done;
            if (DAT_1032d69c == 0)
            {
                DAT_104bfd28 = 3;
                DAT_1030c570 = 1;
                DAT_10202350 = 0;
            }
            if (DAT_10c63384 != 0 || DAT_104c02e6 != 0)
                goto open_done;
            if (DAT_10210fc0[DAT_10c64a5c].points[0].x >= 0 && DAT_1065cb39 == 0 && DAT_1020c2e8 == 0)
            {
                DAT_1020247c[DAT_104bfe88] = 0x3a2;
                DAT_104bfe88 = (DAT_104bfe88 + 1) & 7;
                DAT_1020c2bc = 1;
                DAT_1020c2b0 = 0;
                goto open_done;
            }
            if (DAT_1032d69c != 0 && DAT_1020c33c != 0 && DAT_1065cb39 == 0 && DAT_1020c2e8 == 0)
            {
                DAT_104b7200.RegisterEvent(DAT_1020c33c + 20000);
                goto open_done;
            }
            if (DAT_1020c2e8 != 1 && DAT_1065cb39 == 0)
                goto open_done;
            FUN_1003ad20(SOUND_MENU_OPEN, 0, VOLUME_FULL);
            DAT_104bfe1c = 1;
            DAT_104bfd74 = 1;
            DAT_104bfe90 = 0;
            DAT_104c0048 = 0;
            DAT_102024b0 = 1;
            DAT_1020249c = 1;
            DAT_1030b868 = 2;
            DAT_104bfeb9 = 2;
            selected = DAT_10202464[0];
        }
        else if (DAT_104c03c8 == 0 && DAT_104bfe1c != 0)
        {
        book_check:
            if (DAT_1065cb39 != 0 && DAT_104b7200.IsAnyWindowBusy() == 0)
            {
                FUN_1003ad20(SOUND_MENU_CLOSE, 0, VOLUME_FULL);
                DAT_104bfe90 = 0;
                if (DAT_104bfe1c != 0)
                {
                    DAT_1030c500[0x13] = 0;
                    DAT_104bfd78 = 0;
                    DAT_104b7200.CloseWindow(WINDOW_ID_SLOT_LIST);
                    FUN_100106a0();
                    FUN_1000cb30("");
                    FUN_1000cb90("");
                    DAT_104bf8d4 = 1;
                    if (DAT_104c03c8 != 0)
                    {
                        DAT_104c03dc = 0;
                        DAT_104c03c8->Release();
                        DAT_11b1ba40(DAT_104c03c8);
                        DAT_104c03c8 = 0;
                        DAT_104c03cc = 0;
                        DAT_104c03d0 = 0;
                        DAT_104c03d4 = 0;
                        DAT_104c03d8 = 0;
                        DAT_104c03e0 = 0;
                        DAT_104c03e4 = 0;
                        for (int slot = 0; slot < 0x22; slot++)
                        {
                            if (DAT_1030b870[slot] > -1)
                            {
                                FUN_10174c70(DAT_1030b870[slot], 1);
                                DAT_1030b870[slot] = -1;
                            }
                        }
                    }
                    FUN_100106a0();
                    memset(DAT_1030c500, 0, sizeof(DAT_1030c500));
                    DAT_104bfe1c = 0;
                }
                DAT_104bfd74 = 1;
            }
        }
    open_done:
        DAT_1020c2d8 = 0;
        if (DAT_104c03c8 != 0)
            goto frame_end;
        if (DAT_104c0148 == 0 && DAT_104c0099 > 14 && DAT_104bfe1c != 0 && DAT_104b7200.IsAnyWindowBusy() == 0 &&
            DAT_104c0028 == 0)
        {
            if (stickY == 0 && stickX == 0)
            {
                if (DAT_104bfe98 < STICK_IDLE_LIMIT)
                    DAT_104bfe98 += STICK_IDLE_STEP;
                DAT_104bfe9c = 0;
            }
            else if (DAT_104bfe9c < STICK_REPEAT_RAMP)
                DAT_104bfe9c++;
            if (DAT_104bfe9c + DAT_104bfe98 > STICK_REPEAT_LIMIT)
            {
                double stickYValue = (double)stickY;
                if (fabs(stickYValue) < STICK_MENU_THRESHOLD && stickX > STICK_MENU_THRESHOLD && DAT_104c02bc == 0)
                {
                    DAT_104c02bc = 1;
                    DAT_104c0048 = 0;
                }
                else if (fabs(stickYValue) < STICK_MENU_THRESHOLD && stickX < -STICK_MENU_THRESHOLD &&
                         DAT_104c02bc == 1)
                {
                    DAT_104c02bc = 0;
                    DAT_104c0048 = 0;
                }
                if (stickY < -STICK_MENU_THRESHOLD)
                {
                    DAT_104bfe98 = 0;
                    if (DAT_104bfe94 != 0 && DAT_104bfe90 + DAT_104c0048 != 0)
                        FUN_1003ad20(SOUND_MENU_SCROLL, 0, VOLUME_FULL);
                    if (DAT_104bfe90 < 1)
                    {
                        if (0 < DAT_104c0048)
                            DAT_104c0048--;
                    }
                    else
                        DAT_104bfe90--;
                    DAT_1020d9e0.M_100014a0();
                }
                if (stickY > STICK_MENU_THRESHOLD)
                {
                    DAT_104bfe98 = 0;
                    if (DAT_104bfe94 != 0 && DAT_104bfe90 + DAT_104c0048 < DAT_104c0044 - 1)
                        FUN_1003ad20(SOUND_MENU_SCROLL, 0, VOLUME_FULL);
                    if (DAT_104bfe90 == PAGE_ROWS - 1 && DAT_104c0044 - DAT_104c0048 > 15)
                        DAT_104c0048++;
                    else if (DAT_104c0048 + DAT_104bfe90 < DAT_104c0044 - 1)
                        DAT_104bfe90++;
                    DAT_1020d9e0.M_100014a0();
                }
                if (DAT_104c03c8 != 0)
                    goto frame_end;
            }
        }
        if (useItem == 0 || DAT_104b7200.IsAnyWindowBusy() != 0)
            goto frame_end;
        {
            const ItemInfo &info = DAT_1088e6b0[selected];
            const ItemText &text = DAT_108f667c[selected];
            const char *refusal;
            if (info.healsHealth != 0 && DAT_10daaae8[0].stats[STAT_HEALTH] == DAT_10daaae8[0].stats[STAT_MAX_HEALTH])
                refusal = TEXT_HEALTH_FULL;
            else if (info.restricted != 0)
                refusal = TEXT_CANNOT_USE_HERE;
            else if (info.drillFullFlag != 0 && DAT_10874848.TestScriptFlag(info.drillFullFlag) != 0)
                refusal = TEXT_DRILL_FULL;
            else if (info.type == 1 && DAT_10c655f0 == COSTUME_TOO_SMALL)
                refusal = TEXT_TOO_SMALL;
            else if (info.type == 1 && (DAT_10c655f0 == COSTUME_UNSUITED_A || DAT_10c655f0 == COSTUME_UNSUITED_B ||
                                        DAT_10c655f0 == COSTUME_UNSUITED_C))
                refusal = TEXT_UNSUITED;
            else
                refusal = 0;
            if (refusal != 0)
            {
                FUN_1000edf0(refusal, 0, ALTERNATE_STYLE_FRAMES);
                goto frame_end;
            }
            if (info.type != 2 && info.type != 3 && info.type != 4)
            {
                DAT_1020c344 = 1;
                DAT_104c03a4 = 0;
                DAT_104c039c->PreScanFile(FUN_1013f8a0(text.modelPath, 0), 0);
                DAT_104bfdf0 = text.heldState;
                DAT_104c0319 = 0;
            }
            if (text.keepOnUse == 0 && info.consumable != 0)
            {
                int amount = 1;
                int index = selected;
                FUN_100120a0(&index, &amount);
                if (DAT_103150c0[selected] < 1)
                    DAT_10202464[0] = 0;
            }
            if (DAT_10c5e1c4[text.useScript] != 0)
            {
                FUN_1000da20(text.useScript);
                FUN_1003ad20(SOUND_ITEM_USED, 0, VOLUME_FULL);
                return 0;
            }
        }
    frame_end:
        if (DAT_104c009a == 0)
        {
            if (DAT_104c0099 != 0)
                DAT_104c0099--;
        }
        else if (DAT_104c0099 < MENU_ANIM_FRAMES)
            DAT_104c0099++;
        if (DAT_104bfe1c == 0)
        {
            if (DAT_104bfe18 > 0)
                DAT_104bfe18--;
            DAT_10f654b4 = -(DAT_10202b24 * DAT_104bfe18 / FADE_DIVISOR);
            DAT_10f654b8 = -(DAT_10202b28 * DAT_104bfe18 / FADE_DIVISOR);
            if (DAT_104bfd74 != 4)
            {
                DAT_104c009a = 0;
                DAT_104c0099 = 0;
                return 0;
            }
        }
        else
        {
            if (DAT_104bfe18 < FADE_STEPS)
                DAT_104bfe18++;
            DAT_10f654b4 = -(DAT_10202b24 * DAT_104bfe18 / FADE_DIVISOR);
            DAT_10f654b8 = -(DAT_10202b28 * DAT_104bfe18 / FADE_DIVISOR);
            FUN_1000d620(0, 0, 10);
        }
        DAT_104c009a = 1;
        return 0;
    }
    if (DAT_1020c354 > 0)
        DAT_1020c354--;
    if (DAT_1020c304 != 0)
        DAT_1020c354 = INPUT_LOCK_FRAMES;
    return 0;
}
